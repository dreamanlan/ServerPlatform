#include "Thread.h"
#include "ScriptThread.h"
#include "MaintenanceScript.h"
#include "Communication/SessionManager.h"
#include "netioservice.h"
#include "tcp_session.h"

using SessionManagers = HashtableT<int, SessionManager*>;
static const int MAX_SESSION_MANAGER_NUM = 1024;

static SessionManagers g_SessionManagers;
static int g_ConnectFinish = TRUE;
static NetIoService* g_pNetIoService = NULL;
static event_base* g_EventBase = NULL;

using HandleNameHandleChangedPtr = void(*)(int worldId, int addOrUpdate, const char* name, uint64_t handle);
static HandleNameHandleChangedPtr g_HandleNameHandleChangedPtr = NULL;

using HandleMessagePtr = void(*)(int worldId, unsigned int seq, uint64_t src, uint64_t dest, const void* msg, int len);
static HandleMessagePtr g_HandleMessagePtr = NULL;

using HandleMessageResultPtr = void(*)(int worldId, unsigned int seq, uint64_t src, uint64_t dest, int result);
static HandleMessageResultPtr g_HandleMessageResultPtr = NULL;

using HandleCommandPtr = void(*)(int worldId, uint64_t src, uint64_t dest, const char* msg);
static HandleCommandPtr g_HandleCommandPtr = NULL;

using LogHandlerPtr = void(*)(const char* log, int len);

int g_IsRun = TRUE;

static void ResetSessionManagers()
{
    const char* pCenterNum = GetConfig("centernum");
    int centerNum = 0;
    if (NULL != pCenterNum) {
        centerNum = (int)atoi(pCenterNum);
        for (int ix = 0; ix < centerNum; ++ix) {
            char temp[256];
            tsnprintf(temp, 256, "ip%d", ix);
            const char* pIp = GetConfig(temp);
            if (NULL == pIp)
                pIp = "127.0.0.1";
            tsnprintf(temp, 256, "port%d", ix);
            const char* pPort = GetConfig(temp);
            int port = 20000;
            if (NULL != pPort)
                port = (int)atoi(pPort);
            tsnprintf(temp, 256, "worldid%d", ix);
            const char* pWorldId = GetConfig(temp);
            int worldId = 0;
            if (NULL != pWorldId)
                worldId = (int)atoi(pWorldId);
            tsnprintf(temp, 256, "name%d", ix);
            const char* pName = GetConfig(temp);

            if (NULL != pWorldId && NULL != pName) {
                SessionManager* pMgr = g_SessionManagers.Get(worldId);
                if (NULL == pMgr) {
                    pMgr = new SessionManager();
                    pMgr->Init(g_EventBase);
                    pMgr->SetIpPort(worldId, pIp, port);
                    pMgr->UpdateMyName(pName);
                    g_SessionManagers.Add(worldId, pMgr);
                }
                else {
                    pMgr->SetIpPort(worldId, pIp, port);
                    pMgr->UpdateMyName(pName);
                    pMgr->SetSession(NULL);
                }
            }
        }
    }
}

void HandleNameHandleChanged(int worldId, bool addOrUpdate, const char* name, uint64_t handle)
{
    if (NULL != g_HandleNameHandleChangedPtr && NULL != name) {
        (*g_HandleNameHandleChangedPtr)(worldId, addOrUpdate ? 1 : 0, name, handle);
    }
}

void HandleMessage(int worldId, unsigned int seq, uint64_t src, uint64_t dest, const char* msg, int len)
{
    if (NULL != g_HandleMessagePtr && NULL != msg) {
        (*g_HandleMessagePtr)(worldId, seq, src, dest, msg, len);
    }
}

void HandleMessageResult(int worldId, unsigned int seq, uint64_t src, uint64_t dest, int result)
{
    if (NULL != g_HandleMessageResultPtr) {
        (*g_HandleMessageResultPtr)(worldId, seq, src, dest, result);
    }
}

void HandleCommand(int worldId, uint64_t src, uint64_t dest, const char* msg)
{
    if (NULL != g_HandleCommandPtr && NULL != msg) {
        (*g_HandleCommandPtr)(worldId, src, dest, msg);
    }
}

extern "C" void ReloadConfigScript()
{
    ReloadConfigState();
    ResetSessionManagers();
}

extern "C" int GetConfig(const char* key, char* buf, int len)
{
    int ret = 0;
    if (NULL != key && NULL != buf) {
        const char* val = GetConfig(key);
        if (NULL != val) {
            tsnprintf(buf, len, "%s", val);
            ret = 1;
        }
    }
    return ret;
}

extern "C" uint64_t TargetHandle(int worldId, const char* name)
{
    uint64_t handle = 0;
    if (NULL != name) {
        SessionManager* pMgr = g_SessionManagers.Get(worldId);
        if (NULL != pMgr) {
            handle = pMgr->TargetHandle(name);
        }
    }
    return handle;
}

extern "C" int TargetName(int worldId, uint64_t handle, char* buf, int len)
{
    int ret = 0;
    if (NULL != buf) {
        SessionManager* pMgr = g_SessionManagers.Get(worldId);
        if (NULL != pMgr) {
            const char* name = pMgr->TargetName(handle);
            if (NULL != name) {
                tsnprintf(buf, len, "%s", name);
                ret = 1;
            }
        }
    }
    return ret;
}

extern "C" int SendByHandle(int worldId, uint64_t dest, const void* data, int len)
{
    int ret = 0;
    if (NULL != data) {
        SessionManager* pMgr = g_SessionManagers.Get(worldId);
        if (NULL != pMgr) {
            TcpSession* pSession = pMgr->GetSession();
            if (NULL != pSession) {
                MessageTransmit msg;
                msg.m_Sequence = pMgr->SessionSequence();
                msg.m_Src = pMgr->MyHandle();
                msg.m_Dest = dest;
                if (msg.IsValid() && pSession->Send(reinterpret_cast<const char*>(&msg), sizeof(msg) - 1, reinterpret_cast<const char*>(data), len)) {
                    pMgr->IncSessionSequence();
                    ret = 1;
                }
            }
        }
    }
    return ret;
}

extern "C" int SendByName(int worldId, const char* name, const void* data, int len)
{
    int ret = 0;
    if (NULL != name && NULL != data) {
        SessionManager* pMgr = g_SessionManagers.Get(worldId);
        if (NULL != pMgr) {
            uint64_t dest = pMgr->TargetHandle(name);
            ret = SendByHandle(worldId, dest, data, len);
        }
    }
    return ret;
}

extern "C" int SendCommandByHandle(int worldId, uint64_t dest, const char* command)
{
    int ret = 0;
    if (NULL != command) {
        SessionManager* pMgr = g_SessionManagers.Get(worldId);
        if (NULL != pMgr) {
            TcpSession* pSession = pMgr->GetSession();
            if (NULL != pSession) {
                size_t cmdSize = strlen(command);
                MessageCommand msg;
                msg.m_Src = pMgr->MyHandle();
                msg.m_Dest = dest;
                if (pSession->Send(reinterpret_cast<const char*>(&msg), static_cast<int>(sizeof(msg) - 1), command, static_cast<int>(cmdSize + 1))) {
                    ret = 1;
                }
            }
        }
    }
    return ret;
}

extern "C" int SendCommandByName(int worldId, const char* name, const char* command)
{
    int ret = 0;
    if (NULL != name && NULL != command) {
        SessionManager* pMgr = g_SessionManagers.Get(worldId);
        if (NULL != pMgr) {
            uint64_t dest = pMgr->TargetHandle(name);
            ret = SendCommandByHandle(worldId, dest, command);
        }
    }
    return ret;
}

extern "C" int IsRun()
{
    return g_IsRun;
}

extern "C" void Quit()
{
    g_IsRun = FALSE;
}

extern "C" void SetCenterLogHandler(LogHandlerPtr logHandler)
{
    SetLogHandler(logHandler);
}

extern "C" void Init(const char* serverType, int argc, const char* const* argv, HandleNameHandleChangedPtr nameHandleCallback, HandleMessagePtr msgCallback, HandleMessageResultPtr msgResultCallback, HandleCommandPtr cmdCallback)
{
#if defined(__LINUX__)
    signal(SIGPIPE, SIG_IGN);
#endif

    InitConfigState(serverType, argc, argv);

    g_HandleNameHandleChangedPtr = nameHandleCallback;
    g_HandleMessagePtr = msgCallback;
    g_HandleMessageResultPtr = msgResultCallback;
    g_HandleCommandPtr = cmdCallback;

    g_pNetIoService = new NetIoService();
    g_pNetIoService->Init();
    g_EventBase = g_pNetIoService->EventBase();

    g_SessionManagers.InitTable(MAX_SESSION_MANAGER_NUM);
    ResetSessionManagers();
}

extern "C" void Tick()
{
    if (NULL != g_pNetIoService && NULL != g_EventBase) {
        g_pNetIoService->Poll();
        for (SessionManagers::Iterator it = g_SessionManagers.First(); !it.IsNull(); ++it) {
            SessionManager* pMgr = it->GetValue();
            if (NULL != pMgr) {
                pMgr->Tick();
            }
        }
    }
}

extern "C" int IsConnected(int worldId)
{
    int ret = 0;
    SessionManager* pMgr = g_SessionManagers.Get(worldId);
    if (NULL != pMgr) {
        ret = (NULL != pMgr->GetSession() ? 1 : 0);
    }
    return ret;
}

extern "C" void Disconnect(int worldId)
{
    SessionManager* pMgr = g_SessionManagers.Get(worldId);
    if (NULL != pMgr) {
        pMgr->SetSession(NULL);
    }
}

extern "C" void Release()
{
    for (SessionManagers::Iterator it = g_SessionManagers.First(); !it.IsNull(); ++it) {
        SessionManager* pMgr = it->GetValue();
        if (NULL != pMgr) {
            delete pMgr;
        }
    }
    g_SessionManagers.CleanUp();
    if (NULL != g_pNetIoService) {
        g_pNetIoService->Finalize();
        delete g_pNetIoService;
        g_pNetIoService = NULL;
    }
    g_HandleNameHandleChangedPtr = NULL;
    g_HandleMessagePtr = NULL;
    g_HandleCommandPtr = NULL;
}

#ifdef _CONSOLE
int main(int argc, char* argv[])
{
    Init("centerhub", NULL);
    for (;;) {
        Tick();
        MySleep(10);
    }
    Release();
    return 0;
}
#else
int main(int argc, char* argv[])
{
    return 0;
}
#endif
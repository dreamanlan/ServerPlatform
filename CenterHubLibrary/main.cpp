#include "Thread.h"
#include "ScriptThread.h"
#include "MaintenanceScript.h"
#include "Communication/SessionManager.h"
#include "netioservice.h"
#include "tcp_session.h"

typedef HashtableT<int, SessionManager*> SessionManagers;
static const int MAX_SESSION_MANAGER_NUM = 1024;

static SessionManagers g_SessionManagers;
static int g_ConnectFinish = TRUE;
static NetIoService* g_pNetIoService = NULL;
static event_base* g_EventBase = NULL;

typedef void(*HandleNameHandleChangedPtr)(int worldId, int addOrUpdate, const char* name, int handle);
static HandleNameHandleChangedPtr g_HandleNameHandleChangedPtr = NULL;

typedef void(*HandleMessagePtr)(int worldId, unsigned int seq, int src, int dest, const void* msg, int len);
static HandleMessagePtr g_HandleMessagePtr = NULL;

typedef void(*HandleMessageResultPtr)(int worldId, unsigned int seq, int src, int dest, int result);
static HandleMessageResultPtr g_HandleMessageResultPtr = NULL;

typedef void(*HandleCommandPtr)(int worldId, int src, int dest, const char* msg);
static HandleCommandPtr g_HandleCommandPtr = NULL;

typedef void(*LogHandlerPtr)(const char* log, int len);

int g_IsRun = TRUE;

static void ResetSessionManagers(void)
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
        } else {
          pMgr->SetIpPort(worldId, pIp, port);
          pMgr->UpdateMyName(pName);
          pMgr->SetSession(NULL);
        }
      }
    }
  }
}

void HandleNameHandleChanged(int worldId, bool addOrUpdate, const char* name, int handle)
{
  if (NULL != g_HandleNameHandleChangedPtr && NULL != name) {
    (*g_HandleNameHandleChangedPtr)(worldId, addOrUpdate ? 1 : 0, name, handle);
  }
}

void HandleMessage(int worldId, unsigned int seq, int src, int dest, const char* msg, int len)
{
  if (NULL != g_HandleMessagePtr && NULL != msg) {
    (*g_HandleMessagePtr)(worldId, seq, src, dest, msg, len);
  }
}

void HandleMessageResult(int worldId, unsigned int seq, int src, int dest, int result)
{
  if (NULL != g_HandleMessageResultPtr) {
    (*g_HandleMessageResultPtr)(worldId, seq, src, dest, result);
  }
}

void HandleCommand(int worldId, int src, int dest, const char* msg)
{
  if (NULL != g_HandleCommandPtr && NULL != msg) {
    (*g_HandleCommandPtr)(worldId, src, dest, msg);
  }
}

extern "C" void ReloadConfigScript(void)
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

extern "C" int TargetHandle(int worldId, const char* name)
{
  int handle = 0;
  if (NULL != name) {
    SessionManager* pMgr = g_SessionManagers.Get(worldId);
    if (NULL != pMgr) {
      handle = pMgr->TargetHandle(name);
    }
  }
  return handle;
}

extern "C" int TargetName(int worldId, int handle, char* buf, int len)
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

extern "C" int SendByHandle(int worldId, int dest, const void* data, int len)
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
      int dest = pMgr->TargetHandle(name);
      ret = SendByHandle(worldId, dest, data, len);
    }
  }
  return ret;
}

extern "C" int SendCommandByHandle(int worldId, int dest, const char* command)
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
        if (pSession->Send(reinterpret_cast<const char*>(&msg), sizeof(msg) - 1, command, cmdSize + 1)) {
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
      int dest = pMgr->TargetHandle(name);
      ret = SendCommandByHandle(worldId, dest, command);
    }
  }
  return ret;
}

extern "C" int IsRun(void)
{
  return g_IsRun;
}

extern "C" void Quit(void)
{
  g_IsRun = FALSE;
}

extern "C" void SetCenterLogHandler(LogHandlerPtr logHandler)
{
  SetLogHandler(logHandler);
}

extern "C" void Init(const char* serverType, int argc, char* argv[], HandleNameHandleChangedPtr nameHandleCallback, HandleMessagePtr msgCallback, HandleMessageResultPtr msgResultCallback, HandleCommandPtr cmdCallback)
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

extern "C" void Tick(void)
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

extern "C" void Release(void)
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
int main(char argc, char* argv[])
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
int main(char argc, char* argv[])
{
  return 0;
}
#endif
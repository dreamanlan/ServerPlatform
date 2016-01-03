#include <event2/event.h>
#include <event2/thread.h>
#include "connector.h"
#include "netioservice.h"
#include "tcp_session.h"
#include "Thread.h"
#include "ScriptThread.h"
#include "MaintenanceScript.h"
#include "Communication/SessionManager.h"

static const int MAX_IP_LEN = 128;

static SessionManager* g_pSessionManager = NULL;
static int g_ConnectFinish = TRUE;
static NetIoService* g_pNetIoService = NULL;
static char g_Ip[MAX_IP_LEN + 1] = { 0 };
static int g_Port = 20000;
static event_base* g_EventBase = NULL;
static Connector* g_Connector = NULL;

typedef void(*HandleNameHandleChangedPtr)(int addOrUpdate, const char* name, int handle);
static HandleNameHandleChangedPtr g_HandleNameHandleChangedPtr = NULL;

typedef void(*HandleMessagePtr)(unsigned int seq, int src, int dest, const void* msg, int len);
static HandleMessagePtr g_HandleMessagePtr = NULL;

typedef void(*HandleMessageResultPtr)(unsigned int seq, int src, int dest, int result);
static HandleMessageResultPtr g_HandleMessageResultPtr = NULL;

typedef void(*HandleCommandPtr)(int src, int dest, const char* msg);
static HandleCommandPtr g_HandleCommandPtr = NULL;

typedef void(*LogHandlerPtr)(const char* log, int len);

int g_IsRun = TRUE;

void HandleNameHandleChanged(bool addOrUpdate, const char* name, int handle)
{
  if (NULL != g_HandleNameHandleChangedPtr && NULL != name) {
    (*g_HandleNameHandleChangedPtr)(addOrUpdate ? 1 : 0, name, handle);
  }
}

void HandleMessage(unsigned int seq, int src, int dest, const char* msg, int len)
{
  if (NULL != g_HandleMessagePtr && NULL != msg) {
    (*g_HandleMessagePtr)(seq, src, dest, msg, len);
  }
}

void HandleMessageResult(unsigned int seq, int src, int dest, int result)
{
  if (NULL != g_HandleMessageResultPtr) {
    (*g_HandleMessageResultPtr)(seq, src, dest, result);
  }
}

void HandleCommand(int src, int dest, const char* msg)
{
  if (NULL != g_HandleCommandPtr && NULL != msg) {
    (*g_HandleCommandPtr)(src, dest, msg);
  }
}

bool OnConnectFinish(TcpSession* session, int err)
{
  if (NULL != session) {
    char buf[1025];
    session->PeerInfo(buf, 1024);
    __Internal_Log("Connect: %s", buf);

    g_pSessionManager->SetSession(session);
  } else {
    __Internal_Log("Connect failed:%d", err);

    g_pSessionManager->SetSession(NULL);
  }
  g_ConnectFinish = TRUE;
  return true;
}

extern "C" void ReloadConfigScript(void)
{
  ReloadConfigState();

  const char* pIp = GetConfig("ip");
  if (NULL == pIp)
    pIp = "127.0.0.1";
  const char* pPort = GetConfig("port");
  int port = 20000;
  if (NULL != pPort)
    port = (int)atoi(pPort);

  tsnprintf(g_Ip, sizeof(g_Ip), "%s", pIp);
  g_Port = port;

  if (NULL != g_pSessionManager) {
    g_pSessionManager->UpdateMyName();
  }
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

extern "C" int TargetHandle(const char* name)
{
  int handle = 0;
  if (NULL != g_pSessionManager && NULL != name) {
    handle = g_pSessionManager->TargetHandle(name);
  }
  return handle;
}

extern "C" int TargetName(int handle, char* buf, int len)
{
  int ret = 0;
  if (NULL != g_pSessionManager && NULL != buf) {
    const char* name = g_pSessionManager->TargetName(handle);
    if (NULL != name) {
      tsnprintf(buf, len, "%s", name);
      ret = 1;
    }
  }
  return ret;
}

extern "C" int SendByHandle(int dest, const void* data, int len)
{
  int ret = 0;
  if (NULL != g_pSessionManager && NULL != data) {
    TcpSession* pSession = g_pSessionManager->GetSession();
    if (NULL != pSession) {
      MessageTransmit msg;
      msg.m_Sequence = g_pSessionManager->SessionSequence();
      msg.m_Src = g_pSessionManager->MyHandle();
      msg.m_Dest = dest;
      if (msg.IsValid() && pSession->Send(reinterpret_cast<const char*>(&msg), sizeof(msg) - 1, reinterpret_cast<const char*>(data), len)) {
        g_pSessionManager->IncSessionSequence();
        ret = 1;
      }
    }
  }
  return ret;
}

extern "C" int SendByName(const char* name, const void* data, int len)
{
  int ret = 0;
  if (NULL != g_pSessionManager && NULL != name && NULL != data) {
    int dest = g_pSessionManager->TargetHandle(name);
    ret = SendByHandle(dest, data, len);
  }
  return ret;
}

extern "C" int SendCommandByHandle(int dest, const char* command)
{
  int ret = 0;
  if (NULL != g_pSessionManager && NULL != command) {
    TcpSession* pSession = g_pSessionManager->GetSession();
    if (NULL != pSession) {
      size_t cmdSize = strlen(command);
      MessageCommand msg;
      msg.m_Src = g_pSessionManager->MyHandle();
      msg.m_Dest = dest;
      if (pSession->Send(reinterpret_cast<const char*>(&msg), sizeof(msg) - 1, command, cmdSize + 1)) {
        ret = 1;
      }
    }
  }
  return ret;
}

extern "C" int SendCommandByName(const char* name, const char* command)
{
  int ret = 0;
  if (NULL != g_pSessionManager && NULL != name && NULL != command) {
    int dest = g_pSessionManager->TargetHandle(name);
    ret = SendCommandByHandle(dest, command);
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

  const char* pIp = GetConfig("ip");
  if (NULL == pIp)
    pIp = "127.0.0.1";
  const char* pPort = GetConfig("port");
  int port = 20000;
  if (NULL != pPort)
    port = (int)atoi(pPort);

  tsnprintf(g_Ip, sizeof(g_Ip), "%s", pIp);
  g_Port = port;

  g_HandleNameHandleChangedPtr = nameHandleCallback;
  g_HandleMessagePtr = msgCallback;
  g_HandleMessageResultPtr = msgResultCallback;
  g_HandleCommandPtr = cmdCallback;

  g_pSessionManager = new SessionManager();
  if (NULL != g_pSessionManager) {
    g_pSessionManager->Init();
    g_pSessionManager->UpdateMyName();
  }

  g_pNetIoService = new NetIoService();
  g_pNetIoService->Init();
  g_EventBase = g_pNetIoService->EventBase();
  if (NULL != g_EventBase) {
    g_Connector = new Connector(g_EventBase);
  }
}

extern "C" void Tick(void)
{
  if (NULL != g_pSessionManager && NULL != g_pNetIoService && NULL != g_EventBase && NULL != g_Connector) {
    if (g_pSessionManager->GetSession() == NULL && g_ConnectFinish == TRUE) {
      g_ConnectFinish = FALSE;
      try {
        g_Connector->Connect(g_Ip, g_Port, OnConnectFinish);
      } catch (std::exception&) {
        __Internal_Log("Connect exception !!!");
      } catch (...) {
        g_ConnectFinish = TRUE;
      }
    }

    g_pNetIoService->Poll();
    g_pSessionManager->Tick();
  }
}

extern "C" int IsConnected(void)
{
  int ret = 0;
  if (NULL != g_pSessionManager) {
    ret = (NULL != g_pSessionManager->GetSession() ? 1 : 0);
  }
  return ret;
}

extern "C" void Disconnect(void)
{
  if (NULL != g_pSessionManager) {
    g_pSessionManager->SetSession(NULL);
  }
}

extern "C" void Release(void)
{
  if (NULL != g_pSessionManager) {
    delete g_pSessionManager;
    g_pSessionManager = NULL;
  }
  if (NULL != g_Connector) {
    delete g_Connector;
    g_Connector = NULL;
  }
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
  Init("centerclient", NULL);
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
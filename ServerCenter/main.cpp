
#include <event2/event.h>
#include <event2/thread.h>
#include "Thread.h"
#include "acceptor.h"
#include "netioservice.h"
#include "tcp_session.h"
#include "MaintenanceScript.h"
#include "ScriptThread.h"
#include "Communication/SessionManager.h"

using namespace net_base;

int g_IsRun = TRUE;
static SessionManager* g_pSessionManager = NULL;

bool OnAccept(TcpSession* session)
{
  if (NULL != session){
    char buf[1025];
    session->PeerInfo(buf, 1024);
    __Internal_Log("Accept: %s", buf);

    g_pSessionManager->AddSession(session);
  }
  return true;
}

int main(int argc, char* argv[])
{
#if defined(__LINUX__)
  signal(SIGPIPE, SIG_IGN);
#endif

  InitConfigState("center", argc - 1, argc > 1 ? &argv[1] : NULL);

  const char* pIp = GetConfig("ip");
  if (NULL == pIp)
    pIp = "0.0.0.0";
  const char* pPort = GetConfig("port");
  int port = 20000;
  if (NULL != pPort)
    port = (int)atoi(pPort);

  g_pSessionManager = new SessionManager();
  if (NULL == g_pSessionManager){
    __Internal_Log("Can't alloc SessionManager !!!");
    exit(-1);
  }
  g_pSessionManager->Init();

  g_pScriptThread = new ScriptThread();
  if (NULL != g_pScriptThread){
    g_pScriptThread->start();
  }

  NetIoService ioservice;
  ioservice.Init();
  event_base* evbase = ioservice.EventBase();
  if (NULL != evbase){
    Acceptor acceptor(evbase);
    try{
      acceptor.Listen(pIp, port, OnAccept);
    } catch (std::exception&){
      __Internal_Log("Accept failed !!!");
    } catch (...){
    }
    while (g_IsRun){
      ioservice.Poll();
      g_pSessionManager->Tick();
      MySleep(10);
    }
  }
  delete g_pSessionManager;
  g_pSessionManager = NULL;
  ioservice.Finalize();

  if (NULL != g_pScriptThread){
    g_pScriptThread->MarkWaitingQuit();
    g_pScriptThread->stop();
    while (g_CreateThreadCount > g_QuitThreadCount){
      MySleep(100);
    }
    delete g_pScriptThread;
    g_pScriptThread = NULL;
  }
  return 0;
}
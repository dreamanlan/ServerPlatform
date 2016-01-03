
// sys
#include <vector>
#include <memory>
#include <algorithm>
#include <string.h>
// 3rd-party
#include <event2/event.h>
#include <event2/thread.h>
// mine
#include "netioservice.h"
#include "acceptor.h"
#include "connector.h"

namespace net_base
{
  struct NetIoService::Impl
  {
    event_base* evbase;

    Impl(void) :evbase(NULL)
    {
#ifdef WIN32
      WORD wVersionRequested;
      WSADATA wsaData;
      int32_t nError = 0;

      wVersionRequested = MAKEWORD(2, 2);
      nError = WSAStartup(wVersionRequested, &wsaData);
      if (nError != 0) {
        __Internal_Log("WSAStartup error !");
      }
#endif
    }
    ~Impl(void)
    {
#ifdef WIN32
      if (WSACleanup() != 0){
        __Internal_Log("WSACleanup error !");
      }
#endif
    }
  };

  NetIoService::NetIoService(void) : m_Impl(new Impl()) {}

  NetIoService::~NetIoService(void) { delete m_Impl; }

  void NetIoService::Init(void)
  {
#ifdef WIN32
    evthread_use_windows_threads();
#else
    evthread_use_pthreads();
#endif

    m_Impl->evbase = event_base_new();
  }

  void NetIoService::Poll(void)
  {
    // run dispatch
    try {
      event_base_loop(m_Impl->evbase, EVLOOP_NONBLOCK);
    }
    catch (std::exception& e) {
      net_base::__Internal_Log(e.what(), strlen(e.what()));
    }
    catch (...) {}
  }

  void NetIoService::Loop(void)
  {
    // add a nothing-to-do event to keep the loop not exiting
    event* ev_work = event_new(m_Impl->evbase, -1, EV_READ | EV_PERSIST, NULL, NULL);
    event_add(ev_work, NULL);
    // run dispatch
    try {
      event_base_dispatch(m_Impl->evbase);
    }
    catch (std::exception& e) {
      net_base::__Internal_Log(e.what(), strlen(e.what()));
    }
    catch (...) {}
    // clean up everything
    event_free(ev_work);
  }

  void NetIoService::Finalize(void)
  {
    event_base_free(m_Impl->evbase);
  }

  void NetIoService::Shutdown(void)
  {
    event_base_loopbreak(m_Impl->evbase);
  }

  event_base* NetIoService::EventBase(void) const
  {
    return m_Impl->evbase;
  }
} // namespace net_base


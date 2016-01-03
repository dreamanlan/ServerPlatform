
// sys
#ifdef WIN32
#include <WinSock2.h>
#include <ws2tcpip.h>
#else
#include <arpa/inet.h>
#endif
#include <string.h>
#include <errno.h>
// 3rd-party
#include <event2/listener.h>
#include <event2/util.h>
#include <event2/bufferevent.h>
// mine
#include "acceptor.h"
#include "tcp_session.h"
#include "exception.h"

namespace net_base
{
  struct Acceptor::Impl
  {
    event_base* evt_base;
    evconnlistener* lev;
    NewConnectionCallback new_conn_cb;

    Impl(event_base* evbase) : lev(NULL), evt_base(evbase) {}
    ~Impl(void)
    {
      if (lev)
        evconnlistener_free(lev);
    }
  };
} // namespace net_base

namespace
{

  void OnAccept(evconnlistener* lev, evutil_socket_t fd, sockaddr* sin,
    int socklen, void* arg)
  {
    using namespace net_base;
    Acceptor::Impl* impl = reinterpret_cast<Acceptor::Impl*>(arg);
    bufferevent* bev = bufferevent_socket_new(impl->evt_base, fd,
      BEV_OPT_CLOSE_ON_FREE |
      BEV_OPT_THREADSAFE |
      BEV_OPT_DEFER_CALLBACKS);
    if (!bev) {
      __Internal_Log("OnAccept: failed to create bev, fd(%d)", fd);
      return;
    }

    (impl->new_conn_cb)(new TcpSession(bev));
  }

  void OnError(struct evconnlistener* lev, void* arg)
  {
    using namespace net_base;
    if (lev) {
      evutil_socket_t fd = evconnlistener_get_fd(lev);
      sockaddr_in sin;
      socklen_t len = sizeof(sin);
      char ip[16] = { 0 };
      if (0 == getsockname(fd, (sockaddr*)&sin, &len) &&
        NULL != evutil_inet_ntop(AF_INET, &sin.sin_addr, ip, 16)) {
        __Internal_Log("error occur at listener(%s:%u)", ip, sin.sin_port);
      } else {
        char buffer[512] = { 0 };
#ifdef WIN32
        strerror_s(buffer, errno);
#else
        strerror_r(errno, buffer, 512);
#endif
        __Internal_Log("error occur at listener(%s)", buffer);
      }

      evconnlistener_free(lev);
    }
    Acceptor::Impl* impl = reinterpret_cast<Acceptor::Impl*>(arg);
    impl->lev = NULL;
  }

}

namespace net_base {
  Acceptor::Acceptor(event_base* evbase)
    : m_Impl(new Impl(evbase))
  {
  }

  Acceptor::~Acceptor(void)
  {
    if (NULL != m_Impl){
      delete m_Impl;
      m_Impl = NULL;
    }
  }

  void Acceptor::Listen(const char* ip, int port, const NewConnectionCallback& cb)
  {
    sockaddr_in sin;
    memset(&sin, 0, sizeof(sin));
    if (1 != evutil_inet_pton(AF_INET, ip, &sin.sin_addr))
      throw sysapi_error(errno);

    sin.sin_family = AF_INET;
    sin.sin_port = htons(port);

    Listen((sockaddr*)&sin, sizeof(sin), cb);
  }

  void Acceptor::Listen(const char* ip_port, const NewConnectionCallback& cb)
  {
    sockaddr_in sin;
    int len = sizeof(sin);
    if (0 != evutil_parse_sockaddr_port(ip_port, (sockaddr*)&sin, &len))
      throw parse_ip_port_error(ip_port);
    Listen((sockaddr*)&sin, len, cb);
  }

  void Acceptor::Listen(const sockaddr* sa, int socklen, const NewConnectionCallback& cb)
  {
    m_Impl->new_conn_cb = cb;
    m_Impl->lev = evconnlistener_new_bind(m_Impl->evt_base, OnAccept, m_Impl,
      LEV_OPT_CLOSE_ON_FREE | LEV_OPT_REUSEABLE |
      LEV_OPT_THREADSAFE,
      -1, sa, socklen);
    if (!m_Impl->lev) throw create_listener_failed();
    evconnlistener_set_error_cb(m_Impl->lev, OnError);
  }
} // namespace net_base


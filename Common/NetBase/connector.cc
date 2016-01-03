
// sys
#ifdef WIN32
#include <WinSock2.h>
#else
#include <arpa/inet.h>
#endif
#include <errno.h>
#include <string.h>
// 3rd-party
#include <event2/bufferevent.h>
#include <event2/util.h>
// mine
#include "connector.h"
#include "tcp_session.h"
#include "exception.h"

namespace net_base
{
  struct Connector::Impl
  {
    event_base* m_EventBase;
    ConnectFinishCallback m_Callback;

    Impl(void) :m_EventBase(NULL)
    {}
    ~Impl(void)
    {
    }
  };
}

namespace
{
  void ConnEvt(bufferevent* bev, short what, void* ctx) {
    using namespace net_base;
    Connector::Impl* obj = reinterpret_cast<Connector::Impl*>(ctx);
    if (bev) {
      int err = 0;
      TcpSession* session = NULL;
      if (what & BEV_EVENT_CONNECTED) {
        session = new TcpSession(bev);
      } else {
        evutil_socket_t fd = bufferevent_getfd(bev);
        (void)fd;
        err = evutil_socket_geterror(fd);
      }
      if (!obj->m_Callback.isNull())
        (obj->m_Callback)(session, err);
      if (!(what & BEV_EVENT_CONNECTED))
        bufferevent_free(bev);
    }
  }
}

namespace net_base
{
  void Connector::Connect(const char* ip, int port, ConnectFinishCallback callback) {
    sockaddr_in sin;
    memset(&sin, 0, sizeof(sin));
    if (1 != evutil_inet_pton(AF_INET, ip, &sin.sin_addr))
      throw sysapi_error(errno);
    sin.sin_family = AF_INET;
    sin.sin_port = htons(port);

    Connect((sockaddr*)&sin, sizeof(sin), callback);
  }

  void Connector::Connect(const char* ip_port, ConnectFinishCallback callback) {
    sockaddr_in sin;
    int len = sizeof(sin);
    if (0 != evutil_parse_sockaddr_port(ip_port, (sockaddr*)&sin, &len))
      throw parse_ip_port_error(ip_port);

    Connect((sockaddr*)&sin, len, callback);
  }

  void Connector::Connect(sockaddr* sa, int socklen, ConnectFinishCallback callback) {
    m_Impl->m_Callback = callback;
    bufferevent* bev = bufferevent_socket_new(m_Impl->m_EventBase, -1,
      BEV_OPT_CLOSE_ON_FREE |
      BEV_OPT_THREADSAFE |
      BEV_OPT_DEFER_CALLBACKS);
    if (!bev) throw create_socket_failed();
    bufferevent_setcb(bev, NULL, NULL, ConnEvt, m_Impl);
    if (0 != bufferevent_socket_connect(bev, sa, socklen))
      throw connect_failed();
  }

  Connector::Connector(event_base* evbase) :m_Impl(new Impl())
  {
    m_Impl->m_EventBase = evbase;
  }

  Connector::~Connector(void)
  {
    if (NULL != m_Impl){
      delete m_Impl;
      m_Impl = NULL;
    }
  }
} // namespace net_base


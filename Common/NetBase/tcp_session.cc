
// sys
#include <stdint.h>
#include <errno.h>
#include <string.h>
#include <assert.h>
// 3rd-party
#include <event2/buffer.h>
#include <event2/bufferevent.h>
#include <event2/event.h>
// mine
#include "Atomic.h"
#include "tcp_session.h"
#include "exception.h"

#ifdef WIN32
#include <windows.h>
#include <ws2tcpip.h>
#endif

namespace net_base
{
  struct TcpSession::Impl
  {
    TcpSession* session;
    bufferevent* bev;
    ReceiveCallback recv_cb;
    ErrorCallback error_cb;

    // explain ref_cnt:
    // TcpSession hold one ref of IMPL, and when event_cb is call, inc this ref 
    // by one. because TcpSession is likely to commit suicide with error_cb, which
    // will cause the upcomming bufferevent_free on bev lead to undefine behavior.
    // with the use of ref_cnt, IMPL will not be released with the ctor of
    // TcpSession while it's called within event_cb, and it will be released 
    // when leaving the scoped of event_cb 
    volatile int32_t ref_cnt;

    void inc_ref() {
      lock_free_utility::increment(&ref_cnt);
    }

    void dec_ref() {
      assert(ref_cnt > 0);
      if(lock_free_utility::decrement(&ref_cnt)==0)
        delete this;
    }
    
    Impl(TcpSession* s, bufferevent* _bev) :session(s), bev(_bev), ref_cnt(1)
    {}

    ~Impl(void)
    {
      if (bev)
        bufferevent_free(bev);
    }
  };
} // namespace net_base

namespace
{
  void recv_cb(bufferevent* bev, void* ctx)
  {
    using namespace net_base;
    TcpSession::Impl* impl = reinterpret_cast<TcpSession::Impl*>(ctx);
    impl->inc_ref();
    if (bev) {
      evbuffer* input = bufferevent_get_input(bev);
      do {
        int input_len = evbuffer_get_length(input);
        if (input_len <= sizeof(int))
          break;
        int size;
        if (evbuffer_copyout(input, &size, sizeof(size)) < sizeof(size))
          break;

        size = ntohl(size);
        if (input_len < size + sizeof(size))
          break;

        if (-1 == evbuffer_drain(input, sizeof(size)))
          break;

        try {
          ReceiveCallback recv_cb = impl->recv_cb;
          if (!recv_cb.isNull()) {
            const char* data = (const char*)evbuffer_pullup(input, size);
            (recv_cb)(impl->session, data, size);
          }
        } catch (...) {}

        evbuffer_drain(input, size);
      } while (true);
    }
    impl->dec_ref();
  }

  void event_cb(bufferevent* bev, short what, void* ctx)
  {
    using namespace net_base;
    TcpSession::Impl* impl = reinterpret_cast<TcpSession::Impl*>(ctx);
    impl->inc_ref();
    if (bev) {
      if (what & BEV_EVENT_EOF || what & BEV_EVENT_ERROR) {
        __Internal_Log("tcp session event BEV_EVENT_EOF or BEV_EVENT_ERROR, what = %d", what);
      } else {
        __Internal_Log("tcp session event other type, what = %d", what);
      }
      try {
        ErrorCallback error_cb = impl->error_cb;
        if (!error_cb.isNull()) {
          evutil_socket_t fd = bufferevent_getfd(bev);
          (void)fd;
          (error_cb)(impl->session, evutil_socket_geterror(fd));
        }
      } catch (...) {}
      bufferevent_free(bev);
    }
    impl->bev = NULL;
    impl->dec_ref();
  }
}

namespace net_base
{
  TcpSession::TcpSession(bufferevent* bev)
    : m_Impl( new Impl(this,bev))
  {
    bufferevent_setcb(m_Impl->bev, recv_cb, NULL, event_cb, m_Impl);
    bufferevent_enable(m_Impl->bev, EV_WRITE);
  }

  TcpSession::~TcpSession(void)
  {
    if (NULL != m_Impl){
      m_Impl->dec_ref();
    }
  }
  
  void TcpSession::SetCB(const ReceiveCallback& recv_cb, const ErrorCallback& error_cb)
  {
    m_Impl->recv_cb = recv_cb;
    m_Impl->error_cb = error_cb;

    if (m_Impl->bev) {
      if (!recv_cb.isNull())
        bufferevent_enable(m_Impl->bev, EV_READ);
      else
        bufferevent_disable(m_Impl->bev, EV_READ);
    }
  }

  bool TcpSession::Send(const char* header, int header_size, const char* data, int size)
  {
    if (!m_Impl->bev){
      __Internal_Log("Invalid Session !!!");
      return false;
    }

    int nsize = htonl(header_size + size);
    bufferevent_lock(m_Impl->bev);
    if (0 != bufferevent_write(m_Impl->bev, (const void*)&nsize, sizeof(nsize)) ||
      0 != bufferevent_write(m_Impl->bev, header, header_size) || 
      0 != bufferevent_write(m_Impl->bev, data, size)) {
      bufferevent_unlock(m_Impl->bev);
      return false;
    }
    bufferevent_unlock(m_Impl->bev);
    return true;
  }

  bool TcpSession::Send(const char* full_data, int size)
  {
    if (!m_Impl->bev){
      __Internal_Log("Invalid Session !!!");
      return false;
    }

    int nsize = htonl(size);
    bufferevent_lock(m_Impl->bev);
    if (0 != bufferevent_write(m_Impl->bev, (const void*)&nsize, sizeof(nsize)) ||
      0 != bufferevent_write(m_Impl->bev, full_data, size)) {
      bufferevent_unlock(m_Impl->bev);
      return false;
    }
    bufferevent_unlock(m_Impl->bev);
    return true;
  }

  bool TcpSession::Send(evbuffer* evbuf)
  {
    if (!m_Impl->bev){
      __Internal_Log("Invalid Session !!!");
      return false;
    }

    int size = evbuffer_get_length(evbuf);
    int nsize = htonl((int)size);
    bufferevent_lock(m_Impl->bev);
    if (0 != bufferevent_write(m_Impl->bev, (const void*)&nsize, sizeof(nsize)) ||
      0 != bufferevent_write_buffer(m_Impl->bev, evbuf)) {
      bufferevent_unlock(m_Impl->bev);
      return false;
    }
    bufferevent_unlock(m_Impl->bev);
    return true;
  }

  bool TcpSession::PeerInfo(char* buf, size_t size) const
  {
    if (!m_Impl->bev) return false;

    evutil_socket_t fd = bufferevent_getfd(m_Impl->bev);
    sockaddr_in sin;
    socklen_t len = sizeof(sin);
    char ip[16] = { 0 };
    sockaddr_in sin2;
    socklen_t len2 = sizeof(sin2);
    char ip2[16] = { 0 };
    if (0 == getsockname(fd, (sockaddr*)&sin, &len) &&
      NULL != evutil_inet_ntop(AF_INET, &sin.sin_addr, ip, 16) && 
      0 == getpeername(fd, (sockaddr*)&sin2, &len2) && 
      NULL != evutil_inet_ntop(AF_INET, &sin2.sin_addr, ip2, 16)) {
      unsigned short port = ntohs(sin.sin_port);
      unsigned short port2 = ntohs(sin2.sin_port);
#ifdef WIN32
      sprintf_s(buf, size, "%s:%u<->%s:%u", ip, port, ip2, port2);
#else
      snprintf(buf, size, "%s:%u<->%s:%u", ip, port, ip2, port2);
#endif
      return true;
    }
    return false;
  }

  int32_t TcpSession::GetFD(void) const
  {
    if (!m_Impl->bev) return -1;
    return bufferevent_getfd(m_Impl->bev);
  }
  
  bool TcpSession::IsValid(void) const
	{
		return NULL!=m_Impl->bev;
	}  	

} // namespace net_base


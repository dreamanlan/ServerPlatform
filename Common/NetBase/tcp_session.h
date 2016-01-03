
#ifndef NETBASE_TCP_SESSION_H
#define NETBASE_TCP_SESSION_H

// sys
#include <stdint.h>
#include <memory>
// mine
#include "util.h"
#include "Delegation.h"

struct evbuffer;
struct bufferevent;

namespace net_base
{
  class TcpSession;
  typedef Delegation3<bool, TcpSession*, const char*, int> ReceiveCallback;
  typedef Delegation2<bool, TcpSession*, int> ErrorCallback;

  class TcpSession
  {
  public:
    TcpSession(bufferevent* bev);
    ~TcpSession(void);

    void SetCB(const ReceiveCallback& recv_cb, const ErrorCallback& error_cb);
    bool Send(const char* header, int header_size, const char* full_data, int size);
    bool Send(const char* full_data, int size);
    bool Send(evbuffer* evbuf);
    bool PeerInfo(char* buf, size_t size) const;
    int32_t GetFD(void) const;
    bool IsValid(void) const;

  private:
    NONCOPYABLE(TcpSession);
  public:
    struct Impl;
  private:
    Impl* m_Impl;
  };
} // namespace net_base

#endif // NETBASE_TCP_SESSION_H


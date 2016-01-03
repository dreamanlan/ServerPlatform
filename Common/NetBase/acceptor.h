
#ifndef ACCEPTOR_H
#define ACCEPTOR_H

// sys
#include <stdint.h>
#include <functional>
#include <memory>
// mine
#include "util.h"
#include "Delegation.h"

struct sockaddr;
struct event_base;
struct evconnlistener;

namespace net_base
{
  class TcpSession;
  typedef Delegation1<bool, TcpSession*> NewConnectionCallback;

  class Acceptor
  {
  public:
    void Listen(const char* ip, int port, const NewConnectionCallback& cb);

    /*!
     * listen on specific ip/port, call cb when new connection arrived
     * ip_port format:
     *  IPv4Address:port
     *  IPv4Address
     * If no port is specified, the port in the output is set to 0
     *
     */
    void Listen(const char* ip_port, const NewConnectionCallback& cb);

  public:
    Acceptor(event_base* evbase);
    ~Acceptor(void);

  private:
    void Listen(const sockaddr* sa, int socklen, const NewConnectionCallback& cb);

    NONCOPYABLE(Acceptor);
  public:
    struct Impl;
  private:
    Impl* m_Impl;
  };
} // namespace net_base

#endif // ACCEPTOR_H


#ifndef CONNECTOR_H
#define CONNECTOR_H

// sys
#include <cstdint>
#include <memory>
// mine
#include "util.h"
#include "Delegation.h"

struct sockaddr;
struct event_base;

namespace net_base {
  class TcpSession;
  typedef Delegation2<bool, TcpSession*, int> ConnectFinishCallback;

  class Connector {
  public:
    void Connect(const char* ip, int port, ConnectFinishCallback callback);
    void Connect(const char* ip_port, ConnectFinishCallback callback);
  public:
    Connector(event_base* evbase);
    ~Connector(void);
  private:
    void Connect(sockaddr* sa, int socklen, ConnectFinishCallback callback);

    NONCOPYABLE(Connector);
  public:
    struct Impl;
  private:
    Impl* m_Impl;
  };
} // namespace net_base

#endif // CONNECTOR_H


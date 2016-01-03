#ifndef NETIOSERVICE_H
#define NETIOSERVICE_H

// sys
#include <stdint.h>
#include <memory>
// mine
#include "util.h"

struct event_base;

namespace net_base
{
  class NetIoService
  {
  public:
    NetIoService(void);
    ~NetIoService(void);
    void Init(void);
    void Poll(void);
    void Loop(void);
    void Finalize(void);
    void Shutdown(void);
    event_base* EventBase(void) const;

  private:
    NONCOPYABLE(NetIoService);

    struct Impl;
    Impl* m_Impl;
  };
} // namespace net_base

#endif // NETIOSERVICE_H


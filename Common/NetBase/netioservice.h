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
        NetIoService();
        ~NetIoService();
        void Init();
        void Poll();
        void Loop();
        void Finalize();
        void Shutdown();
        event_base* EventBase() const;

    private:
        NONCOPYABLE(NetIoService);

        struct Impl;
        Impl* m_Impl;
    };
} // namespace net_base

#endif // NETIOSERVICE_H


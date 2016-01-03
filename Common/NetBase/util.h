
#ifndef NETBASE_UTIL_H
#define NETBASE_UTIL_H

#ifndef NONCOPYABLE
#define NONCOPYABLE(classname) \
    classname(const classname&); \
    classname& operator=(const classname&);
#endif

namespace net_base {
typedef void (*LogHandler)(const char* log, int len);

void SetLogHandler(LogHandler log_handler);

void __Internal_Log(const char* format, ...);
} // namespace net_base

#endif // NETBASE_UTIL_H



// sys
#include <stdarg.h>
#include <stdio.h>
#include <stdint.h>
#include <time.h>
// mine
#include "util.h"

namespace net_base {
static LogHandler g_log_handler = 0;

static void DefaultLog(const char* msg, size_t len) {
  struct tm tmTime;
  time_t t = time(NULL);
#if WIN32
  localtime_s(&tmTime, &t);
#else
  localtime_r(&t, &tmTime);
#endif
  printf("[%u-%u-%u_%u-%u-%u]: %s\n", tmTime.tm_year + 1900, tmTime.tm_mon + 1, tmTime.tm_mday, tmTime.tm_hour, tmTime.tm_min, tmTime.tm_sec, msg);
  fflush(stdout);
}

void SetLogHandler(LogHandler log_handler) {
  g_log_handler = log_handler;
}

void __Internal_Log(const char* format, ...) {
  char str[256] = {0};
  size_t sz = 256;

  va_list vl;
  va_start(vl, format);
  int ret = vsnprintf(str, sz, format, vl);
  va_end(vl);

  if (ret > 0) {
    size_t psz = static_cast<size_t>(ret);
    if (psz >= sz) {
      psz = sz;
      str[psz - 1] = '\0';
      if (g_log_handler)
        g_log_handler(str, psz);
      else
        DefaultLog(str, psz);
    } else {
      if (g_log_handler)
        g_log_handler(str, psz);
      else
        DefaultLog(str, psz);
    }
  }
}

} // namespace net_base



// sys
#include <string.h>
// mine
#include "exception.h"

namespace net_base {

const char* sysapi_error::what() { 
  if (buffer[0] == '\0') {
#ifndef WIN32
    strerror_r(errno_, buffer, 512);
#else
    strerror_s(buffer, errno_);
#endif
  }
  return buffer;
};

parse_ip_port_error::parse_ip_port_error(const char* ip_port) {
  strncpy(buffer, ip_port, 32);
}

} // namespace net_base


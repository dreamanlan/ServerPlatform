
#ifndef NETBASE_EXCEPTION_H
#define NETBASE_EXCEPTION_H

#include <stdexcept>

namespace net_base {

#define SIMPLE_DEF(classname) \
  class classname : public std::exception { \
   public: virtual const char* what() const throw() { return #classname; } };

#define DEF(classname) class classname : public std::exception

SIMPLE_DEF(create_socket_failed);
SIMPLE_DEF(connect_failed);
SIMPLE_DEF(create_listener_failed);

DEF(sysapi_error) {
 public:
  sysapi_error(int errno_copy) : errno_(errno_copy) {
    buffer[0] = '\0'; 
  }
  virtual const char* what();
 private:
  int errno_;
  char buffer[512];
};

DEF(parse_ip_port_error) {
 public:
  parse_ip_port_error(const char* ip_port);
  virtual const char* what() { return buffer; }
 private:
  char buffer[32];
};


#undef SIMPLE_DEF
#undef DEF

} // namespace net_base

#endif // NETBASE_EXCEPTION_H

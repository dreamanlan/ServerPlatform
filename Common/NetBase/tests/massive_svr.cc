
// sys
#include <signal.h>
#include <stdlib.h>
#include <vector>
#include <stdio.h>
#include <assert.h>
#include <sys/syscall.h>
#include <stdarg.h>
// 3rd-party
#include <boost/crc.hpp>
// mine
#include "netbase.h"
#include "listener.h"
#include "tcp_session.h"

#define gettid() syscall(__NR_gettid)

using namespace std::placeholders;
using namespace net_base;

static volatile sig_atomic_t kFinish = 0;
static void sig_int(int);

static void NewSession(TcpSessionPtr session_ptr);
static void OnSessionRecv(TcpSessionPtr self, const char* data, uint16_t size);
static void OnSessionError(TcpSessionPtr self, int sock_ec);

static boost::crc_32_type::value_type data_crc;
static size_t data_sz;

static void DataInit(void);
static bool DataVerify(const char* buffer, size_t sz);

static void PrintHelp(const char* name);
static void LOG(const char* format, ...);

int main(int argc, char* argv[]) {
  struct sigaction act;
  act.sa_handler = sig_int;
  sigfillset(&act.sa_mask);
  act.sa_flags = 0;
  sigaction(SIGINT, &act, NULL);

  if (argc != 3) {
    PrintHelp(argv[0]);
    return 1;
  }

  uint16_t start_port = (uint16_t)atoi(argv[1]);
  uint16_t port_num = (uint16_t)atoi(argv[2]);

  DataInit();
  NetBase netbase;
  netbase.Run(5);

  std::vector<ListenerPtr> listeners;
  for (uint16_t i = 0; i < port_num; i++) {
    uint16_t port = start_port + i;
    LOG("listen on 0.0.0.0:%u\n", port);
    ListenerPtr listener_ptr = netbase.NewListener();
    listener_ptr->Listen("0.0.0.0", port, NewSession);
    listeners.push_back(std::move(listener_ptr));
  }

  while(0 == kFinish)
    sleep(1);

  puts("SHUTDOWN");
  for (auto& ptr : listeners)
    ptr.reset();
  netbase.Shutdown();
  return 0;
}

static void sig_int(int) {
  kFinish = 1;
}

static void DataInit(void) {
  FILE* fp = fopen("data", "rb");
  assert(fp);
  fseek(fp, 0, SEEK_END);
  long size = ftell(fp);
  assert(size >= 0);
  fseek(fp, 0, SEEK_SET);

  data_sz = (size_t)size;

  char* buffer = new char[size];
  fread(buffer, 1, size, fp);
  boost::crc_32_type crc32;
  crc32.process_bytes(buffer, size);
  data_crc = crc32.checksum();
  delete [] buffer;
}

static bool DataVerify(const char* buffer, size_t sz) {
  boost::crc_32_type crc32;
  crc32.process_bytes(buffer, sz);
  return data_crc == crc32.checksum();
}

static void NewSession(TcpSessionPtr session_ptr) {
  char buf[32];
  session_ptr->PeerInfo(buf, 32);
  LOG("new session: %s\n", buf);
  session_ptr->SetCB(std::bind(OnSessionRecv, session_ptr, _1, _2),
                     std::bind(OnSessionError, session_ptr, _1));
}

static void OnSessionRecv(TcpSessionPtr self, const char* data, uint16_t size) {
  if (size < data_sz)
    return;

  assert(DataVerify(data, size));

  char buf[32];
  self->PeerInfo(buf, 32);
  LOG("[%d>%s DONE AND ECHO\n", gettid(), buf);

  self->Send(data, size); 
}

static void OnSessionError(TcpSessionPtr self, int sock_ec) {
  char buf[32];
  self->PeerInfo(buf, 32);
  LOG("[%d>%s DISCONNECT\n", gettid(), buf);

  self->SetCB(nullptr, nullptr);
}

static void PrintHelp(const char* name) {
  printf("%s start_port port_num\n", name);
}

static void LOG(const char* format, ...) {
  static uint8_t lock = 0;

  while(!__sync_bool_compare_and_swap(&lock, 0, 1));
  va_list vl;
  va_start(vl, format);
  vprintf(format, vl);
  va_end(vl);
  __sync_bool_compare_and_swap(&lock, 1, 0);
}

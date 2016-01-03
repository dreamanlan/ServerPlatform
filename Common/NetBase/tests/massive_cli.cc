
// sys
#include <signal.h>
#include <stdlib.h>
#include <vector>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <sys/syscall.h>
// 3rd-party
#include <boost/crc.hpp>
#include <event2/util.h>
// mine
#include "netbase.h"
#include "connector.h"
#include "tcp_session.h"

#define gettid() syscall(__NR_gettid)

using namespace std::placeholders;
using namespace net_base;

static volatile sig_atomic_t kFinish = 0;
static void sig_int(int);

static boost::crc_32_type::value_type data_crc;
static size_t data_sz;
static char* data_buf;

static void DataInit(void);
static bool DataVerify(const char* buffer, size_t sz);

static void PrintHelp(const char* name);
static void LOG(const char* format, ...);

struct Client {
  int echo_times;
  TcpSessionPtr session_ptr;
};

static void NewSession(int echo_times, int sock_ec, TcpSessionPtr session);
static void OnSessionRecv(Client* client_ptr, const char* data, uint16_t size);
static void OnSessionError(Client* client_ptr, int sock_ec);

int main(int argc, char* argv[]) {
  struct sigaction act;
  act.sa_handler = sig_int;
  sigfillset(&act.sa_mask);
  act.sa_flags = 0;
  sigaction(SIGINT, &act, NULL);


  if (argc != 5) {
    PrintHelp(argv[0]);
    return 1; 
  }

  uint16_t start_port = (uint16_t)atoi(argv[1]);
  uint16_t port_num = (uint16_t)atoi(argv[2]);
  int session_num_per_connector = atoi(argv[3]);
  int echo_times = atoi(argv[4]);

  DataInit();
  NetBase netbase;
  netbase.Run(5);

  std::vector<ConnectorPtr> connectors;
  for (uint16_t i = 0; i < port_num; i++) {
    ConnectorPtr connector_ptr = netbase.NewConnector(); 
    uint16_t port = start_port + i;
    for (int j = 0; j < session_num_per_connector; j++) {
      connector_ptr->Connect("127.0.0.1", port, 
                             std::bind(NewSession, echo_times, _1, _2));
    }
    connectors.push_back(std::move(connector_ptr));
  }

  while(0 == kFinish)
    sleep(1);

  puts("SHUTDOWN");
  netbase.Shutdown();
  delete [] data_buf;
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

  data_buf = new char[size];
  fread(data_buf, 1, size, fp);
  boost::crc_32_type crc32;
  crc32.process_bytes(data_buf, size);
  data_crc = crc32.checksum();
}

static bool DataVerify(const char* buffer, size_t sz) {
  boost::crc_32_type crc32;
  crc32.process_bytes(buffer, sz);
  return data_crc == crc32.checksum();
}

static void PrintHelp(const char* name) {
  printf("%s start_port port_num session_num_per_connector echo_times\n", name);
}

static void NewSession(int echo_times, int sock_ec, TcpSessionPtr session) {
  if (0 != sock_ec) {
    LOG("[%d>%s\n", gettid(), evutil_socket_error_to_string(sock_ec));
    return;
  }

  Client* cli = new Client;
  cli->echo_times = echo_times;
  cli->session_ptr = session;

  cli->session_ptr->SetCB(std::bind(OnSessionRecv, cli, _1, _2),
                          std::bind(OnSessionError, cli, _1));

  for (int i = echo_times; i > 0; --i) 
    cli->session_ptr->Send(data_buf, data_sz); 
}

static void OnSessionRecv(Client* client_ptr, const char* data, uint16_t size) {
  if (size < data_sz)
    return;

  assert(DataVerify(data, size));

  if (--(client_ptr->echo_times) <= 0) {
    char buf[32];
    client_ptr->session_ptr->PeerInfo(buf, 32);
    LOG("[%d>%s DISCONNECT\n", gettid(), buf);
    delete client_ptr;
  }
}

static void OnSessionError(Client* client_ptr, int sock_ec) {
  char buf[32];
  client_ptr->session_ptr->PeerInfo(buf, 32);
  LOG("[%d>%s ERROR: %s\n", gettid(), buf, evutil_socket_error_to_string(sock_ec));
  delete client_ptr;
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

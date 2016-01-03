
// sys
#include <unistd.h>
#include <signal.h>
#include <functional>
#include <map>
#include <stdio.h>
#include <string.h>
// 3rd-party
#include <event2/buffer.h>
// mine
#include "netbase.h"
#include "listener.h"
#include "connector.h"
#include "tcp_session.h"

using namespace std::placeholders;
using namespace net_base;

static volatile sig_atomic_t kFinish = 0;
static void sig_int(int);

class EchoServer {
 public:
  EchoServer(NetBase& netbase) 
      : listener_(netbase.NewListener()) 
      , counter_(0) {
    listener_->Listen("127.0.0.1", 65000, 
                      std::bind(&EchoServer::NewSession, this, _1));
  }
 private:
  void NewSession(TcpSessionPtr session_ptr) {
    // save the session 
    size_t id = counter_++;
    sessions_[id] = session_ptr;
    session_ptr->SetCB(std::bind(&EchoServer::OnSessionRecv, this, id, _1, _2),
                       std::bind(&EchoServer::OnSessionError, this, id, _1));
  }

  void OnSessionRecv(size_t session_id, const char* data, uint16_t size) {
    // echo the data
    printf("%lu DATA: ", session_id);
    fwrite(data, 1, size, stdout);
    putc('\n', stdout);

    auto it = sessions_.find(session_id);
    if (it != sessions_.end()) {
      (*it).second->Send(data, size); 
    }
  }

  void OnSessionError(size_t session_id, int sock_ec) {
    printf("%lu ERROR: %s\n", session_id, evutil_socket_error_to_string(sock_ec));
    sessions_.erase(session_id);
  }

  ListenerPtr listener_;
  size_t counter_;
  std::map<size_t, TcpSessionPtr> sessions_;
};

class Client {
 public:
  void Init(int sock_ec, TcpSessionPtr session) {
    if (0 != sock_ec) {
      puts(evutil_socket_error_to_string(sock_ec));
      return;
    }

    session_ = session;
    session_->SetCB(std::bind(&Client::OnSessionRecv, this, _1, _2),
                    std::bind(&Client::OnSessionError, this, _1));

    // send first message, use evbuffer 
    char data[] = "hello, world";
    evbuffer* evbuf = evbuffer_new();
    evbuffer_add(evbuf, data, strlen(data));
    session_->Send(evbuf);
    evbuffer_free(evbuf);
  }
 private:
  void OnSessionRecv(const char* data, uint16_t size) {
    printf("client DATA: ");
    fwrite(data, 1, size, stdout);
    putc('\n', stdout);
    session_.reset();
  }

  void OnSessionError(int sock_ec) {
    printf("client ERROR: %s\n", evutil_socket_error_to_string(sock_ec));
  }

  TcpSessionPtr session_;
};

int main(int argc, char* argv[]) {
  struct sigaction act;
  act.sa_handler = sig_int;
  sigfillset(&act.sa_mask);
  act.sa_flags = 0;
  sigaction(SIGINT, &act, NULL);

  NetBase netbase;
  netbase.Run(5);

  // echo svr
  EchoServer echo_svr(netbase);

  // client
  ConnectorPtr conn_ptr(netbase.NewConnector());
  Client client;
  conn_ptr->Connect("127.0.0.1", 65000, std::bind(&Client::Init, &client, _1, _2));

  while(0 == kFinish)
    sleep(1);

  puts("SHUTDOWN");
  netbase.Shutdown();
  return 0;
}

static void sig_int(int) {
  kFinish = 1;
}


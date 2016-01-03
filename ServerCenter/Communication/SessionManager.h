#ifndef SessionManager_H
#define SessionManager_H

#include "Type.h"
#include "Hashtable.h"
#include "SessionInfo.h"

class SessionManager
{
public:
  typedef HashtableT<int, SessionInfo> Sessions;
public:
  void Init(void);
  void AddSession(TcpSession* session);
  void Tick(void);

public:
  bool OnSessionReceive(TcpSession* session, const char* data, int len);
  bool OnSessionError(TcpSession* session, int err);
  
private:
  void HandleMyName(TcpSession* session, const char* data, int len);
  void HandleTransmit(TcpSession* session, const char* data, int len);
  void HandleTransmitResult(TcpSession* session, const char* data, int len);
  void HandleCommand(TcpSession* session, const char* data, int len);

private:
  void BroadcastAddNameHandle(const char* name, int handle);
  void BroadcastRemoveNameHandle(const char* name, int handle);
  void BroadcastNameHandleList(void);
  void SendNameHandleList(int handle);
public:
  SessionManager(void) :m_LastSyncTime(0){}
  ~SessionManager(void);
private:
  Sessions m_Sessions;
  unsigned int m_LastSyncTime;

  static const int MAX_SESSION_NUM = 1024;
};

#endif //SessionManager_H
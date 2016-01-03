#ifndef SessionManager_H
#define SessionManager_H

#include "Type.h"
#include "Hashtable.h"
#include "SessionInfo.h"

struct NameHandleInfo
{
  static const int MAX_SERVER_NAME_LENGTH = 255;

  int m_Handle;
  char m_Name[MAX_SERVER_NAME_LENGTH + 1];

  NameHandleInfo() :m_Handle(0)
  {
    m_Name[0] = 0;
  }
  bool IsValid(void) const { return m_Handle != 0; }
};

class SessionManager
{
public:
  typedef StringKeyT<NameHandleInfo::MAX_SERVER_NAME_LENGTH + 1> StringKey;
  typedef HashtableT<StringKey, NameHandleInfo, StringKey> NameHandles;
public:
  void Init(void);
  void UpdateMyName(void);
  void SetSession(TcpSession* session);
  TcpSession* GetSession(void) const { return m_Session; }
  unsigned int SessionSequence() const { return m_SessionSequence; }
  void IncSessionSequence(void){ ++m_SessionSequence; }
  int MyHandle(void) const { return m_MyselfInfo.m_Handle; }
  int TargetHandle(const char* name) const;
  const char* TargetName(int handle) const;
  void Tick(void);

public:
  bool OnSessionReceive(TcpSession* session, const char* data, int len);
  bool OnSessionError(TcpSession* session, int err);

public:
  SessionManager(void) :m_Session(NULL), m_SessionSequence(1){}
  ~SessionManager(void);
private:
  void HandleMyHandle(TcpSession* session, const char* data, int len);
  void HandleAddNameHandle(TcpSession* session, const char* data, int len);
  void HandleRemoveNameHandle(TcpSession* session, const char* data, int len);
  void HandleClearNameHandles(TcpSession* session, const char* data, int len);
  void HandleTransmit(TcpSession* session, const char* data, int len);
  void HandleTransmitResult(TcpSession* session, const char* data, int len);
  void HandleCommand(TcpSession* session, const char* data, int len);

private:
  TcpSession* m_Session;
  NameHandleInfo m_MyselfInfo;
  NameHandles m_NameHandles;
  unsigned int m_SessionSequence;

  mutable MyLock m_Lock;

  static const int MAX_SESSION_NUM = 1024;
};

#endif //SessionManager_H
#include "SessionManager.h"
#include "tcp_session.h"
#include "Delegation.h"
#include "Thread.h"
#include "MaintenanceScript.h"
#include "ScriptThread.h"

extern void HandleNameHandleChanged(bool addOrUpdate, const char* name, int handle);
extern void HandleMessage(unsigned int seq, int src, int dest, const char* msg, int len);
extern void HandleMessageResult(unsigned int seq, int src, int dest, int result);
extern void HandleCommand(int src, int dest, const char* msg);

SessionManager::~SessionManager(void)
{
  if (NULL != m_Session) {
    delete m_Session;
    m_Session = NULL;
  }
}

void SessionManager::Init(void)
{
  m_NameHandles.InitTable(MAX_SESSION_NUM);
  m_SessionSequence = 1;
}

void SessionManager::UpdateMyName(void)
{
  const char* pName = GetConfig("name");
  if (NULL != pName) {
    tsnprintf(m_MyselfInfo.m_Name, sizeof(m_MyselfInfo.m_Name), "%s", pName);
  }
}

void SessionManager::SetSession(TcpSession* session)
{
  if (NULL != m_Session) {
    ::HandleNameHandleChanged(false, m_MyselfInfo.m_Name, m_MyselfInfo.m_Handle);

    delete m_Session;
    m_Session = NULL;
  }
  m_MyselfInfo.m_Handle = 0;
  m_SessionSequence = 1;

  if (NULL != session) {
    m_Session = session;
    session->SetCB(ReceiveCallback(this, &SessionManager::OnSessionReceive), ErrorCallback(this, &SessionManager::OnSessionError));

    size_t len = strlen(m_MyselfInfo.m_Name);
    int msgSize = sizeof(MessageMyName) + len;
    MessageMyName* msg = reinterpret_cast<MessageMyName*>(new char[msgSize]);
    msg->SetClass(MSG_SC_MYNAME);
    memcpy(msg->m_Name, m_MyselfInfo.m_Name, len + 1);
    session->Send(reinterpret_cast<const char*>(msg), msgSize);

    __Internal_Log("send MyName %s", m_MyselfInfo.m_Name);
    delete[](char*)msg;
  }
}

int SessionManager::TargetHandle(const char* name) const
{
  if (NULL == name)
    return 0;
  AutoLock_T lock(m_Lock);
  int ret = 0;
  const NameHandleInfo& info = m_NameHandles.Get(name);
  if (info.IsValid()) {
    ret = info.m_Handle;
  }
  return ret;
}

const char* SessionManager::TargetName(int handle) const
{
  AutoLock_T lock(m_Lock);
  const char* ret = NULL;
  for (NameHandles::Iterator it = m_NameHandles.First(); !it.IsNull(); ++it) {
    const NameHandleInfo& info = it->GetValue();
    if (info.IsValid() && info.m_Handle == handle) {
      ret = info.m_Name;
    }
  }
  return ret;
}

void SessionManager::Tick(void)
{
  unsigned int curTime = MyTimeGetTime();
  //检查会话是否正常
  if (NULL != m_Session) {
    if (!m_Session->IsValid()) {
      __Internal_Log("session removed for invalid session");
      SetSession(NULL);
    }
  }
}

bool SessionManager::OnSessionReceive(TcpSession* session, const char* data, int len)
{
  if (NULL == session || NULL == data)
    return false;
  if (len > 1) {
    char msgClass = data[0];
    switch (msgClass) {
    case (char)MSG_CS_MYHANDLE:
      HandleMyHandle(session, data, len);
      break;
    case (char)MSG_CS_ADD_NAME_HANDLE:
      HandleAddNameHandle(session, data, len);
      break;
    case (char)MSG_CS_REMOVE_NAME_HANDLE:
      HandleRemoveNameHandle(session, data, len);
      break;
    case (char)MSG_CS_CLEAR_NAME_HANDLE_LIST:
      HandleRemoveNameHandle(session, data, len);
      break;
    case (char)MSG_SCS_TRANSMIT:
      HandleTransmit(session, data, len);
      break;
    case (char)MSG_SCS_TRANSMIT_RESULT:
      HandleTransmitResult(session, data, len);
      break;
    case (char)MSG_SCS_COMMAND:
      HandleCommand(session, data, len);
      break;
    }
  }
  return true;
}

bool SessionManager::OnSessionError(TcpSession* session, int err)
{
  if (NULL == session)
    return false;
  int handle = session->GetFD();

  __Internal_Log("session %d removed for error %d", handle, err);
  SetSession(NULL);
  return true;
}

void SessionManager::HandleMyHandle(TcpSession* session, const char* data, int len)
{
  if (NULL == session || NULL == data)
    return;
  const MessageMyHandle* msg = reinterpret_cast<const MessageMyHandle*>(data);
  m_MyselfInfo.m_Handle = msg->m_Handle;

  {
    AutoLock_T lock(m_Lock);
    NameHandleInfo& info = m_NameHandles.Get(m_MyselfInfo.m_Name);
    if (info.IsValid()) {
      info.m_Handle = m_MyselfInfo.m_Handle;
    } else {
      m_NameHandles.Add(m_MyselfInfo.m_Name, m_MyselfInfo);
    }
  }

  ::HandleNameHandleChanged(true, m_MyselfInfo.m_Name, m_MyselfInfo.m_Handle);

  __Internal_Log("MyHandle:%d", msg->m_Handle);
}

void SessionManager::HandleAddNameHandle(TcpSession* session, const char* data, int len)
{
  if (NULL == session || NULL == data)
    return;
  const MessageAddNameHandle* msg = reinterpret_cast<const MessageAddNameHandle*>(data);

  {
    AutoLock_T lock(m_Lock);
    NameHandleInfo& info = m_NameHandles.Get(msg->m_Name);
    if (info.IsValid()) {
      info.m_Handle = msg->m_Handle;
    } else {
      NameHandleInfo nameHandle;
      nameHandle.m_Handle = msg->m_Handle;
      tsnprintf(nameHandle.m_Name, sizeof(nameHandle.m_Name), "%s", msg->m_Name);
      m_NameHandles.Add(nameHandle.m_Name, nameHandle);
    }
  }

  ::HandleNameHandleChanged(true, msg->m_Name, msg->m_Handle);

  __Internal_Log("AddNameHandle:%s->%d", msg->m_Name, msg->m_Handle);
}

void SessionManager::HandleRemoveNameHandle(TcpSession* session, const char* data, int len)
{
  if (NULL == session || NULL == data)
    return;
  const MessageRemoveNameHandle* msg = reinterpret_cast<const MessageRemoveNameHandle*>(data);

  {
    AutoLock_T lock(m_Lock);
    m_NameHandles.Remove(msg->m_Name);
  }

  ::HandleNameHandleChanged(false, msg->m_Name, msg->m_Handle);

  __Internal_Log("RemoveNameHandle:%s->%d", msg->m_Name, msg->m_Handle);
}

void SessionManager::HandleClearNameHandles(TcpSession* session, const char* data, int len)
{
  {
    AutoLock_T lock(m_Lock);
    m_NameHandles.CleanUp();
  }

  __Internal_Log("ClearNameHandles");
}

void SessionManager::HandleTransmit(TcpSession* session, const char* data, int len)
{
  if (NULL == session || NULL == data)
    return;
  const MessageTransmit* msg = reinterpret_cast<const MessageTransmit*>(data);
  unsigned int seq = msg->m_Sequence;
  int src = msg->m_Src;
  int dest = msg->m_Dest;

  MessageTransmitResult retMsg;
  retMsg.m_Dest = src;
  retMsg.m_Src = dest;
  retMsg.m_Sequence = seq;
  retMsg.m_IsSuccess = true;
  session->Send(reinterpret_cast<const char*>(&retMsg), sizeof(MessageTransmitResult));

  //通知逻辑层
  ::HandleMessage(seq, src, dest, msg->m_Data, len - sizeof(MessageTransmit) + 1);
}

void SessionManager::HandleTransmitResult(TcpSession* session, const char* data, int len)
{
  if (NULL == session || NULL == data)
    return;
  const MessageTransmitResult* msg = reinterpret_cast<const MessageTransmitResult*>(data);
  unsigned int seq = msg->m_Sequence;
  int src = msg->m_Src;
  int dest = msg->m_Dest;

  //通知逻辑层
  ::HandleMessageResult(seq, src, dest, msg->m_IsSuccess ? 1 : 0);

  if (!msg->m_IsSuccess) {
    __Internal_Log("TransmitResult:%d %d", msg->m_Sequence, msg->m_IsSuccess ? 1 : 0);
  }
}

void SessionManager::HandleCommand(TcpSession* session, const char* data, int len)
{
  if (NULL == session || NULL == data)
    return;
  const MessageCommand* msg = reinterpret_cast<const MessageCommand*>(data);
  int src = msg->m_Src;
  int dest = msg->m_Dest;
  ::HandleCommand(src, dest, msg->m_Command);

  __Internal_Log("CommandToLogic:%d->%d %s", src, dest, msg->m_Command);
}
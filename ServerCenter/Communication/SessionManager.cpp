#include "SessionManager.h"
#include "tcp_session.h"
#include "Delegation.h"
#include "Thread.h"
#include "ScriptThread.h"

SessionManager::~SessionManager(void)
{
  for (Sessions::Iterator it = m_Sessions.First(); !it.IsNull(); ++it){
    const SessionInfo& info = it->GetValue();
    if (NULL != info.m_Session){
      delete info.m_Session;
    }
  }
}

void SessionManager::Init(void)
{
  m_Sessions.InitTable(MAX_SESSION_NUM);
  m_LastSyncTime = 0;
}

void SessionManager::AddSession(TcpSession* session)
{
  if (NULL == session)
    return;
  session->SetCB(ReceiveCallback(this, &SessionManager::OnSessionReceive), ErrorCallback(this, &SessionManager::OnSessionError));
  int handle = session->GetFD();
  SessionInfo& info = m_Sessions.Get(handle);
  if (!info.IsValid()){
    SessionInfo sessionInfo;
    sessionInfo.m_Handle = handle;
    sessionInfo.m_Session = session;
    m_Sessions.Add(handle, sessionInfo);
  } else {
    info.m_Handle = handle;
    info.m_Session = session;
  }
}

void SessionManager::Tick(void)
{
  const int c_MaxCheckInvalidSessionNum = 256;

  unsigned int curTime = MyTimeGetTime();
  if (m_LastSyncTime + 60000 < curTime){
    m_LastSyncTime = curTime;

    if (m_Sessions.GetNum() > 0){
      //检查会话是否正常
      int handles[c_MaxCheckInvalidSessionNum];
      int ct = 0;
      for (Sessions::Iterator it = m_Sessions.First(); !it.IsNull(); ++it){
        const SessionInfo& info = it->GetValue();
        TcpSession* pSession = info.m_Session;
        if (NULL != pSession){
          if (!pSession->IsValid()){
            handles[ct] = info.m_Handle;
            ++ct;
            if (ct >= c_MaxCheckInvalidSessionNum){
              break;
            }
          }
        }
      }
      if (ct > 0 && ct<=c_MaxCheckInvalidSessionNum){
        for (int i = 0; i < ct; ++i){
          int handle = handles[i];
          const SessionInfo& info = m_Sessions.Get(handle);
          if (info.IsValid()){
            __Internal_Log("session %d name %s removed for invalid session", info.m_Handle, info.m_Name);

            BroadcastRemoveNameHandle(info.m_Name, info.m_Handle);

            delete info.m_Session;
          }
          m_Sessions.Remove(handle);
        }
      }
    }

    BroadcastNameHandleList();
  }

  if (NULL != g_pScriptThread){
    char result[ScriptThread::MAX_LINE_SIZE];
    if (g_pScriptThread->PopResult(result, ScriptThread::MAX_LINE_SIZE)){
      int dest = g_pScriptThread->CurSrc();
      const SessionInfo& info = m_Sessions.Get(dest);
      if (info.IsValid() && NULL != info.m_Session){
        size_t resSize = strlen(result);
        int msgSize = sizeof(MessageCommand) + resSize;
        MessageCommand* msg = reinterpret_cast<MessageCommand*>(new char[msgSize]);
        msg->SetClass(MSG_SCS_COMMAND);
        msg->m_Src = 0;
        msg->m_Dest = dest;
        memcpy(msg->m_Command, result, resSize + 1);
        if (!info.m_Session->Send(reinterpret_cast<const char*>(msg), msgSize)){
          __Internal_Log("Send command result failed, dest %d", dest);
        }
        delete[] (char*)msg;
      } else {
        __Internal_Log("Can't send command result, unknown dest %d", dest);
      }
    }
  }
}

bool SessionManager::OnSessionReceive(TcpSession* session, const char* data, int len)
{
  if (NULL == session || NULL == data)
    return false;
  if (len > 1){
    char msgClass = data[0];
    switch (msgClass){
    case (char)MSG_SC_MYNAME:
      HandleMyName(session, data, len);
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
  const SessionInfo& info = m_Sessions.Get(handle);
  if (info.IsValid()){
    __Internal_Log("session %d name %s removed for error %d", handle, info.m_Name, err);

    BroadcastRemoveNameHandle(info.m_Name, info.m_Handle);
  } else {
    __Internal_Log("session %d name (unknown) removed for error %d", handle, err);
  }
  delete session;
  m_Sessions.Remove(handle);

  return true;
}

void SessionManager::HandleMyName(TcpSession* session, const char* data, int len)
{
  if (NULL == session || NULL == data)
    return;
  int src = session->GetFD();
  SessionInfo& sessionInfo = m_Sessions.Get(src);
  const MessageMyName* msg = reinterpret_cast<const MessageMyName*>(data);
  if (sessionInfo.IsValid()){
    MessageMyHandle handleMsg;
    handleMsg.m_Handle = src;
    session->Send(reinterpret_cast<const char*>(&handleMsg), sizeof(MessageMyHandle));

    SendNameHandleList(src);

    tsnprintf(sessionInfo.m_Name, sizeof(sessionInfo.m_Name), "%s", msg->m_Name);
    BroadcastAddNameHandle(msg->m_Name, src);

    __Internal_Log("session %d receive (MessageMyName:%s)", src, msg->m_Name);
  } else {
    __Internal_Log("session %d can't find session info (MessageMyName:%s)", src, msg->m_Name);
  }
}

void SessionManager::HandleTransmit(TcpSession* session, const char* data, int len)
{
  if (NULL == session || NULL == data)
    return;
  const MessageTransmit* msg = reinterpret_cast<const MessageTransmit*>(data);
  unsigned int seq = msg->m_Sequence;
  int src = msg->m_Src;
  int dest = msg->m_Dest;
  bool send = false;
  const SessionInfo& sessionInfo = m_Sessions.Get(dest);
  if (sessionInfo.IsValid()){
    if (NULL != sessionInfo.m_Session){
      send = sessionInfo.m_Session->Send(data, len);
    }
  }
  if (!send){
    MessageTransmitResult retMsg;
    retMsg.m_Dest = src;
    retMsg.m_Src = 0;
    retMsg.m_Sequence = seq;
    retMsg.m_IsSuccess = send;
    session->Send(reinterpret_cast<const char*>(&retMsg), sizeof(MessageTransmitResult));
  }
}

void SessionManager::HandleTransmitResult(TcpSession* session, const char* data, int len)
{
  if (NULL == session || NULL == data)
    return;
  const MessageTransmitResult* msg = reinterpret_cast<const MessageTransmitResult*>(data);
  unsigned int seq = msg->m_Sequence;
  int src = msg->m_Src;
  int dest = msg->m_Dest;
  const SessionInfo& sessionInfo = m_Sessions.Get(dest);
  if (sessionInfo.IsValid()){
    if (NULL != sessionInfo.m_Session){
      sessionInfo.m_Session->Send(data, len);
    }
  }
}

void SessionManager::HandleCommand(TcpSession* session, const char* data, int len)
{
  if (NULL == session || NULL == data)
    return;
  const MessageCommand* msg = reinterpret_cast<const MessageCommand*>(data);
  int src = msg->m_Src;
  int dest = msg->m_Dest;
  if (dest == 0){
    if (NULL != g_pScriptThread){
      const char* cmd = msg->m_Command;
      g_pScriptThread->CurSrc(src);
      g_pScriptThread->PushLine(cmd);

      __Internal_Log("Execute Command:%s", cmd);
    }
  } else {
    const SessionInfo& sessionInfo = m_Sessions.Get(dest);
    if (sessionInfo.IsValid()){
      if (NULL != sessionInfo.m_Session){
        sessionInfo.m_Session->Send(data, len);
      }
    }
  }
}

void SessionManager::BroadcastAddNameHandle(const char* name, int handle)
{
  if (NULL == name)
    return;
  size_t len = strlen(name);
  int msgSize = sizeof(MessageAddNameHandle) + len;
  MessageAddNameHandle* msg = reinterpret_cast<MessageAddNameHandle*>(new char[msgSize]);
  msg->SetClass(MSG_CS_ADD_NAME_HANDLE);
  msg->m_Handle = handle;
  memcpy(msg->m_Name, name, len + 1);
  for (Sessions::Iterator it = m_Sessions.First(); !it.IsNull(); ++it){
    const SessionInfo& info = it->GetValue();
    if (info.m_Handle != handle && NULL != info.m_Session){
      info.m_Session->Send(reinterpret_cast<const char*>(msg), msgSize);
    }
  }
  delete[] (char*)msg;
}

void SessionManager::BroadcastRemoveNameHandle(const char* name, int handle)
{
  if (NULL == name)
    return;
  size_t len = strlen(name);
  int msgSize = sizeof(MessageRemoveNameHandle) + len;
  MessageRemoveNameHandle* msg = reinterpret_cast<MessageRemoveNameHandle*>(new char[msgSize]);
  msg->SetClass(MSG_CS_REMOVE_NAME_HANDLE);
  msg->m_Handle = handle;
  memcpy(msg->m_Name, name, len + 1);
  for (Sessions::Iterator it = m_Sessions.First(); !it.IsNull(); ++it){
    const SessionInfo& info = it->GetValue();
    if (info.m_Handle != handle && NULL != info.m_Session){
      info.m_Session->Send(reinterpret_cast<const char*>(msg), msgSize);
    }
  }
  delete[] (char*)msg;
}

void SessionManager::BroadcastNameHandleList(void)
{
  if (m_Sessions.GetNum() <= 0)
    return;
  MessageClearNameHandleList clearMsg;  
  MessageAddNameHandle** addMsgs = new MessageAddNameHandle*[m_Sessions.GetNum()];
  int ct = 0;
  for (Sessions::Iterator it = m_Sessions.First(); !it.IsNull(); ++it){
    const SessionInfo& info = it->GetValue();
    size_t len = strlen(info.m_Name);
    if (len > 0){
      int msgSize = sizeof(MessageAddNameHandle) + len;
      MessageAddNameHandle* msg = reinterpret_cast<MessageAddNameHandle*>(new char[msgSize]);
      msg->SetClass(MSG_CS_ADD_NAME_HANDLE);
      msg->m_Handle = info.m_Handle;
      memcpy(msg->m_Name, info.m_Name, len + 1);

      addMsgs[ct] = msg;
      ++ct;
    }
  }

  for (Sessions::Iterator it = m_Sessions.First(); !it.IsNull(); ++it){
    const SessionInfo& info = it->GetValue();
    if (NULL != info.m_Session){
      info.m_Session->Send(reinterpret_cast<const char*>(&clearMsg), sizeof(MessageClearNameHandleList));
      for (int i = 0; i < ct; ++i){
        MessageAddNameHandle* msg = addMsgs[i];
        int msgSize = sizeof(MessageAddNameHandle) + strlen(msg->m_Name);
        info.m_Session->Send(reinterpret_cast<const char*>(msg), msgSize);
      }
    }
  }

  for (int i = 0; i < ct; ++i){
    delete[] (char*)addMsgs[i];
  }
  delete[] addMsgs;
}

void SessionManager::SendNameHandleList(int handle)
{
  if (m_Sessions.GetNum() <= 0)
    return;
  const SessionInfo& receiver = m_Sessions.Get(handle);
  if (receiver.IsValid()){
    for (Sessions::Iterator it = m_Sessions.First(); !it.IsNull(); ++it){
      const SessionInfo& info = it->GetValue();
      size_t len = strlen(info.m_Name);
      if (len > 0){
        int msgSize = sizeof(MessageAddNameHandle) + len;
        MessageAddNameHandle* msg = reinterpret_cast<MessageAddNameHandle*>(new char[msgSize]);
        msg->SetClass(MSG_CS_ADD_NAME_HANDLE);
        msg->m_Handle = info.m_Handle;
        memcpy(msg->m_Name, info.m_Name, len + 1);
        
        receiver.m_Session->Send(reinterpret_cast<const char*>(msg), msgSize);
        delete[] (char*)msg;
      }
    }
  }
}
#ifndef __ScriptThread_H__
#define __ScriptThread_H__

#include "Type.h"
#include "Queue.h"
#include "ConfigScript/CommonScriptApi.h"
#include <event2/event.h>
#include <event2/thread.h>
#include "connector.h"
#include "netioservice.h"
#include "tcp_session.h"
#include "Stream.h"
#include "SessionInfo.h"

class SharedStringMap;
extern SharedStringMap* g_pMap;
extern int g_Argc;
extern char** g_Argv;

class SharedMessageQueue
{
  enum
  {
    MAX_CONTENT_LENGTH = 512,
  };
public:
  struct MessageType
  {
    char m_Content[MAX_CONTENT_LENGTH + 1];

    MessageType(void)
    {
      Reset();
    }
    void Reset(void)
    {
      memset(m_Content, 0, sizeof(m_Content));
    }
  };
  typedef DequeT<MessageType> MessageQueueType;
public:
  void InitQueue(int num)
  {
    AutoLock_T lock(m_Lock);
    m_MessageQueue.Clear();
    m_MessageQueue.Init(num);
  }
  void Push(const MessageType& msg)
  {
    AutoLock_T lock(m_Lock);
    if (m_MessageQueue.Full())
    {
      m_MessageQueue.PopFront();
    }
    m_MessageQueue.PushBack(msg);
  }
  MessageType Pop(void)
  {
    AutoLock_T lock(m_Lock);
    MessageType ret;
    if (FALSE == m_MessageQueue.Empty())
    {
      ret = m_MessageQueue.Front();
      m_MessageQueue.PopFront();
    }
    return ret;
  }
  void ClearQueue(void)
  {
    AutoLock_T lock(m_Lock);
    m_MessageQueue.Clear();
  }
public:
  SharedMessageQueue(void)
  {
    InitQueue(1024);
  }
  virtual ~SharedMessageQueue(void)
  {
    ClearQueue();
  }
private:
  MessageQueueType	m_MessageQueue;
  mutable MyLock		m_Lock;
};

class SimpleClient;
int CreateStreamPacketObj(FunctionScript::Value& retValue, Interpreter& interpreter, SimpleClient& client, BinaryStream& stream);

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

class SimpleClient
{
  static const int MAX_IP_LEN = 128;
  static const int MAX_SESSION_NUM = 1024;
public:
  typedef StringKeyT<NameHandleInfo::MAX_SERVER_NAME_LENGTH + 1> StringKey;
  typedef HashtableT<StringKey, NameHandleInfo, StringKey> NameHandles;
public:
  void SetCallbackObj(ExpressionApi* pExpression)
  {
    m_pCallbackObj = pExpression;
  }
  void SetConnectInfo(const char* name, const char* ip, int port)
  {
    tsnprintf(m_MyselfInfo.m_Name, NameHandleInfo::MAX_SERVER_NAME_LENGTH, "%s", name);
    tsnprintf(m_Ip, sizeof(m_Ip), "%s", ip);
    m_Port = port;
  }
  void Tick(void)
  {
    if (NULL != m_pNetIoService && NULL != m_pEventBase && NULL != m_pConnector){
      if (NULL == m_pSession && TRUE == m_ConnectFinish){
        m_ConnectFinish = FALSE;
        try{
          m_pConnector->Connect(m_Ip, m_Port, ConnectFinishCallback(this, &SimpleClient::OnConnectFinish));
        } catch (std::exception&){
          __Internal_Log("Connect failed !!!");
        } catch (...){
          m_ConnectFinish = TRUE;
        }
      }

      m_pNetIoService->Poll();

      //检查会话是否正常
      if (NULL != m_pSession){
        if (!m_pSession->IsValid()) {
          __Internal_Log("session removed for invalid session");
          delete m_pSession;
          m_pSession = NULL;
          m_MyselfInfo.m_Handle = 0;
          m_SessionSequence = 1;
        }
      }
    }
  }
  int IsConnected(void) const
  {
    return NULL != m_pSession ? 1 : 0;
  }
  void Disconnect(void)
  {
    if (NULL != m_pSession){
      delete m_pSession;
      m_pSession = NULL;
    }
    m_MyselfInfo.m_Handle = 0;
    m_SessionSequence = 1;
  }
public:
  int MyHandle(void) const { return m_MyselfInfo.m_Handle; }
  int TargetHandle(const char* name) const
  {
    int ret = 0;
    const NameHandleInfo& info = m_NameHandles.Get(name);
    if (info.IsValid()){
      ret = info.m_Handle;
    }
    return ret;
  }
  const char* TargetName(int handle) const
  {
    const char* ret = NULL;
    for (NameHandles::Iterator it = m_NameHandles.First(); !it.IsNull(); ++it){
      const NameHandleInfo& info = it->GetValue();
      if (info.IsValid() && info.m_Handle == handle){
        ret = info.m_Name;
      }
    }
    return ret;
  }
  int SendByHandle(int dest, const void* data, int len)
  {
    int ret = 0;
    if (NULL != data){
      if (NULL != m_pSession){
        MessageTransmit msg;
        msg.m_Sequence = SessionSequence();
        msg.m_Src = MyHandle();
        msg.m_Dest = dest;
        if (msg.IsValid() && m_pSession->Send(reinterpret_cast<const char*>(&msg), sizeof(msg) - 1, reinterpret_cast<const char*>(data), len)){
          IncSessionSequence();
          ret = 1;
        }
      }
    }
    return ret;
  }
  int SendByName(const char* name, const void* data, int len)
  {
    int ret = 0;
    if (NULL != name && NULL != data){
      int dest = TargetHandle(name);
      ret = SendByHandle(dest, data, len);
    }
    return ret;
  }
  int SendCommandByHandle(int dest, const char* result)
  {
    int ret = 0;
    if (NULL != result){
      if (NULL != m_pSession){
        size_t cmdSize = strlen(result);
        MessageCommand msg;
        msg.m_Src = MyHandle();
        msg.m_Dest = dest;
        if (m_pSession->Send(reinterpret_cast<const char*>(&msg), sizeof(msg) - 1, result, cmdSize + 1)){
          ret = 1;
        }
      }
    }
    return ret;
  }
  int SendCommandByName(const char* name, const char* result)
  {
    int ret = 0;
    if (NULL != name && NULL != result){
      int dest = TargetHandle(name);
      ret = SendCommandByHandle(dest, result);
    }
    return ret;
  }
private:
  unsigned int SessionSequence() const { return m_SessionSequence; }
  void IncSessionSequence(void){ ++m_SessionSequence; }
  bool OnConnectFinish(TcpSession* session, int err)
  {
    if (NULL != m_pSession){
      HandleNameHandleChanged(false, m_MyselfInfo.m_Name, m_MyselfInfo.m_Handle);

      delete m_pSession;
      m_pSession = NULL;
    }
    m_MyselfInfo.m_Handle = 0;
    m_SessionSequence = 1;

    if (NULL != session){
      m_pSession = session;
      session->SetCB(ReceiveCallback(this, &SimpleClient::OnSessionReceive), ErrorCallback(this, &SimpleClient::OnSessionError));

      size_t len = strlen(m_MyselfInfo.m_Name);
      int msgSize = sizeof(MessageMyName) + len;
      MessageMyName* msg = reinterpret_cast<MessageMyName*>(new char[msgSize]);
      msg->SetClass(MSG_SC_MYNAME);
      memcpy(msg->m_Name, m_MyselfInfo.m_Name, len + 1);
      session->Send(reinterpret_cast<const char*>(msg), msgSize);

      __Internal_Log("send MyName %s", m_MyselfInfo.m_Name);
      delete[](char*)msg;
    }
    m_ConnectFinish = TRUE;
    return true;
  }
  bool OnSessionReceive(TcpSession* session, const char* data, int len)
  {
    if (len > 1){
      char msgClass = data[0];
      switch (msgClass){
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
  bool OnSessionError(TcpSession* session, int err)
  {
    int handle = session->GetFD();

    __Internal_Log("session %d removed for error %d", handle, err);
    if (NULL != m_pSession){
      delete m_pSession;
      m_pSession = NULL;
    }
    m_MyselfInfo.m_Handle = 0;
    m_SessionSequence = 1;
    return true;
  }
private:
  void HandleMyHandle(TcpSession* session, const char* data, int len)
  {
    const MessageMyHandle* msg = reinterpret_cast<const MessageMyHandle*>(data);
    m_MyselfInfo.m_Handle = msg->m_Handle;

    {
      NameHandleInfo& info = m_NameHandles.Get(m_MyselfInfo.m_Name);
      if (info.IsValid()){
        info.m_Handle = m_MyselfInfo.m_Handle;
      } else {
        m_NameHandles.Add(m_MyselfInfo.m_Name, m_MyselfInfo);
      }
    }

    HandleNameHandleChanged(true, m_MyselfInfo.m_Name, m_MyselfInfo.m_Handle);

    __Internal_Log("MyHandle:%d", msg->m_Handle);
  }

  void HandleAddNameHandle(TcpSession* session, const char* data, int len)
  {
    const MessageAddNameHandle* msg = reinterpret_cast<const MessageAddNameHandle*>(data);

    {
      NameHandleInfo& info = m_NameHandles.Get(msg->m_Name);
      if (info.IsValid()){
        info.m_Handle = msg->m_Handle;
      } else {
        NameHandleInfo nameHandle;
        nameHandle.m_Handle = msg->m_Handle;
        tsnprintf(nameHandle.m_Name, sizeof(nameHandle.m_Name), "%s", msg->m_Name);
        m_NameHandles.Add(nameHandle.m_Name, nameHandle);
      }
    }

    HandleNameHandleChanged(true, msg->m_Name, msg->m_Handle);

    __Internal_Log("AddNameHandle:%s->%d", msg->m_Name, msg->m_Handle);
  }

  void HandleRemoveNameHandle(TcpSession* session, const char* data, int len)
  {
    const MessageRemoveNameHandle* msg = reinterpret_cast<const MessageRemoveNameHandle*>(data);

    {
      m_NameHandles.Remove(msg->m_Name);
    }

    HandleNameHandleChanged(false, msg->m_Name, msg->m_Handle);

    __Internal_Log("RemoveNameHandle:%s->%d", msg->m_Name, msg->m_Handle);
  }

  void HandleClearNameHandles(TcpSession* session, const char* data, int len)
  {
    {
      m_NameHandles.CleanUp();
    }

    __Internal_Log("ClearNameHandles");
  }

  void HandleTransmit(TcpSession* session, const char* data, int len)
  {
    const MessageTransmit* msg = reinterpret_cast<const MessageTransmit*>(data);
    unsigned int seq = msg->m_Sequence;
    int src = msg->m_Src;
    int dest = msg->m_Dest;

    //通知逻辑层
    HandleMessage(seq, src, dest, msg->m_Data, len - sizeof(MessageTransmit) + 1);
  }

  void HandleTransmitResult(TcpSession* session, const char* data, int len)
  {
    const MessageTransmitResult* msg = reinterpret_cast<const MessageTransmitResult*>(data);

    __Internal_Log("TransmitResult:%d %d", msg->m_Sequence, msg->m_IsSuccess ? 1 : 0);
  }

  void HandleCommand(TcpSession* session, const char* data, int len)
  {
    const MessageCommand* msg = reinterpret_cast<const MessageCommand*>(data);
    int src = msg->m_Src;
    int dest = msg->m_Dest;

    HandleCommand(src, dest, msg->m_Command);

    __Internal_Log("CommandToLogic:%d->%d %s", src, dest, msg->m_Command);
  }
private:
  void HandleNameHandleChanged(bool addOrUpdate, const char* name, int handle)
  {
    if (NULL != m_pInterpreter && NULL != m_pCallbackObj){
      Value nameVal;
      nameVal.InitString(name);
      Value params[] = { Value(addOrUpdate ? 1 : 0), nameVal, Value(handle) };
      Value retVal;
      m_pInterpreter->CallMember(*m_pCallbackObj, Value("onNameHandleChanged"), FALSE, Function::PARAM_CLASS_PARENTHESIS, params, 3, &retVal);
    }
  }
  void HandleMessage(unsigned int seq, int src, int dest, const char* data, int len)
  {
    if (NULL != m_pInterpreter && NULL != m_pCallbackObj){
      BinaryStream* pStream = new BinaryStream(len);
      pStream->SetStreamData(data, len);
      Value packet;
      CreateStreamPacketObj(packet, *m_pInterpreter, *this, *pStream);
      Value params[] = { Value((int)seq), Value(src), Value(dest), packet };
      Value retVal;
      m_pInterpreter->CallMember(*m_pCallbackObj, Value("onMessage"), FALSE, Function::PARAM_CLASS_PARENTHESIS, params, 4, &retVal);
    }
  }
  void HandleMessageResult(unsigned int seq, int src, int dest, bool result)
  {
    if (NULL != m_pInterpreter && NULL != m_pCallbackObj){
      Value params[] = { Value((int)seq), Value(src), Value(dest), Value(result ? 1 : 0) };
      Value retVal;
      m_pInterpreter->CallMember(*m_pCallbackObj, Value("onMessageResult"), FALSE, Function::PARAM_CLASS_PARENTHESIS, params, 4, &retVal);
    }
  }
  void HandleCommand(int src, int dest, const char* result)
  {
    if (NULL != m_pInterpreter && NULL != m_pCallbackObj){
      Value params[] = { Value(src), Value(dest), Value(result) };
      Value retVal;
      m_pInterpreter->CallMember(*m_pCallbackObj, Value("onCommand"), FALSE, Function::PARAM_CLASS_PARENTHESIS, params, 3, &retVal);
    }
  }
private:
  void Init(void)
  {
    if (m_NameHandles.IsInited()){
      m_NameHandles.CleanUp();
    }
    m_NameHandles.InitTable(MAX_SESSION_NUM);
    m_SessionSequence = 1;

    m_pNetIoService = new NetIoService();
    if (NULL != m_pNetIoService){
      m_pNetIoService->Init();
      m_pEventBase = m_pNetIoService->EventBase();
      if (NULL != m_pEventBase){
        m_pConnector = new Connector(m_pEventBase);
      }
    }
  }
  void Release(void)
  {
    if (NULL != m_pSession){
      delete m_pSession;
      m_pSession = NULL;
    }
    if (NULL != m_pConnector){
      delete m_pConnector;
      m_pConnector = NULL;
    }
    if (NULL != m_pNetIoService){
      m_pNetIoService->Finalize();
      delete m_pNetIoService;
      m_pNetIoService = NULL;
    }
  }
public:
  SimpleClient(Interpreter& interpreter) :m_pInterpreter(&interpreter), m_pCallbackObj(NULL), m_ConnectFinish(TRUE),
    m_pSession(NULL), m_pEventBase(NULL), m_pConnector(NULL), m_pNetIoService(NULL)
  {
    Init();
  }
  ~SimpleClient(void)
  {
    Release();
  }
private:
  char m_Ip[MAX_IP_LEN + 1];
  int m_Port;

  Interpreter* m_pInterpreter;
  ExpressionApi* m_pCallbackObj;

  NameHandleInfo m_MyselfInfo;
  NameHandles m_NameHandles;
  unsigned int m_SessionSequence;
  int m_ConnectFinish;

  TcpSession* m_pSession;
  event_base* m_pEventBase;
  NetIoService* m_pNetIoService;
  Connector* m_pConnector;
};

class SimpleClientObj : public ObjectBase
{
  enum
  {
    CUSTOM_MEMBER_INDEX_SETCALLBACK = 0,
    CUSTOM_MEMBER_INDEX_SETCONNECTINFO,
    CUSTOM_MEMBER_INDEX_TICK,
    CUSTOM_MEMBER_INDEX_IS_CONNECTED,
    CUSTOM_MEMBER_INDEX_DISCONNECT,
    CUSTOM_MEMBER_INDEX_MYHANDLE,
    CUSTOM_MEMBER_INDEX_TARGETHANLE,
    CUSTOM_MEMBER_INDEX_TARGETNAME,
    CUSTOM_MEMBER_INDEX_CREATEPACKET,
    CUSTOM_MEMBER_INDEX_SENDCOMMANDTOCOREBYHANDLE,
    CUSTOM_MEMBER_INDEX_SENDCOMMANDTOCOREBYANME,
    CUSTOM_MEMBER_INDEX_SENDCOMMANDBYHANDLE,
    CUSTOM_MEMBER_INDEX_SENDCOMMANDBYANME,
    CUSTOM_MEMBER_INDEX_NUM,
  };
public:
  SimpleClientObj(Interpreter& interpreter) :ObjectBase(interpreter, CUSTOM_MEMBER_INDEX_NUM), m_Client(interpreter)
  {}
  virtual ~SimpleClientObj(void)
  {}
protected:
  virtual int	 GetCustomInnerMemberIndex(const char* name)const
  {
    if (NULL == name)
      return -1;
    else if (strcmp(name, "setCallback") == 0)
      return CUSTOM_MEMBER_INDEX_SETCALLBACK;
    else if (strcmp(name, "setConnectInfo") == 0)
      return CUSTOM_MEMBER_INDEX_SETCONNECTINFO;
    else if (strcmp(name, "tick") == 0)
      return CUSTOM_MEMBER_INDEX_TICK;
    else if (strcmp(name, "isConnected") == 0)
      return CUSTOM_MEMBER_INDEX_IS_CONNECTED;
    else if (strcmp(name, "disconnect") == 0)
      return CUSTOM_MEMBER_INDEX_DISCONNECT;
    else if (strcmp(name, "myHanle") == 0)
      return CUSTOM_MEMBER_INDEX_MYHANDLE;
    else if (strcmp(name, "targetHandle") == 0)
      return CUSTOM_MEMBER_INDEX_TARGETHANLE;
    else if (strcmp(name, "targetName") == 0)
      return CUSTOM_MEMBER_INDEX_TARGETNAME;
    else if (strcmp(name, "createPacket") == 0)
      return CUSTOM_MEMBER_INDEX_CREATEPACKET;
    else if (strcmp(name, "sendCommandByHandle") == 0)
      return CUSTOM_MEMBER_INDEX_SENDCOMMANDBYHANDLE;
    else if (strcmp(name, "sendCommandByName") == 0)
      return CUSTOM_MEMBER_INDEX_SENDCOMMANDBYANME;
    else
      return -1;
  }
  virtual ExecuteResultEnum ExecuteCustomMember(int index, int paramClass, Value* pParams, int num, Value* pRetValue)
  {
    if (NULL != pParams)
    {
      switch (index)
      {
      case CUSTOM_MEMBER_INDEX_SETCALLBACK:
        if (1 == num && pParams[0].IsExpression() && NULL != pRetValue){
          ExpressionApi* pExpression = pParams[0].GetExpression();
          m_Client.SetCallbackObj(pExpression);
        }
        break;
      case CUSTOM_MEMBER_INDEX_SETCONNECTINFO:
        if (3 == num && pParams[0].IsString() && pParams[1].IsString() && pParams[2].IsInt() && NULL != pRetValue){
          const char* pName = pParams[0].GetString();
          const char* pIP = pParams[1].GetString();
          int port = pParams[2].GetInt();
          if (pName && pIP){
            int r = 0;
            m_Client.SetConnectInfo(pName, pIP, (int)port);
            pRetValue->SetInt(r);
          } else {
            pRetValue->SetInt(0);
          }
        }
        break;
      case CUSTOM_MEMBER_INDEX_TICK:
        if (NULL != pRetValue)
        {
          unsigned int time = MyTimeGetTime();
          m_Client.Tick();
          pRetValue->SetInt(1);
        }
        break;
      case CUSTOM_MEMBER_INDEX_IS_CONNECTED:
        if (NULL != pRetValue)
        {
          int r = m_Client.IsConnected();
          pRetValue->SetInt(r);
        }
        break;
      case CUSTOM_MEMBER_INDEX_DISCONNECT:
        if (NULL != pRetValue)
        {
          m_Client.Disconnect();
          pRetValue->SetInt(1);
        }
        break;
      case CUSTOM_MEMBER_INDEX_MYHANDLE:
        if (NULL != pRetValue)
        {
          int r = m_Client.MyHandle();
          pRetValue->SetInt(r);
        }
        break;
      case CUSTOM_MEMBER_INDEX_TARGETHANLE:
        if (1 == num && pParams[0].IsString() && NULL != pRetValue)
        {
          int r = FALSE;
          const char* pName = pParams[0].GetString();
          if (pName){
            r = m_Client.TargetHandle(pName);
          }
          pRetValue->SetInt(r);
        }
        break;
      case CUSTOM_MEMBER_INDEX_TARGETNAME:
        if (1 == num && pParams[0].IsInt() && NULL != pRetValue)
        {
          int handle = pParams[0].GetInt();
          const char* pName = m_Client.TargetName(handle);
          pRetValue->AllocString(pName);
        }
        break;
      case CUSTOM_MEMBER_INDEX_CREATEPACKET:
        if (1 == num && pParams[0].IsInt() && NULL != pRetValue)
        {
          int maxSize = pParams[0].GetInt();
          BinaryStream* pStream = new BinaryStream(maxSize);
          CreateStreamPacketObj(*pRetValue, *m_Interpreter, m_Client, *pStream);
        }
        break;
      case CUSTOM_MEMBER_INDEX_SENDCOMMANDBYHANDLE:
        if (2 == num && pParams[0].IsInt() && pParams[1].IsString() && NULL != pRetValue)
        {
          int dest = pParams[0].GetInt();
          const char* result = pParams[1].GetString();
          if (result){
            int r = m_Client.SendCommandByHandle(dest, result);
            pRetValue->SetInt(r);
          } else {
            pRetValue->SetInt(0);
          }
        }
        break;
      case CUSTOM_MEMBER_INDEX_SENDCOMMANDBYANME:
        if (2 == num && pParams[0].IsString() && pParams[1].IsString() && NULL != pRetValue)
        {
          const char* name = pParams[0].GetString();
          const char* result = pParams[1].GetString();
          if (name && result){
            int r = m_Client.SendCommandByName(name, result);
            pRetValue->SetInt(r);
          } else {
            pRetValue->SetInt(0);
          }
        }
        break;
      }
    }
    return EXECUTE_RESULT_NORMAL;
  }
private:
  SimpleClient m_Client;
};

class StreamPacketObj : public ObjectBase
{
  enum
  {
    CUSTOM_MEMBER_INDEX_ADD_BYTE = 0,
    CUSTOM_MEMBER_INDEX_ADD_SHORT,
    CUSTOM_MEMBER_INDEX_ADD_INT,
    CUSTOM_MEMBER_INDEX_ADD_FLOAT,
    CUSTOM_MEMBER_INDEX_ADD_STRING,
    CUSTOM_MEMBER_INDEX_ADD_FIXED_STRING,
    CUSTOM_MEMBER_INDEX_GET_BYTE,
    CUSTOM_MEMBER_INDEX_GET_SHORT,
    CUSTOM_MEMBER_INDEX_GET_INT,
    CUSTOM_MEMBER_INDEX_GET_FLOAT,
    CUSTOM_MEMBER_INDEX_GET_STRING,
    CUSTOM_MEMBER_INDEX_GET_FIXED_STRING,
    CUSTOM_MEMBER_INDEX_GET_SIZE,
    CUSTOM_MEMBER_INDEX_SENDBYHANDLE,
    CUSTOM_MEMBER_INDEX_SENDBYNAME,
    CUSTOM_MEMBER_INDEX_DESTROY,
    CUSTOM_MEMBER_INDEX_RESET_CURSOR,
    CUSTOM_MEMBER_INDEX_SKIP,
    CUSTOM_MEMBER_INDEX_NUM,
  };
public:
  StreamPacketObj(Interpreter& interpreter, SimpleClient& client, BinaryStream* pStream) :ObjectBase(interpreter, CUSTOM_MEMBER_INDEX_NUM), m_Client(client), m_pStream(pStream)
  {}
  virtual ~StreamPacketObj(void)
  {}
protected:
  virtual int	 GetCustomInnerMemberIndex(const char* name)const
  {
    if (NULL == name)
      return -1;
    else if (strcmp(name, "addByte") == 0)
      return CUSTOM_MEMBER_INDEX_ADD_BYTE;
    else if (strcmp(name, "addShort") == 0)
      return CUSTOM_MEMBER_INDEX_ADD_SHORT;
    else if (strcmp(name, "addInt") == 0)
      return CUSTOM_MEMBER_INDEX_ADD_INT;
    else if (strcmp(name, "addFloat") == 0)
      return CUSTOM_MEMBER_INDEX_ADD_FLOAT;
    else if (strcmp(name, "addString") == 0)
      return CUSTOM_MEMBER_INDEX_ADD_STRING;
    else if (strcmp(name, "addFixedString") == 0)
      return CUSTOM_MEMBER_INDEX_ADD_FIXED_STRING;
    else if (strcmp(name, "getByte") == 0)
      return CUSTOM_MEMBER_INDEX_GET_BYTE;
    else if (strcmp(name, "getShort") == 0)
      return CUSTOM_MEMBER_INDEX_GET_SHORT;
    else if (strcmp(name, "getInt") == 0)
      return CUSTOM_MEMBER_INDEX_GET_INT;
    else if (strcmp(name, "getFloat") == 0)
      return CUSTOM_MEMBER_INDEX_GET_FLOAT;
    else if (strcmp(name, "getString") == 0)
      return CUSTOM_MEMBER_INDEX_GET_STRING;
    else if (strcmp(name, "getFixedString") == 0)
      return CUSTOM_MEMBER_INDEX_GET_FIXED_STRING;
    else if (strcmp(name, "getSize") == 0)
      return CUSTOM_MEMBER_INDEX_GET_SIZE;
    else if (strcmp(name, "sendByHandle") == 0)
      return CUSTOM_MEMBER_INDEX_SENDBYHANDLE;
    else if (strcmp(name, "sendByName") == 0)
      return CUSTOM_MEMBER_INDEX_SENDBYNAME;
    else if (strcmp(name, "destroy") == 0)
      return CUSTOM_MEMBER_INDEX_DESTROY;
    else if (strcmp(name, "resetCursor") == 0)
      return CUSTOM_MEMBER_INDEX_RESET_CURSOR;
    else if (strcmp(name, "skip") == 0)
      return CUSTOM_MEMBER_INDEX_SKIP;
    else
      return -1;
  }
  virtual ExecuteResultEnum ExecuteCustomMember(int index, int paramClass, Value* pParams, int num, Value* pRetValue)
  {
    if (NULL != pParams && NULL != m_pStream)
    {
      switch (index)
      {
      case CUSTOM_MEMBER_INDEX_ADD_BYTE:
      {
        if (1 == num && pParams[0].IsInt() && NULL != pRetValue)
        {
          unsigned char val = (unsigned char)pParams[0].GetInt();
          m_pStream->Add(val);
          pRetValue->SetInt(1);
        }
      }
        break;
      case CUSTOM_MEMBER_INDEX_ADD_SHORT:
      {
        if (1 == num && pParams[0].IsInt() && NULL != pRetValue)
        {
          short val = (short)pParams[0].GetInt();
          m_pStream->Add(val);
          pRetValue->SetInt(1);
        }
      }
        break;
      case CUSTOM_MEMBER_INDEX_ADD_INT:
      {
        if (1 == num && pParams[0].IsInt() && NULL != pRetValue)
        {
          int val = (int)pParams[0].GetInt();
          m_pStream->Add(val);
          pRetValue->SetInt(1);
        }
      }
        break;
      case CUSTOM_MEMBER_INDEX_ADD_FLOAT:
      {
        if (1 == num && pParams[0].IsFloat() && NULL != pRetValue)
        {
          float val = pParams[0].GetFloat();
          m_pStream->Add(val);
          pRetValue->SetInt(1);
        }
      }
        break;
      case CUSTOM_MEMBER_INDEX_ADD_STRING:
      {
        if (2 == num && pParams[0].IsString() && pParams[1].IsInt() && NULL != pRetValue)
        {
          const char* pStr = pParams[0].GetString();
          int sizeByteNum = pParams[1].GetInt();
          if (pStr)
          {
            m_pStream->Add(pStr, sizeByteNum);
            pRetValue->SetInt(1);
          } else
          {
            pRetValue->SetInt(0);
          }
        }
      }
        break;
      case CUSTOM_MEMBER_INDEX_ADD_FIXED_STRING:
      {
        if (2 == num && pParams[0].IsString() && pParams[1].IsInt() && NULL != pRetValue)
        {
          const char* pStr = pParams[0].GetString();
          int size = pParams[1].GetInt();
          if (pStr)
          {
            char* pBuf = new char[size + 1];
            if (pBuf)
            {
              memset(pBuf, 0, size + 1);
              tsnprintf(pBuf, size + 1, "%s", pStr);
              m_pStream->AddBlock(pBuf, size);
              pRetValue->SetInt(1);
              delete[] pBuf;
            } else
            {
              pRetValue->SetInt(0);
            }
          } else
          {
            pRetValue->SetInt(0);
          }
        }
      }
        break;
      case CUSTOM_MEMBER_INDEX_GET_BYTE:
      {
        if (NULL != pRetValue)
        {
          unsigned char val = 0;
          m_pStream->Get(val);
          pRetValue->SetInt(val);
        }
      }
        break;
      case CUSTOM_MEMBER_INDEX_GET_SHORT:
      {
        if (NULL != pRetValue)
        {
          short val = 0;
          m_pStream->Get(val);
          pRetValue->SetInt(val);
        }
      }
        break;
      case CUSTOM_MEMBER_INDEX_GET_INT:
      {
        if (NULL != pRetValue)
        {
          int val = 0;
          m_pStream->Get(val);
          pRetValue->SetInt(val);
        }
      }
        break;
      case CUSTOM_MEMBER_INDEX_GET_FLOAT:
      {
        if (NULL != pRetValue)
        {
          float val = 0;
          m_pStream->Get(val);
          pRetValue->SetFloat(val);
        }
      }
        break;
      case CUSTOM_MEMBER_INDEX_GET_STRING:
      {
        if (2 == num && pParams[0].IsInt() && pParams[1].IsInt() && NULL != pRetValue)
        {
          int size = pParams[0].GetInt();
          int sizeByteNum = pParams[1].GetInt();
          if (TRUE == pRetValue->AllocString(size))
          {
            char* pBuf = pRetValue->GetString();
            if (pBuf)
            {
              int realSize = m_pStream->Get(pBuf, size, sizeByteNum);
              if (realSize <= size)
              {
                pBuf[realSize] = 0;
              }
            } else
            {
              pRetValue->SetWeakRefConstString("");
            }
          } else
          {
            pRetValue->SetWeakRefConstString("");
          }
        }
      }
        break;
      case CUSTOM_MEMBER_INDEX_GET_FIXED_STRING:
      {
        if (1 == num && pParams[0].IsInt() && NULL != pRetValue)
        {
          int size = pParams[0].GetInt();
          if (TRUE == pRetValue->AllocString(size))
          {
            char* pBuf = pRetValue->GetString();
            if (pBuf)
            {
              memset(pBuf, 0, size);
              m_pStream->GetBlock(pBuf, size);
            } else
            {
              pRetValue->SetWeakRefConstString("");
            }
          } else
          {
            pRetValue->SetWeakRefConstString("");
          }
        }
      }
        break;
      case CUSTOM_MEMBER_INDEX_GET_SIZE:
      {
        if (NULL != pRetValue)
        {
          unsigned int size = m_pStream->GetStreamSize();
          pRetValue->SetInt((int)size);
        }
      }
        break;
      case CUSTOM_MEMBER_INDEX_SENDBYHANDLE:
      {
        if (1 == num && pParams[0].IsInt() && NULL != pRetValue)
        {
          int dest = pParams[0].GetInt();
          int r = m_Client.SendByHandle(dest, m_pStream->GetStreamData(), (int)m_pStream->GetStreamSize());
          pRetValue->SetInt(r);
        }
      }
        break;
      case CUSTOM_MEMBER_INDEX_SENDBYNAME:
      {
        if (1 == num && pParams[0].IsString() && NULL != pRetValue)
        {
          const char* name = pParams[0].GetString();
          if (NULL != name){
            int r = m_Client.SendByName(name, m_pStream->GetStreamData(), (int)m_pStream->GetStreamSize());
            pRetValue->SetInt(r);
          } else {
            pRetValue->SetInt(0);
          }
        }
      }
        break;
      case CUSTOM_MEMBER_INDEX_DESTROY:
      {
        if (NULL != pRetValue)
        {
          Destroy();
          pRetValue->SetInt(1);
        }
      }
        break;
      case CUSTOM_MEMBER_INDEX_RESET_CURSOR:
      {
        if (NULL != pRetValue)
        {
          m_pStream->ResetCursor();
          pRetValue->SetInt(1);
        }
      }
        break;
      case CUSTOM_MEMBER_INDEX_SKIP:
      {
        if (1 == num && pParams[0].IsInt() && NULL != pRetValue)
        {
          int size = pParams[0].GetInt();
          int r = m_pStream->Skip(size);
          pRetValue->SetInt(r);
        }
      }
        break;
      }
    }
    return EXECUTE_RESULT_NORMAL;
  }
private:
  void Destroy(void)
  {
    if (m_pStream)
    {
      delete m_pStream;
      m_pStream = NULL;
    }
    delete this;
  }
private:
  SimpleClient& m_Client;
  BinaryStream* m_pStream;
};

class ScriptThread : public Thread
{
public:
	enum
	{
		MAX_SCRIPT_NUM = 1024,
		MAX_LINE_SIZE= 512,
	};
private:
	struct ScriptLine
	{
		char m_Line[MAX_LINE_SIZE+1];
		ScriptLine(void)
		{
			m_Line[0]='\0';
		}
	};
	typedef DequeT<ScriptLine,MAX_SCRIPT_NUM> Scripts;
public:
	virtual void stop(void);
	virtual void run(void);
public:
	int GetIndex(void)const{return m_Index;}
	int PushLine(const char* p);
	int PopLine(char* p,int len);
	void MarkWaitingQuit(void){m_IsWaitingQuit=TRUE;}
	int IsWaitingQuit(void)const{return m_IsWaitingQuit;}
	void StopAfterTheLine(void){m_IsContinue=FALSE;}
public:
	ScriptThread(int index):m_Index(index),m_IsWaitingQuit(FALSE),m_IsContinue(TRUE),m_RunFlag(TRUE)
  {}
  SharedMessageQueue& GetSharedMessageQueue(void) { return m_SharedMessageQueue; }
private:
	void Reset(void)
	{
		m_IsWaitingQuit=FALSE;
		m_IsContinue=TRUE;
		m_RunFlag=TRUE;
	}
private:
	int	m_IsWaitingQuit;

	int	m_IsContinue;
	int	m_RunFlag;
	MyLock	m_Lock;
	Scripts	m_Scripts;

	int		m_Index;
  SharedMessageQueue  m_SharedMessageQueue;
};

int DecryptMemoryWithCyclone(char* pInMemory,int size,char*& pOutMemory,int& outSize,int& haveOutMemory);

class SharedStringMap
{
	typedef StringKeyT<128> StringType;
	typedef HashtableT<StringType,char*,StringType> StringMap;
public:
	void InitMap(int maxNum)
	{
		AutoLock_T lock(m_Lock);
		if(m_StringMap.IsInited())
		{
			_ClearMap();
		}
		m_StringMap.InitTable(maxNum);
	}
	int Set(const char* key,char* val)
	{
		AutoLock_T lock(m_Lock);
		int ret=FALSE;
		if(0!=key && 0!=val)
		{
			size_t size=strlen(val);
			char* p=new char[size+1];
			if(p)
			{
				memcpy(p,val,size);
				p[size]=0;
				char* pOld=m_StringMap.Get(key);
				if(pOld)
				{
					delete[] pOld;
				}
				m_StringMap.Remove(key);
				ret=m_StringMap.Add(key,p);
			}
		}
		return ret;
	}
	char* Get(const char* key)const
	{
		AutoLock_T lock(m_Lock);
		return m_StringMap.Get(key);
	}
	void Erase(const char* key)
	{
		AutoLock_T lock(m_Lock);
		char* pOld=m_StringMap.Get(key);
		if(pOld)
		{
			delete[] pOld;
		}
		m_StringMap.Remove(key);
	}
	int GetNum(void)const
	{
		AutoLock_T lock(m_Lock);
		return m_StringMap.GetNum();
	}
	void ClearMap(void)
	{
		AutoLock_T lock(m_Lock);
		_ClearMap();
	}
	void ListMap(const char* file)const
	{
		AutoLock_T lock(m_Lock);
		FILE* fp=fopen(file,"ab");
		StringMap::Iterator it=m_StringMap.First();
		for(;FALSE==it.IsNull();++it)
		{
			const char* key=it->GetKey().GetString();
			char* p=it->GetValue();
			printf("%s -> %s\n",key,p);
			if(fp)
			{
				fprintf(fp,"%s -> %s\n",key,p);
			}
		}
		if(fp)
		{
			fclose(fp);
		}
	}
public:
	SharedStringMap(void)
	{}
	virtual ~SharedStringMap(void)
	{
		_ClearMap();
	}
private:
	void _ClearMap(void)
	{
		StringMap::Iterator it=m_StringMap.First();
		for(;FALSE==it.IsNull();++it)
		{
			char* p=it->GetValue();
			if(p)
			{
				delete[] p;
			}
		}
		m_StringMap.CleanUp();
	}
private:
	StringMap		m_StringMap;
	mutable MyLock	m_Lock;
};

class SharedStringMapObj : public ObjectBase
{
	enum
	{
		CUSTOM_MEMBER_INDEX_INITMAP=0,
		CUSTOM_MEMBER_INDEX_GETNUM,
		CUSTOM_MEMBER_INDEX_FIND,
		CUSTOM_MEMBER_INDEX_INSERT,
		CUSTOM_MEMBER_INDEX_ERASE,
		CUSTOM_MEMBER_INDEX_CLEARMAP,
		CUSTOM_MEMBER_INDEX_LISTMAP,
		CUSTOM_MEMBER_INDEX_NUM,
	};
public:
	SharedStringMapObj(Interpreter& interpreter):ObjectBase(interpreter,CUSTOM_MEMBER_INDEX_NUM)
	{}
	virtual ~SharedStringMapObj(void)
	{}
protected:
	virtual int	 GetCustomInnerMemberIndex(const char* name)const
	{
		if(NULL==name)
			return -1;
		else if(strcmp(name,"initMap")==0)
			return CUSTOM_MEMBER_INDEX_INITMAP;
		else if(strcmp(name,"getNum")==0)
			return CUSTOM_MEMBER_INDEX_GETNUM;
		else if(strcmp(name,"find")==0)
			return CUSTOM_MEMBER_INDEX_FIND;
		else if(strcmp(name,"insert")==0)
			return CUSTOM_MEMBER_INDEX_INSERT;
		else if(strcmp(name,"erase")==0)
			return CUSTOM_MEMBER_INDEX_ERASE;
		else if(strcmp(name,"clearMap")==0)
			return CUSTOM_MEMBER_INDEX_CLEARMAP;
		else if(strcmp(name,"listMap")==0)
			return CUSTOM_MEMBER_INDEX_LISTMAP;
		else
			return -1;
	}
	virtual ExecuteResultEnum ExecuteCustomMember(int index,int paramClass,Value* pParams,int num,Value* pRetValue)
	{
		if(pParams && pRetValue && g_pMap)
		{
			switch(index)
			{
			case CUSTOM_MEMBER_INDEX_INITMAP:
				{
					if(1==num && pParams[0].IsInt())
					{
						int val=pParams[0].GetInt();
						g_pMap->InitMap(val);
						pRetValue->SetInt(1);
					}
				}
				break;
			case CUSTOM_MEMBER_INDEX_GETNUM:
				{
					int val=g_pMap->GetNum();
					pRetValue->SetInt(val);
				}
				break;
			case CUSTOM_MEMBER_INDEX_FIND:
				{
					if(1==num && pParams[0].IsString())
					{
						const char* pKey=pParams[0].GetString();
						if(NULL!=pKey)
						{
							char* p=g_pMap->Get(pKey);
							pRetValue->SetWeakRefString(p);
						}
						else
						{
							pRetValue->SetWeakRefString("");
						}
					}
				}
				break;
			case CUSTOM_MEMBER_INDEX_INSERT:
				{
					if(2==num && pParams[0].IsString() && pParams[1].IsString())
					{
						const char* pKey=pParams[0].GetString();
						char* pValue=pParams[1].GetString();
						if(NULL!=pKey && NULL!=pValue)
						{
							int r=g_pMap->Set(pKey,pValue);
							pRetValue->SetInt(r);
						}
						else
						{
							pRetValue->SetInt(0);
						}
					}
				}
				break;
			case CUSTOM_MEMBER_INDEX_ERASE:
				{
					if(1==num && pParams[0].IsString())
					{
						const char* pKey=pParams[0].GetString();
						if(NULL!=pKey)
						{
							g_pMap->Erase(pKey);
							pRetValue->SetInt(1);
						}
						else
						{
							pRetValue->SetInt(0);
						}
					}
				}
				break;
			case CUSTOM_MEMBER_INDEX_CLEARMAP:
				{
					g_pMap->ClearMap();
					pRetValue->SetInt(1);
				}
				break;
			case CUSTOM_MEMBER_INDEX_LISTMAP:
				{
					if(1==num && pParams[0].IsString())
					{
						const char* pFile=pParams[0].GetString();
						if(NULL!=pFile)
						{
							g_pMap->ListMap(pFile);
							pRetValue->SetInt(1);
						}
						else
						{
							pRetValue->SetInt(0);
						}
					}
				}
				break;
			}
		}
		return EXECUTE_RESULT_NORMAL;
	}
};

class ArgvApi : public ExpressionApi
{
  enum
  {
    MAX_STRING_SIZE = 1024,
  };
public:
  virtual ExecuteResultEnum Execute(int paramClass, Value* pParams, int num, Value* pRetValue)
  {
    if (m_Interpreter && pParams && pRetValue)
    {
      ReplaceVariableWithValue(pParams, num);
      if (1 == num && pParams[0].IsInt())
      {
        int ix = pParams[0].GetInt();
        if (ix >= 0 && ix < m_Argc)
        {
          pRetValue->SetWeakRefConstString(m_Argv[ix]);
        }
      }
    }
    return EXECUTE_RESULT_NORMAL;
  }
public:
  ArgvApi(Interpreter& interpreter) :ExpressionApi(interpreter), m_Argc(g_Argc), m_Argv(g_Argv){}
private:
  int m_Argc;
  char** m_Argv;
};

#endif //__ScriptThread_H__
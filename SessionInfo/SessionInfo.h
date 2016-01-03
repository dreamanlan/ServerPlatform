#ifndef SessionInfo_H
#define SessionInfo_H


namespace net_base
{
  class TcpSession;
}
using namespace net_base;

struct SessionInfo
{
  static const int MAX_SERVER_NAME_LENGTH = 255;

  int m_Handle;
  TcpSession* m_Session;
  char m_Name[MAX_SERVER_NAME_LENGTH + 1];

  SessionInfo() :m_Handle(0), m_Session(NULL)
  {
    m_Name[0] = 0;
  }
  bool IsValid(void) const { return m_Session != NULL; }
};

/*
* ServerCenterÏûÏ¢
*/
#pragma pack(push,1)

enum MessageEnum
{
  MSG_SC_MYNAME,
  MSG_CS_MYHANDLE,
  MSG_CS_ADD_NAME_HANDLE,
  MSG_CS_REMOVE_NAME_HANDLE,
  MSG_CS_CLEAR_NAME_HANDLE_LIST,
  MSG_SCS_TRANSMIT,
  MSG_SCS_TRANSMIT_RESULT,
  MSG_SCS_COMMAND,
};

struct MessageHeader
{
  char m_Class;

  MessageHeader(MessageEnum id) :m_Class((char)id)
  {}
  void SetClass(MessageEnum id)
  {
    m_Class = (char)id;
  }
};

struct MessageMyName : public MessageHeader
{
  char m_Name[1];

  MessageMyName() :MessageHeader(MSG_SC_MYNAME)
  {}
};

struct MessageMyHandle : public MessageHeader
{
  int m_Handle;

  MessageMyHandle() :MessageHeader(MSG_CS_MYHANDLE)
  {}
};

struct MessageAddNameHandle : public MessageHeader
{
  int m_Handle;
  char m_Name[1];

  MessageAddNameHandle() :MessageHeader(MSG_CS_ADD_NAME_HANDLE)
  {}
};

struct MessageRemoveNameHandle : public MessageHeader
{
  int m_Handle;
  char m_Name[1];

  MessageRemoveNameHandle() :MessageHeader(MSG_CS_REMOVE_NAME_HANDLE)
  {}
};

struct MessageClearNameHandleList : public MessageHeader
{
  MessageClearNameHandleList() :MessageHeader(MSG_CS_CLEAR_NAME_HANDLE_LIST)
  {}
};

struct MessageTransmit : public MessageHeader
{
  unsigned int m_Sequence;
  int m_Src;
  int m_Dest;
  char m_Data[1];

  MessageTransmit() :MessageHeader(MSG_SCS_TRANSMIT)
  {}
  bool IsValid(void) const
  {
    return m_Src > 0 && m_Dest > 0;
  }
};

struct MessageTransmitResult : public MessageHeader
{
  unsigned int m_Sequence;
  int m_Src;
  int m_Dest;
  bool m_IsSuccess;

  MessageTransmitResult() :MessageHeader(MSG_SCS_TRANSMIT_RESULT)
  {}
};

struct MessageCommand : public MessageHeader
{
  int m_Src;
  int m_Dest;
  char m_Command[1];

  MessageCommand() :MessageHeader(MSG_SCS_COMMAND)
  {}
};

#pragma pack(pop)

#endif //SessionInfo_H
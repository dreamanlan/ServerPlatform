#ifndef SessionManager_H
#define SessionManager_H

#include <event2/event.h>
#include <event2/thread.h>
#include "connector.h"
#include "Type.h"
#include "Hashtable.h"
#include "SessionInfo.h"

struct NameHandleInfo
{
    static const int MAX_SERVER_NAME_LENGTH = 255;

    uint64_t m_Handle;
    char m_Name[MAX_SERVER_NAME_LENGTH + 1];

    NameHandleInfo() :m_Handle(0)
    {
        m_Name[0] = 0;
    }
    bool IsValid() const { return m_Handle != 0; }
};

class SessionManager
{
    static const int MAX_IP_LEN = 128;
public:
    using StringKey = StringKeyT<NameHandleInfo::MAX_SERVER_NAME_LENGTH + 1>;
    using NameHandles = HashtableT<StringKey, NameHandleInfo, StringKey>;
public:
    void Init(event_base* pEventBase);
    void SetIpPort(int worldId, const char* pIp, int port);
    void UpdateMyName(const char* pName);
    void SetSession(TcpSession* session);
    TcpSession* GetSession() const { return m_Session; }
    unsigned int SessionSequence() const { return m_SessionSequence; }
    void IncSessionSequence() { ++m_SessionSequence; }
    uint64_t MyHandle() const { return m_MyselfInfo.m_Handle; }
    uint64_t TargetHandle(const char* name) const;
    const char* TargetName(uint64_t handle) const;
    void Tick();

public:
    bool OnConnectFinish(TcpSession* session, int err);
    bool OnSessionReceive(TcpSession* session, const char* data, int len);
    bool OnSessionError(TcpSession* session, int err);

public:
    SessionManager() :m_ConnectFinish(TRUE), m_Connector(NULL), m_Session(NULL), m_SessionSequence(1), m_WorldId(0), m_Port(0) {}
    ~SessionManager();
private:
    void KeepConnection();
private:
    void HandleMyHandle(TcpSession* session, const char* data, int len);
    void HandleAddNameHandle(TcpSession* session, const char* data, int len);
    void HandleRemoveNameHandle(TcpSession* session, const char* data, int len);
    void HandleClearNameHandles(TcpSession* session, const char* data, int len);
    void HandleTransmit(TcpSession* session, const char* data, int len);
    void HandleTransmitResult(TcpSession* session, const char* data, int len);
    void HandleCommand(TcpSession* session, const char* data, int len);

private:
    int m_ConnectFinish;
    Connector* m_Connector;
    TcpSession* m_Session;
    NameHandleInfo m_MyselfInfo;
    NameHandles m_NameHandles;
    unsigned int m_SessionSequence;

    int m_WorldId;
    char m_Ip[MAX_IP_LEN + 1];
    int m_Port;

    mutable MyLock m_Lock;

    static const int MAX_SESSION_NUM = 1024;
};

#endif //SessionManager_H
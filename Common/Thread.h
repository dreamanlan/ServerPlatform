#ifndef __THREAD_H__
#define __THREAD_H__

#include "Type.h"

class Thread
{
public:
    enum ThreadStatus
    {
        READY,
        RUNNING,
        EXIT
    };
public:
    Thread();
    virtual ~Thread();
public:
    void start();
    virtual void stop();
    void exit(void* retval = NULL);
    virtual void run();
public:
#if defined(__LINUX__)
    unsigned long long getTID() { return m_TID; }
#elif defined (__WINDOWS__)
    DWORD getTID() { return m_TID * 1000; }
#endif
    ThreadStatus getStatus() { return m_Status; }
    void setStatus(ThreadStatus status) { m_Status = status; }
private:
#if defined(__LINUX__)
    unsigned long long m_TID;
#elif defined (__WINDOWS__)
    DWORD m_TID;
#endif
    ThreadStatus m_Status;

#if defined(__WINDOWS__)
    HANDLE m_hThread;
#endif

};

extern unsigned int g_CreateThreadCount;
extern unsigned int g_QuitThreadCount;

extern void MySleep(unsigned int millionseconds);
extern unsigned long long MyGetCurrentThreadID();
extern unsigned long long MyGetCurrentTrueThreadID();
extern unsigned int MyTimeGetTime();

#if defined(__LINUX__)
void* MyThreadProcess(void* derivedThread);
#elif defined(__WINDOWS__)
DWORD WINAPI MyThreadProcess(void* derivedThread);
#endif

#endif

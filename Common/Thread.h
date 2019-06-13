#ifndef __THREAD_H__
#define __THREAD_H__

#include "Type.h"

class Thread 
{
public:	
	enum ThreadStatus 
	{
		READY ,
		RUNNING ,
		EXIT
	};
public:
	Thread(void);
	virtual ~Thread(void);
public:
	void start(void);
	virtual void stop(void);
	void exit(void * retval = NULL);
	virtual void run(void);
public:
#if defined(__LINUX__)
	unsigned long long getTID(void){return m_TID;}
#elif defined (__WINDOWS__)
	DWORD getTID(void){return m_TID*1000;}
#endif
	ThreadStatus getStatus(void){return m_Status;}
	void setStatus(ThreadStatus status){m_Status = status;}
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

extern void MySleep( unsigned int millionseconds );
extern unsigned long long MyGetCurrentThreadID(void);
extern unsigned long long MyGetCurrentTrueThreadID(void);
extern unsigned int MyTimeGetTime(void);

#if defined(__LINUX__)
void * MyThreadProcess(void* derivedThread);
#elif defined(__WINDOWS__)
DWORD WINAPI MyThreadProcess(void* derivedThread);
#endif

#endif

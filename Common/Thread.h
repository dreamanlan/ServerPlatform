#ifndef __THREAD_H__
#define __THREAD_H__

#include "Type.h"

class Thread 
{
public:	
	enum ThreadStatus 
	{
		READY ,		// 当前线程处于准备状态
		RUNNING ,	// 处于运行状态
		EXIT		// 已经退出 
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
	unsigned long getTID(void){return m_TID;}
#elif defined (__WINDOWS__)
	unsigned long getTID(void){return m_TID*1000;}
#endif
	ThreadStatus getStatus(void){return m_Status;}
	void setStatus(ThreadStatus status){m_Status = status;}
private:
	unsigned long m_TID;
	ThreadStatus m_Status;

#if defined(__WINDOWS__)
	HANDLE m_hThread;
#endif

};

extern unsigned int g_CreateThreadCount;
extern unsigned int g_QuitThreadCount;

extern void MySleep( unsigned int millionseconds );
extern unsigned long MyGetCurrentThreadID(void);
extern unsigned long MyGetCurrentTrueThreadID(void);
extern unsigned int MyTimeGetTime(void);

#if defined(__LINUX__)
void * MyThreadProcess(void* derivedThread);
#elif defined(__WINDOWS__)
DWORD WINAPI MyThreadProcess(void* derivedThread);
#endif

#endif

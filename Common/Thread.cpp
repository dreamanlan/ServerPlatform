#include "Thread.h"

unsigned int g_CreateThreadCount = 0; 
unsigned int g_QuitThreadCount = 0;
MyLock g_thread_lock;

Thread::Thread()
{
	m_TID		= 0;
	m_Status	= Thread::READY;

#if defined(__WINDOWS__)
	m_hThread = NULL;
#endif
}

Thread::~Thread()
{}

void Thread::start()
{		
	if(m_Status != Thread::READY)
		return;

#if defined(__LINUX__)
	pthread_create((pthread_t*)&m_TID, NULL , MyThreadProcess , this);
#elif defined(__WINDOWS__)
	m_hThread = CreateThread(NULL, 0, MyThreadProcess , this, 0, &m_TID);
#endif
}

void Thread::stop()
{}

void Thread::exit(void * retval)
{
	try
	{
		#if defined(__LINUX__)
			pthread_exit(retval);
		#elif defined(__WINDOWS__)
			CloseHandle(m_hThread);
		#endif
	}
	catch(...)
	{}
}

#if defined(__LINUX__)
void * MyThreadProcess(void * derivedThread)
{
	Thread * thread =(Thread *)derivedThread;
	if(thread==NULL)
		return NULL;

	{
		AutoLock_T autolock(g_thread_lock);
		g_CreateThreadCount++;
	}while(FALSE);

	// set thread's status to "RUNNING"
	thread->setStatus(Thread::RUNNING);

	// here - polymorphism used.(derived::run() called.)
	thread->run();
	
	// set thread's status to "EXIT"
	thread->setStatus(Thread::EXIT);
	
	//INT ret = 0;
	//thread->exit(&ret);

	{
		AutoLock_T autolock(g_thread_lock);
		g_QuitThreadCount++;
	}while(FALSE);

	thread->setStatus(Thread::READY);

	return NULL;	// avoid compiler's warning
}
#elif defined(__WINDOWS__)
DWORD WINAPI MyThreadProcess(void* derivedThread)
{
	Thread * thread =(Thread *)derivedThread;
	if(thread==NULL)
		return 0;
	
	do{
		AutoLock_T autolock(g_thread_lock);
		g_CreateThreadCount++;
	}while(FALSE);

	// set thread's status to "RUNNING"
	thread->setStatus(Thread::RUNNING);

	// here - polymorphism used.(derived::run() called.)
	thread->run();
	
	// set thread's status to "EXIT"
	thread->setStatus(Thread::EXIT);

	thread->exit(NULL);

	do{
		AutoLock_T autolock(g_thread_lock);
		g_QuitThreadCount++; 
	}while(FALSE);

	thread->setStatus(Thread::READY);

	return 0;	// avoid compiler's warning
}
#endif

void Thread::run()
{}

void MySleep( unsigned int millionseconds )
{
#if defined(__WINDOWS__)
	Sleep( millionseconds );
#elif defined(__LINUX__)
	usleep( millionseconds*1000 );
#endif
}

unsigned long MyGetCurrentThreadID(void)
{
	unsigned long tRet = MyGetCurrentTrueThreadID();
	int nValue = 0;//g_ThreadValue.GetThreadValue( tRet );
	tRet += nValue;

	return tRet;
}

unsigned long MyGetCurrentTrueThreadID(void)
{
#if defined(__WINDOWS__)
	return GetCurrentThreadId( )*1000;
#elif defined(__LINUX__)
	return pthread_self();
#endif
}

unsigned int MyTimeGetTime(void)
{
#if defined(__WINDOWS__)
	return GetTickCount();
#elif defined(__LINUX__)
	struct timeval _t;
	struct timezone tz;
	gettimeofday(&_t,&tz);
	double t =  (double)_t.tv_sec*1000 + (double)_t.tv_usec/1000;
	return (unsigned int)t;
#endif
}
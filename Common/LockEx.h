#ifndef __LockEx_H__
#define __LockEx_H__

#if defined(__WINDOWS__)

class MyLock
{
public:
	MyLock()
	{ 
		::InitializeCriticalSection(&m_Lock);
	}
	~MyLock()
	{ 
		::DeleteCriticalSection(&m_Lock); 
	}

	void Lock(void)
	{ 
		::EnterCriticalSection(&m_Lock); 
	}

	void Unlock(void)
	{ 
		::LeaveCriticalSection(&m_Lock); 
	}

private:
	CRITICAL_SECTION m_Lock ;
};

#elif defined(__LINUX__)

class MyLock
{
public :
	MyLock()
	{ 
		pthread_mutex_init(&m_Mutex , NULL);
	}
	~MyLock()
	{ 
		pthread_mutex_destroy(&m_Mutex);
	}

	void Lock(void)
	{ 
		pthread_mutex_lock(&m_Mutex); 
	}

	void Unlock(void)
	{ 
		pthread_mutex_unlock(&m_Mutex); 
	}

private:
	pthread_mutex_t 	m_Mutex;
};

#endif

//自动加锁解锁器
class AutoLock_T
{
public:
	AutoLock_T(MyLock& rLock)
	{
		m_pLock = &rLock;
		m_pLock->Lock();
	}

	AutoLock_T(MyLock* pLock)
	{
		m_pLock = pLock;

		if(m_pLock)
		{
			m_pLock->Lock();
		}
	}

	~AutoLock_T()
	{
		if(m_pLock)
		{
			m_pLock->Unlock();
			m_pLock = 0;
		}
	}

private:
	AutoLock_T();
	MyLock* m_pLock;
};

//自动加锁解锁器
template<typename LockT>
class AutoLockT
{
public:
	AutoLockT(LockT& rLock)
	{
		m_pLock = &rLock;
		m_pLock->Lock();
	}
	AutoLockT(LockT* pLock)
	{
		m_pLock = pLock;

		if(m_pLock)
		{
			m_pLock->Lock();
		}
	}
	~AutoLockT(void)
	{
		if(m_pLock)
		{
			m_pLock->Unlock();
			m_pLock = 0;
		}
	}
private:
	AutoLockT(void);
	LockT* m_pLock;
};

//虚拟锁，符合MyLock的接口，但不执行锁操作。
class DummyLock
{
public :
	DummyLock(void){}
	~DummyLock(void){}	
	void Lock(void){}
	void Unlock(void){}
};

#endif //__LockEx_H__
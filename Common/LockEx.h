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

    void Lock()
    {
        ::EnterCriticalSection(&m_Lock);
    }

    void Unlock()
    {
        ::LeaveCriticalSection(&m_Lock);
    }

private:
    CRITICAL_SECTION m_Lock;
};

#elif defined(__LINUX__)

class MyLock
{
public:
    MyLock()
    {
        pthread_mutex_init(&m_Mutex, NULL);
    }
    ~MyLock()
    {
        pthread_mutex_destroy(&m_Mutex);
    }

    void Lock()
    {
        pthread_mutex_lock(&m_Mutex);
    }

    void Unlock()
    {
        pthread_mutex_unlock(&m_Mutex);
    }

private:
    pthread_mutex_t 	m_Mutex;
};

#endif

//Automatic lock and unlocker
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

        if (m_pLock) {
            m_pLock->Lock();
        }
    }

    ~AutoLock_T()
    {
        if (m_pLock) {
            m_pLock->Unlock();
            m_pLock = 0;
        }
    }

private:
    AutoLock_T();
    MyLock* m_pLock;
};

//Automatic lock and unlocker
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

        if (m_pLock) {
            m_pLock->Lock();
        }
    }
    ~AutoLockT()
    {
        if (m_pLock) {
            m_pLock->Unlock();
            m_pLock = 0;
        }
    }
private:
    AutoLockT();
    LockT* m_pLock;
};

//Virtual lock, conforms to the MyLock interface, but does not perform lock operations.
class DummyLock
{
public:
    DummyLock() {}
    ~DummyLock() {}
    void Lock() {}
    void Unlock() {}
};

#endif //__LockEx_H__
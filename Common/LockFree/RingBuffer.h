#ifndef __RingBuffer_H__
#define __RingBuffer_H__

#include <memory.h>
#include "Atomic.h"

class LockFreeRingedBuffer
{
    static const int s_c_MaxRetryCount = 256;

#ifdef __WINDOWS__
    using InfoPosAndTagType = unsigned short;
#else
    using InfoPosAndTagType = unsigned int;
#endif

#ifdef __WINDOWS__
    __declspec(align(8))
#endif
        struct Pointer
    {
        volatile unsigned long		m_BufPos;
        union
        {
            struct
            {
                volatile InfoPosAndTagType	m_InfoPos;
                volatile InfoPosAndTagType	m_Tag;
            } m_IPT;
            volatile unsigned long	m_IPTV;
        };
        inline void Clear() volatile
        {
            m_BufPos = 0;
            m_IPTV = 0;
        }
        inline void CopyFrom(volatile const Pointer& other) volatile
        {
            m_BufPos = other.m_BufPos;
            m_IPTV = other.m_IPTV;
        }
        inline void AssignTo(volatile Pointer& other) const volatile
        {
            other.m_BufPos = m_BufPos;
            other.m_IPTV = m_IPTV;
        }
    }
#ifndef __WINDOWS__
    __attribute((aligned(16)))
#endif
        ;
    struct Info
    {
        volatile unsigned long m_StartPos;
        volatile unsigned long m_EndPos;
        volatile unsigned long m_Flag;

        inline Info() :m_StartPos(0), m_EndPos(0), m_Flag(0)
        {}
        inline void Set(volatile unsigned long startPos, volatile unsigned long endPos) volatile
        {
            m_StartPos = startPos;
            m_EndPos = endPos;
            unsigned long old = lock_free_utility::xchg(&m_Flag, 1);
#if	defined(LockFreeDebugPrint)
            if (old != 0) {
                LockFreeDebugPrint("error old-flag!=0 in RingedBuffer::Info::Set\n");
            }
#endif
        }
        inline void Clear() volatile
        {
            m_StartPos = m_EndPos = 0;
            unsigned long old = lock_free_utility::xchg(&m_Flag, 0);
#if	defined(LockFreeDebugPrint)
            if (old == 0) {
                LockFreeDebugPrint("error old-flag==0 in RingedBuffer::Info::Clear\n");
            }
#endif
        }
        inline bool Valid() const volatile
        {
            return 0 != m_Flag;
        }
    };
#ifdef __WINDOWS__
    static const unsigned int s_c_MaxInfoNum = 0xffff;
#else
    static const unsigned int s_c_MaxInfoNum = 0xffffffff;
#endif

public:
    enum RINGBUF_RET
    {
        PUSH_SUCESSFUL,		/* push�ɹ�		*/
        PUSH_FAIL_BUFFUL,	/* pushʧ�ܣ�д����ֽڳ����˻��������ֽ���				*/
        PUSH_FAIL_RETRYFUL, /* pushʧ�ܣ����Դ�������255��							*/
        PUSH_FAIL_INVALID_ARG,/* pushʧ�ܣ������������							*/
    };

public:
    inline void Init(unsigned long maxSize)
    {
        Destroy();
        m_pBuffer = new char[maxSize];
        if (NULL != m_pBuffer) {
            m_MaxSize = maxSize;
            m_MaxInfoNum = s_c_MaxInfoNum;
            if (maxSize < m_MaxInfoNum)
                m_MaxInfoNum = maxSize;
            m_pInfo = new Info[m_MaxInfoNum];
            if (NULL == m_pInfo) {
                Destroy();
            }
        }
    }
    inline void Destroy()
    {
        if (NULL != m_pBuffer) {
            delete[] m_pBuffer;
            m_pBuffer = NULL;
        }
        if (NULL != m_pInfo) {
            delete[] m_pInfo;
            m_pInfo = NULL;
        }
        m_Head.Clear();
        m_Tail.Clear();
        m_MaxSize = 0;
    }
    inline unsigned long GetSize()const
    {
        return (m_Tail.m_BufPos + m_MaxSize - m_Head.m_BufPos) % m_MaxSize;
    }


    /*	-1 ����Ϊ��Tail��Head�غ�ʱ������Ϊ��ʱ������Ϊ�գ������д����ֽڱȿ����ֽ�С1��	*/
    inline unsigned long GetFreeSize()const
    {
        return m_MaxSize - (m_Tail.m_BufPos + m_MaxSize - m_Head.m_BufPos) % m_MaxSize - 1;
    }

    inline unsigned int GetNum()const
    {
        return (m_Tail.m_IPT.m_InfoPos + m_MaxInfoNum - m_Head.m_IPT.m_InfoPos) % m_MaxInfoNum;
    }
    inline unsigned long GetMaxSize()const { return m_MaxSize; }
    inline unsigned int GetMaxNum()const { return m_MaxInfoNum; }
    //lock-free
    inline RINGBUF_RET Push(const char* buf, int size, int& retryTime)
    {
        if (NULL == buf || size <= 0) {
            return PUSH_FAIL_INVALID_ARG;
        }
        RINGBUF_RET ret = PUSH_FAIL_RETRYFUL;
        //Take up space first
        Pointer tail, newTail;
        tail.Clear();
        newTail.Clear();
        for (int i = 0; i < s_c_MaxRetryCount; ++i) {
            tail.CopyFrom(m_Tail);
            newTail.CopyFrom(tail);
            newTail.m_BufPos = (newTail.m_BufPos + size) % m_MaxSize;
            newTail.m_IPT.m_InfoPos = (newTail.m_IPT.m_InfoPos + 1) % m_MaxInfoNum;
            ++newTail.m_IPT.m_Tag;

            if (size > 0 && (unsigned long)size > GetFreeSize() || newTail.m_IPT.m_InfoPos == m_Head.m_IPT.m_InfoPos) {
                ret = PUSH_FAIL_BUFFUL;
                break;
            }

            if (m_pInfo[tail.m_IPT.m_InfoPos].Valid()) {
                lock_free_utility::pause();
                continue;
            }
            if (lock_free_utility::compare_and_swap(reinterpret_cast<volatile unsigned long*>(&m_Tail), tail.m_BufPos, tail.m_IPTV, newTail.m_BufPos, newTail.m_IPTV)) {
                ret = PUSH_SUCESSFUL;
                retryTime = i;
                break;
            }
            else {
                lock_free_utility::pause();
            }
        }
        if (PUSH_SUCESSFUL == ret) {
            if (tail.m_BufPos <= newTail.m_BufPos)
                memcpy((char*)m_pBuffer + tail.m_BufPos, buf, size);
            else {
                int leftSpace = m_MaxSize - tail.m_BufPos;
                memcpy((char*)m_pBuffer + tail.m_BufPos, buf, leftSpace);
                memcpy((char*)m_pBuffer, buf + leftSpace, newTail.m_BufPos);
            }
            volatile Info& info = m_pInfo[tail.m_IPT.m_InfoPos];
            //mark last
            info.Set(tail.m_BufPos, newTail.m_BufPos);
        }
        return ret;
    }
    //lock-free
    inline bool Pop(char* buf, int& size)
    {
        bool ret = false;
        //Take up space first
        Pointer head, newHead;
        head.Clear();
        newHead.Clear();
        for (int i = 0; i < s_c_MaxRetryCount; ++i) {
            head.CopyFrom(m_Head);
            newHead.CopyFrom(head);
            newHead.m_IPT.m_InfoPos = (newHead.m_IPT.m_InfoPos + 1) % m_MaxInfoNum;
            ++newHead.m_IPT.m_Tag;

            if (head.m_IPT.m_InfoPos == m_Tail.m_IPT.m_InfoPos || !m_pInfo[head.m_IPT.m_InfoPos].Valid()) {
                //If the queue is empty or the content has not been written in, it will fail directly.
                size = 0;
                break;
            }
            //Read operations to unprotected memory are placed after memory validity judgment
            volatile Info& info = m_pInfo[head.m_IPT.m_InfoPos];
            newHead.m_BufPos = info.m_EndPos;

            if (size < 0 || (unsigned long)size < (newHead.m_BufPos - head.m_BufPos + m_MaxSize) % m_MaxSize) {
                size = (newHead.m_BufPos + m_MaxSize - head.m_BufPos) % m_MaxSize;
                break;
            }
            if (NULL == buf) {
                return false;
            }
            if (lock_free_utility::compare_and_swap(reinterpret_cast<volatile unsigned long*>(&m_Head), head.m_BufPos, head.m_IPTV, newHead.m_BufPos, newHead.m_IPTV)) {
                ret = true;
                break;
            }
            else {
                lock_free_utility::pause();
            }
        }
        //Take away data
        if (ret) {
            size = (newHead.m_BufPos + m_MaxSize - head.m_BufPos) % m_MaxSize;
            if (head.m_BufPos < newHead.m_BufPos)
                memcpy(buf, (const char*)m_pBuffer + head.m_BufPos, size);
            else {
                unsigned long leftSpace = m_MaxSize - head.m_BufPos;
                memcpy(buf, (const char*)m_pBuffer + head.m_BufPos, leftSpace);
                memcpy(buf + leftSpace, (const char*)m_pBuffer, newHead.m_BufPos);
            }
            volatile Info& info = m_pInfo[head.m_IPT.m_InfoPos];
            //Last clear mark
            info.Clear();
        }
        return ret;
    }
public:
    LockFreeRingedBuffer() :m_pBuffer(NULL), m_pInfo(NULL), m_MaxSize(0), m_MaxInfoNum(0)
    {
        m_Head.Clear();
        m_Tail.Clear();
    }
    ~LockFreeRingedBuffer() { Destroy(); }
private:
    LockFreeRingedBuffer(const LockFreeRingedBuffer& other);
    LockFreeRingedBuffer& operator=(const LockFreeRingedBuffer& other);
private:
    volatile char* m_pBuffer;
    volatile Info* m_pInfo;
    volatile Pointer	m_Head;
    volatile Pointer	m_Tail;
    unsigned long		m_MaxSize;
    unsigned int		m_MaxInfoNum;
};

#endif //__RingBuffer_H__
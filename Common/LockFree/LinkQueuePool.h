#ifndef __LinkQueuePool_H__
#define __LinkQueuePool_H__

#include "RingQueue.h"

template<class NodeT>
class LockFreeLinkedNodePoolT
{
public:
	typedef LockFreeRingedQueueT<NodeT*> FreeQueue;
public:
	inline void Init(int maxNum)
	{
		m_pData=new NodeT[maxNum];
		if(NULL!=m_pData)
		{
			m_FreeQueue.Init(maxNum);
			for(int i=0;i<maxNum;++i)
			{
				m_FreeQueue.Push(&(m_pData[i]));
			}
		}
	}
	inline volatile NodeT* Alloc(void)
	{
		NodeT* pNode=NULL;
		lock_free_utility::increment(&m_Count);
		for(int i=0;i<256;++i)
		{
			if(m_FreeQueue.Pop(pNode))
			{
				break;
			}
			else
			{
				lock_free_utility::pause();
			}
		}
		return pNode;
	}
	inline void Free(volatile NodeT* p)
	{
		if(NULL!=p)
		{
			NodeT* _p=const_cast<NodeT*>(p);
			lock_free_utility::decrement(&m_Count);
			for(int i=0;i<256;++i)
			{
				if(m_FreeQueue.Push(_p))
				{
					break;
				}
				else
				{
					lock_free_utility::pause();
				}
			}
		}
	}
	inline unsigned long GetCount(void)const
	{
		return m_Count;
	}
public:
	LockFreeLinkedNodePoolT(void):m_Count(0)
	{}
	~LockFreeLinkedNodePoolT(void)
	{
		m_FreeQueue.Clear();
		if(NULL!=m_pData)
		{
			delete[] m_pData;
			m_pData=NULL;
		}
		m_Count=0;
	}
private:
	LockFreeLinkedNodePoolT(const LockFreeLinkedNodePoolT& other);
	LockFreeLinkedNodePoolT& operator=(const LockFreeLinkedNodePoolT& other);
private:
	volatile unsigned long	m_Count;
private:
	NodeT*			m_pData;
	FreeQueue		m_FreeQueue;
};

#endif //__LinkQueuePool_H__
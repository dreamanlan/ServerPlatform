#ifndef __LINKEDQUEUE_H__
#define __LINKEDQUEUE_H__

#include <stddef.h>
#include "Atomic.h"
#pragma warning( push )
#pragma warning( disable : 4311 )

template<class NodeT>
class LinkedNodePoolT
{
public:
	inline void Init(int maxNum)
	{
	}
	inline volatile NodeT* Alloc(void)
	{
		lock_free_utility::increment(&m_Count);
		return new NodeT;
	}
	inline void Free(volatile NodeT* p)
	{
		if(NULL!=p)
		{
			lock_free_utility::decrement(&m_Count);
			delete p;
		}
	}
	inline unsigned long GetCount(void)const
	{
		return m_Count;
	}
public:
	LinkedNodePoolT(void):m_Count(0)
	{}
private:
	volatile unsigned long m_Count;
};

/**
*	基于链表的lock-free队列，与LockFreeRingedQueue不同，它可以管理指针与普通数据结构。
*	@ref
*	《Simple, Fast, and Practical Non-Blocking and Blocking Concurrent Queue Algorithms》Maged M. Michael、 Michael L. Scott， 1996
*/
template<class DataT,template<class T> class NodePoolT=LinkedNodePoolT>
class LockFreeLinkedQueueT
{
	static const int s_c_MaxRetryCount=256;

	typedef LockFreeLinkedQueueT<DataT>	ThisType;
	struct Node
	{
#ifdef __WINDOWS__
		//32位下按8字节对齐
		__declspec(align(8))
#endif
		struct Pointer
		{
			volatile Node*			m_Ptr;
			volatile unsigned long	m_Count;

			inline Pointer(void)
			{
				Clear();
			}
			volatile inline bool IsNull(void)const volatile
			{
				return NULL==m_Ptr;
			}
			volatile inline void Clear(void) volatile
			{
				m_Ptr=NULL;
				m_Count=0;
			}
			inline Pointer(volatile const Pointer& other)
			{
				m_Ptr=other.m_Ptr;
				m_Count=other.m_Count;
			}
			volatile inline Pointer& operator=(volatile const Pointer& other) volatile
			{
				if(this==&other)
					return *this;
				else
				{
					m_Ptr=other.m_Ptr;
					m_Count=other.m_Count;
					return *this;
				}
			}
		}
#ifndef __WINDOWS__
		//64位下按16字节对齐
		__attribute__ ((aligned (16)))
#endif
		;

		Pointer				m_Next;
		DataT				m_Data;
	};
	typedef NodePoolT<Node> NodePoolType;
public:
	//此方法多线程不安全
	inline void Init(int maxNum)
	{
		m_MaxNum=maxNum;
		m_NodePool.Init(maxNum);
		m_Head.m_Ptr=m_Tail.m_Ptr=m_NodePool.Alloc();
	}
	//此方法多线程不安全
	inline void	Clear(void)
	{
		while(m_Num)
		{
			Pop();
		}
	}
	//因为队列是lock-free的，此值仅提供参考信息，不能据此进行准确判断
	inline int GetNum(void)const
	{
		return m_Num;
	}
	//此方法为lock-free的
	inline bool	Push(const DataT& data)
	{	
		bool ret=false;
		if(m_Num<m_MaxNum)
		{
			volatile Node* pNode=m_NodePool.Alloc();
			if(NULL!=pNode)
			{
				pNode->m_Data=data;
				pNode->m_Next.m_Ptr=NULL;
				volatile typename Node::Pointer tail;
				for(int i=0;i<s_c_MaxRetryCount;++i)
				{
					tail=m_Tail;
					//CrashAssert(!tail.IsNull());
					volatile typename Node::Pointer next=tail.m_Ptr->m_Next;
					if(tail.m_Ptr==m_Tail.m_Ptr && tail.m_Count==m_Tail.m_Count)
					{
						if(next.IsNull())
						{
							//没有下一个结点，装到尾部
							if(lock_free_utility::compare_and_swap(reinterpret_cast<volatile unsigned long*>(&m_Tail.m_Ptr->m_Next),reinterpret_cast<unsigned long>(next.m_Ptr),next.m_Count,reinterpret_cast<unsigned long>(pNode),next.m_Count+1))
							{
								ret=true;
								break;
							}
							else
							{
								lock_free_utility::pause();
							}
						}
						else
						{
							//尾指针不正确，调整
							if(!lock_free_utility::compare_and_swap(reinterpret_cast<volatile unsigned long*>(&m_Tail),reinterpret_cast<unsigned long>(tail.m_Ptr),tail.m_Count,reinterpret_cast<unsigned long>(next.m_Ptr),tail.m_Count+1))
							{
								lock_free_utility::pause();
							}
						}
					}
				}
				if(ret)
				{
					//已经装好了，调整尾指针的位置，失败了什么也不用做，下次push会正确调整尾指针
					lock_free_utility::compare_and_swap(reinterpret_cast<volatile unsigned long*>(&m_Tail),reinterpret_cast<unsigned long>(tail.m_Ptr),tail.m_Count,reinterpret_cast<unsigned long>(pNode),tail.m_Count+1);
					lock_free_utility::increment(&m_Num);
				}
			}
		}
		return ret;
	}
	//此方法为lock-free的
	inline bool Pop(DataT& data)
	{
		bool ret=false;
		volatile typename Node::Pointer head;
		for(int i=0;i<s_c_MaxRetryCount;++i)
		{
			head=m_Head;
			volatile typename Node::Pointer tail=m_Tail;
			//CrashAssert(!head.IsNull());
			volatile typename Node::Pointer next=head.m_Ptr->m_Next;
			if(head.m_Ptr==m_Head.m_Ptr && head.m_Count==m_Head.m_Count)
			{
				if(head.m_Ptr==tail.m_Ptr)
				{
					if(next.IsNull())
					{
						//失败，队列空
						break;
					}
					else
					{
						//尾指针不正确，调整
						if(!lock_free_utility::compare_and_swap(reinterpret_cast<volatile unsigned long*>(&m_Tail),reinterpret_cast<unsigned long>(tail.m_Ptr),tail.m_Count,reinterpret_cast<unsigned long>(next.m_Ptr),tail.m_Count+1))
						{
							lock_free_utility::pause();
						}
					}
				}
				else
				{
					if(next.IsNull())
					{
						//这情况属于异常
						break;
					}
					else
					{
						data=next.m_Ptr->m_Data;
						if(lock_free_utility::compare_and_swap(reinterpret_cast<volatile unsigned long*>(&m_Head),reinterpret_cast<unsigned long>(head.m_Ptr),head.m_Count,reinterpret_cast<unsigned long>(next.m_Ptr),head.m_Count+1))
						{
							ret=true;
							break;
						}
						else
						{
							lock_free_utility::pause();
						}
					}					
				}
			}
		}
		if(ret)
		{
			lock_free_utility::decrement(&m_Num);
			if(head.m_Ptr)
			{
				m_NodePool.Free(head.m_Ptr);
			}
		}
		return ret;
	}
	inline DataT Pop(const DataT& invalidValue=DataT())
	{
		DataT data;
		if(Pop(data))
		{
			return data;
		}
		else
		{
			return invalidValue;
		}
	}
public:
	inline LockFreeLinkedQueueT(void):m_Num(0),m_MaxNum(0)
	{}
	inline ~LockFreeLinkedQueueT(void)
	{
		Clear();
		if(m_Head.m_Ptr)
		{
			m_NodePool.Free(m_Head.m_Ptr);
		}
#if	defined(LockFreeDebugPrint)
		unsigned int ct=m_NodePool.GetCount();
		if(ct)
		{
			LockFreeDebugPrint("error count %u in LinkedQueue after destructor !!!\n",(unsigned int)ct);
		}
#endif
	}
private:
	LockFreeLinkedQueueT(const LockFreeLinkedQueueT& other);
	LockFreeLinkedQueueT& operator=(const LockFreeLinkedQueueT& other);
private:
	volatile typename Node::Pointer	m_Head;
	volatile typename Node::Pointer	m_Tail;
	volatile int					m_Num;
	int								m_MaxNum;
	NodePoolType					m_NodePool;
};

#pragma warning( pop )
#endif
#ifndef __ChainNodePool_H__
#define __ChainNodePool_H__

#include "Chain.h"
/**
*	链结点池，使用此结点池的链结点类的模板参数RecyclerT需要增加一个额外约束：
*	提供公有方法void SetPool(CChainNodePoolT<DataT,RecyclerT>* pPool)。
*/
template<typename DataT,typename RecyclerT,typename LockT = DummyLock,int SizeV = 0>
class CChainNodePoolT
{
public:
	typedef CChainNode<DataT,RecyclerT> DataNode;
	typedef typename DataNode::Iterator Iterator;
	typedef DequeT<DataNode*,SizeV> DataNodeQueue;
	typedef typename CollectionMemory::SelectorT<DataNode,SizeV>::Type MemoryType;
public:
	void						Init(int num);
	void						Cleanup(void);
public:
	const Iterator				NewNode(void);
	const Iterator				GetNode(int index) const;
	int							GetMaxNum(void)const{return m_Num;}
	int							GetUnusedNum(void)const{return m_UnusedDataNodes.Size();}
public:
	virtual void				Recycle(DataNode* pNode);
public:
	CChainNodePoolT(void):m_Num(0),m_DataNodes(NULL)
	{
		if(SizeV>0)
		{
			Init(SizeV);
		}
	}
	CChainNodePoolT(int num):m_Num(0),m_DataNodes(NULL)
	{
		Init(num);
	}
	virtual ~CChainNodePoolT(void)
	{
		Cleanup();
	}
public://内存池的拷贝构造与复制不复制数据
	CChainNodePoolT(const CChainNodePoolT& other)
	{
		Init(other.GetMaxNum());
	}
	CChainNodePoolT& operator=(const CChainNodePoolT& other)
	{
		Cleanup();
		Init(other.GetMaxNum());
	}
protected:
	inline void Create(unsigned int size)
	{
		m_Num = size;
		m_DataNodes = m_Memory.Create(m_Num);
		DebugAssert(m_DataNodes);
		Cleanup();
	}
private:
	int							m_Num;
	DataNode*					m_DataNodes;
	DataNodeQueue				m_UnusedDataNodes;
	LockT						m_Lock;
	MemoryType					m_Memory;
};

template<typename DataT>
class NodeRecyclerForPoolT
{
public:
	typedef NodeRecyclerForPoolT<DataT> Recycler;
	typedef CChainNode<DataT,Recycler> DataNode;
	typedef CChainNodePoolT<DataT,Recycler> DataNodePool;
	friend class CChainNode<DataT,Recycler>;
	friend class CChainNodePoolT<DataT,Recycler>;
public:
	void Recycle(void)
	{
		if(NULL==m_pNode || NULL==m_pPool)
			return;
		m_pPool->Recycle(m_pNode);
	}
	int IsAllocated(void)const
	{
		return m_Allocated;
	}
private:
	void UpdateAllocated(int allocated){m_Allocated=allocated;}
private:
	NodeRecyclerForPoolT(void):m_pPool(NULL),m_pNode(NULL),m_Allocated(FALSE)
	{}
private:
	DataNodePool* m_pPool;
	DataNode* m_pNode;
	int m_Allocated;
};

template<typename DataT>
class NodeRecycler2ForPoolT
{
public:
	typedef NodeRecycler2ForPoolT<DataT> Recycler;
	typedef CChainNode<DataT,Recycler> DataNode;
	typedef CChainNodePoolT<DataT,Recycler> DataNodePool;
	friend class CChainNode<DataT,Recycler>;
	friend class CChainNodePoolT<DataT,Recycler>;
public:
	void Recycle(void)
	{
		if(NULL==m_pNode || NULL==m_pPool)
			return;
		m_pNode->GetData().Recycle();
		m_pPool->Recycle(m_pNode);
	}
	int IsAllocated(void)const
	{
		return m_Allocated;
	}
private:
	void UpdateAllocated(int allocated){m_Allocated=allocated;}
private:
	NodeRecycler2ForPoolT(void):m_pPool(NULL),m_pNode(NULL),m_Allocated(FALSE)
	{}
private:
	DataNodePool* m_pPool;
	DataNode* m_pNode;
	int m_Allocated;
};

#include "ChainNodePool.inl"

#endif //__ChainNodePool_H__
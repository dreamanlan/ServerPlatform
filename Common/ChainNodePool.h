#ifndef __ChainNodePool_H__
#define __ChainNodePool_H__

#include "Chain.h"
/**
* Link point pool, the template parameter RecyclerT of the link point class using this node pool needs to add an additional constraint:
* Provide public method void SetPool(CChainNodePoolT<DataT,RecyclerT>* pPool).
*/
template<typename DataT, typename RecyclerT, typename LockT = DummyLock, int SizeV = 0>
class CChainNodePoolT
{
public:
    using DataNode = CChainNode<DataT, RecyclerT>;
    using Iterator = typename DataNode::Iterator;
    using DataNodeQueue = DequeT<DataNode*, SizeV>;
    using MemoryType = typename CollectionMemory::SelectorT<DataNode, SizeV>::Type;
public:
    void						Init(int num);
    void						Cleanup();
public:
    const Iterator				NewNode();
    const Iterator				GetNode(int index) const;
    int							GetMaxNum()const { return m_Num; }
    int							GetUnusedNum()const { return m_UnusedDataNodes.Size(); }
public:
    virtual void				Recycle(DataNode* pNode);
public:
    CChainNodePoolT() :m_Num(0), m_DataNodes(NULL)
    {
        if constexpr (SizeV > 0) {
            Init(SizeV);
        }
    }
    CChainNodePoolT(int num) :m_Num(0), m_DataNodes(NULL)
    {
        Init(num);
    }
    virtual ~CChainNodePoolT()
    {
        Cleanup();
    }
public://Memory pool copy construction and replication do not copy data
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
        MyAssert(m_DataNodes);
        Cleanup();
    }
private:
    int							m_Num;
    DataNode* m_DataNodes;
    DataNodeQueue				m_UnusedDataNodes;
    LockT						m_Lock;
    MemoryType					m_Memory;
};

template<typename DataT>
class NodeRecyclerForPoolT
{
public:
    using Recycler = NodeRecyclerForPoolT<DataT>;
    using DataNode = CChainNode<DataT, Recycler>;
    using DataNodePool = CChainNodePoolT<DataT, Recycler>;
    friend class CChainNode<DataT, Recycler>;
    friend class CChainNodePoolT<DataT, Recycler>;
public:
    void Recycle()
    {
        if (NULL == m_pNode || NULL == m_pPool)
            return;
        m_pPool->Recycle(m_pNode);
    }
    int IsAllocated()const
    {
        return m_Allocated;
    }
private:
    void UpdateAllocated(int allocated) { m_Allocated = allocated; }
private:
    NodeRecyclerForPoolT() :m_pPool(NULL), m_pNode(NULL), m_Allocated(FALSE)
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
    using Recycler = NodeRecycler2ForPoolT<DataT>;
    using DataNode = CChainNode<DataT, Recycler>;
    using DataNodePool = CChainNodePoolT<DataT, Recycler>;
    friend class CChainNode<DataT, Recycler>;
    friend class CChainNodePoolT<DataT, Recycler>;
public:
    void Recycle()
    {
        if (NULL == m_pNode || NULL == m_pPool)
            return;
        m_pNode->GetData().Recycle();
        m_pPool->Recycle(m_pNode);
    }
    int IsAllocated()const
    {
        return m_Allocated;
    }
private:
    void UpdateAllocated(int allocated) { m_Allocated = allocated; }
private:
    NodeRecycler2ForPoolT() :m_pPool(NULL), m_pNode(NULL), m_Allocated(FALSE)
    {}
private:
    DataNodePool* m_pPool;
    DataNode* m_pNode;
    int m_Allocated;
};

#include "ChainNodePool.inl"

#endif //__ChainNodePool_H__
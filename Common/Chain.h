#ifndef		__CHAIN_H__
#define		__CHAIN_H__

#include "BaseType.h"
#include "Queue.h"

/** It is agreed that CChainNode<DataT, RecyclerT> is allocated by the data pool, and the Recycle method is used to return the node to the data pool.
* In other words, after the Recycle method is called, the node should not be on any linked list, and the data pool can then store the node
* Assigned as a new node.
* @Remarks
*RecyclerT needs to comply with:
* 1. There is no parameter construction or default construction.
* 2. Provide the public method void Recycle(CChainNode<DataT,RecyclerT>* pNode).
*/
template<typename DataT, typename RecyclerT>
class CChain;

template<typename DataT, typename RecyclerT>
class CChainNode
{
public:
    using ChainType = CChain<DataT, RecyclerT>;
    using ItemType = CChainNode<DataT, RecyclerT>;
    friend class CChain<DataT, RecyclerT>;
    friend class Iterator;
    /** Something similar to iterators in STL, but conceptually not strictly the same.
    */
    class Iterator
    {
    public:
        inline ChainType* GetChain() const
        {
            if (NULL == m_pItem)
                return NULL;
            return m_pItem->m_pChain;
        }
        inline const Iterator			GetNext() const
        {
            if (NULL == m_pItem)
                return Iterator();
            return Iterator(m_pItem->m_pNext);
        }
        inline const Iterator			GetPrevious() const
        {
            if (NULL == m_pItem)
                return Iterator();
            return Iterator(m_pItem->m_pPrevious);
        }
        inline int						IsNull() const
        {
            return m_pItem == NULL;
        }
    public:
        inline ItemType* GetPtr() const { return m_pItem; }
        inline void Recycle()
        {
            if (NULL == m_pItem)
                return;
            m_pItem->Recycle();
        }
        inline int						IsAllocated() const
        {
            if (NULL == m_pItem)
                return FALSE;
            return m_pItem->IsAllocated();
        }
    public:
        inline DataT& operator * () const { MyAssert(m_pItem); return m_pItem->m_Data; }
        inline DataT* operator -> () const { MyAssert(m_pItem); return &(m_pItem->m_Data); }
    public:
        inline Iterator& operator++()
        {
            MyAssert(m_pItem);
            m_pItem = m_pItem->m_pNext;
            return *this;
        }
        inline const Iterator operator++(int)
        {
            MyAssert(m_pItem);
            Iterator tmp = *this;
            ++(*this);
            return tmp;
        }
        inline Iterator& operator--()
        {
            MyAssert(m_pItem);
            m_pItem = m_pItem->m_pPrevious;
            return *this;
        }
        inline const Iterator operator--(int)
        {
            MyAssert(m_pItem);
            Iterator tmp = *this;
            --(*this);
            return tmp;
        }
        inline int operator == (const Iterator& other) const
        {
            return m_pItem == other.m_pItem;
        }
        inline int operator != (const Iterator& other) const
        {
            return !(operator ==(other));
        }
    public:
        Iterator()
        {
            m_pItem = NULL;
        }
        explicit Iterator(ItemType* ptr)
        {
            m_pItem = ptr;
        }
        Iterator(const Iterator& other)
        {
            m_pItem = other.m_pItem;
        }
        inline Iterator& operator = (const Iterator& other)
        {
            if (this == &other)
                return *this;
            m_pItem = other.m_pItem;
            return *this;
        }
        inline Iterator& operator = (ItemType* pItem)
        {
            m_pItem = pItem;
            return *this;
        }
    private:
        ItemType* m_pItem;
    };
protected:
    ChainType* m_pChain;
    ItemType* m_pNext;
    ItemType* m_pPrevious;
protected:
    DataT						m_Data;
    RecyclerT					m_Recycler;
public:
    CChainNode()
    {
        m_pChain = NULL;
        m_pNext = NULL;
        m_pPrevious = NULL;
        m_Recycler.m_pNode = this;
    }
    virtual ~CChainNode() {}
public:
    DataT& GetData() { return m_Data; }
    const DataT& GetData() const { return m_Data; }
public:
    inline void Recycle();
    inline int IsAllocated()const;
public:
    RecyclerT& GetRecycler() { return m_Recycler; }
    const RecyclerT& GetRecycler()const { return m_Recycler; }
private:
    //Detach the current node from the linked list and keep the linked list continuously linked (the front and rear nodes become adjacent),
    //and the number of nodes on the linked list is reduced by one
    inline void DisConnect();
};

template<typename DataT, typename RecyclerT>
class CChain
{
public:
    using ChainType = CChain<DataT, RecyclerT>;
    using ItemType = CChainNode<DataT, RecyclerT>;
    using Iterator = typename ItemType::Iterator;
    friend class CChainNode<DataT, RecyclerT>;
private:
    unsigned int				m_uNum;
    ItemType* m_pHead;
    ItemType* m_pTail;
public:
    CChain()
    {
        m_uNum = 0;
        m_pHead = NULL;
        m_pTail = NULL;
    }
    virtual ~CChain()
    {
        RemoveAll();
    }
    //Get the first node of the linked list
    inline const Iterator	First() const
    {
        return Iterator(m_pHead);
    }
    //Get the last node of the linked list
    inline const Iterator	Last() const
    {
        return Iterator(m_pTail);
    }
    //Get the total number of nodes on the linked list
    inline unsigned int				GetNum() const
    {
        return m_uNum;
    }
    //Get the i-th node (counting from 0)
    inline Iterator			GetIterator(int i) const;
    //Find the specified node on the linked list and return the node's position in the linked list
    inline int				IndexOf(const Iterator& it) const;
    //Add a node to the head of the linked list, this node will become the new head of the linked list
    inline int				AddFirst(const Iterator& it);
    //Add a node to the end of the linked list, this node will become the new end of the linked list
    inline int				AddLast(const Iterator& it);
    //Insert a node before the specified node. The specified node cannot be empty.
    inline int				InsertBefore(const Iterator& pos, const Iterator& it);
    //Insert a node after the specified node. The specified node cannot be empty.
    inline int				InsertAfter(const Iterator& pos, const Iterator& it);
    //Delete the first node, return the node to the data pool, and return the first node of the linked list after deletion
    inline const Iterator	RemoveFirst();
    //Delete the last node, return the node to the data pool, and return the last node of the linked list after deletion
    inline const Iterator	RemoveLast();
    //Delete a node from the linked list, return the node to the data pool, and return the node behind the deleted node.
    inline const Iterator	Remove(const Iterator& it);
    //Delete all nodes from the linked list and return all nodes to the data pool
    inline int				RemoveAll();
    //Move the specified node to the first node of the chain
    inline const Iterator	MoveAsFirst(const Iterator& it);
    //Move the specified node to the tail node of the chain
    inline const Iterator	MoveAsLast(const Iterator& it);
};

template<typename DataT>
class NodeRecyclerForDataT
{
public:
    using Recycler = NodeRecyclerForDataT<DataT>;
    using DataNode = CChainNode<DataT, Recycler>;
    friend class CChainNode<DataT, Recycler>;
public:
    void Recycle()
    {
        if (NULL == m_pNode)
            return;
        m_pNode->GetData().Recycle();
    }
    int IsAllocated() const
    {
        return m_pNode->GetData().IsAllocated();
    }
private:
    NodeRecyclerForDataT() :m_pNode(NULL)
    {}
private:
    DataNode* m_pNode;
};

#include "Chain.inl"

#endif
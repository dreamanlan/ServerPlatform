#ifndef		__OBJECTPOOL_H__
#define		__OBJECTPOOL_H__

#include "Type.h"
#include "Queue.h"

/**
*	Data object pool. DataT using this data pool needs to inherit from PoolAllocatedObjectBaseT<DataT>
*/
template<typename DataT>
class PoolAllocatedObjectBaseT;

template<typename DataT>
class ObjectPoolBaseT
{
    friend class PoolAllocatedObjectBaseT<DataT>;
public:
    virtual ~ObjectPoolBaseT() {}
protected:
    virtual void Recycle(DataT* pData) = 0;
protected:
    void ConnectData(DataT* pData);
    void UpdateDataAllocated(DataT* pData, int allocated);
};

template<typename DataT>
class PoolAllocatedObjectBaseT
{
    friend class ObjectPoolBaseT<DataT>;
    using ObjectPoolBase = ObjectPoolBaseT<DataT>;
public:
    PoolAllocatedObjectBaseT() :m_ObjectPool(NULL), m_Allocated(FALSE)
    {}
    virtual ~PoolAllocatedObjectBaseT() {}
public:
    void					Recycle();
    int					IsAllocated()const { return m_Allocated; }
protected:
    virtual void			OnRecycle() {}
private:
    ObjectPoolBase* m_ObjectPool;
    int					m_Allocated;
};

template<typename DataT, typename LockT = DummyLock, int SizeV = 0>
class ObjectPoolT : public ObjectPoolBaseT<DataT>
{
public:
    using DataQueue = DequeT<DataT*, SizeV>;
    using MemoryType = typename CollectionMemory::SelectorT<DataT, SizeV>::Type;
public:
    void						Init(int num);
    void						Cleanup();
public:
    DataT* NewObject();
    DataT* GetObject(int index) const;
    int						GetMaxNum()const { return m_Num; }
    int						GetUnusedNum()const { return m_UnusedDatas.Size(); }
    size_t						GetMemoryInUsed()const { return m_Memory.GetMemoryInUsed(); }
public:
    ObjectPoolT() :m_Num(0), m_Datas(NULL)
    {
        if (SizeV > 0) {
            Init(SizeV);
        }
    }
    ObjectPoolT(int num) :m_Num(0), m_Datas(NULL)
    {
        Init(num);
    }
    virtual ~ObjectPoolT()
    {
        Cleanup();
    }
public://Memory pool copy construction and replication do not copy data
    ObjectPoolT(const ObjectPoolT& other)
    {
        Init(other.GetMaxNum());
    }
    ObjectPoolT& operator=(const ObjectPoolT& other)
    {
        Cleanup();
        Init(other.GetMaxNum());
        return *this;
    }
protected:
    virtual void				Recycle(DataT* pData);
protected:
    inline void Create(unsigned int size)
    {
        m_Num = size;
        m_Datas = m_Memory.Create(m_Num);
        MyAssert(m_Datas);
        Cleanup();
    }
private:
    int							m_Num;
    DataT* m_Datas;
    DataQueue					m_UnusedDatas;
    LockT						m_Lock;
    MemoryType					m_Memory;
};

#include "ObjectPool.inl"

#endif
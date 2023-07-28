#ifndef		__OBJECTPOOL_H__
#define		__OBJECTPOOL_H__

#include "Type.h"
#include "Queue.h"

/**
*	���ݶ���أ�ʹ�ô����ݳص�DataT��Ҫ�̳���PoolAllocatedObjectBaseT<DataT>
*/
template<typename DataT>
class PoolAllocatedObjectBaseT;

template<typename DataT>
class ObjectPoolBaseT
{
	friend class PoolAllocatedObjectBaseT<DataT>;
public:
	virtual ~ObjectPoolBaseT(void){}
protected:
	virtual void Recycle(DataT* pData) = 0;
protected:
	void ConnectData(DataT* pData);
	void UpdateDataAllocated(DataT* pData,int allocated);
};

template<typename DataT>
class PoolAllocatedObjectBaseT
{
	friend class ObjectPoolBaseT<DataT>;
	using ObjectPoolBase = ObjectPoolBaseT<DataT>;
public:
	PoolAllocatedObjectBaseT(void):m_ObjectPool(NULL),m_Allocated(FALSE)
	{}
	virtual ~PoolAllocatedObjectBaseT(void){}
public:
	void					Recycle(void);
	int					IsAllocated(void)const{return m_Allocated;}
protected:
	virtual void			OnRecycle(void){}
private:
	ObjectPoolBase*			m_ObjectPool;
	int					m_Allocated;
};

template<typename DataT,typename LockT = DummyLock,int SizeV = 0>
class ObjectPoolT : public ObjectPoolBaseT<DataT>
{
public:
	using DataQueue = DequeT<DataT*,SizeV>;
	using MemoryType = typename CollectionMemory::SelectorT<DataT,SizeV>::Type;
public:
	void						Init(int num);
	void						Cleanup(void);
public:
	DataT*						NewObject(void);
	DataT*						GetObject(int index) const;
	int						GetMaxNum(void)const{return m_Num;}
	int						GetUnusedNum(void)const{return m_UnusedDatas.Size();}
	size_t						GetMemoryInUsed(void)const{return m_Memory.GetMemoryInUsed(); }
public:
	ObjectPoolT(void):m_Num(0),m_Datas(NULL)
	{
		if(SizeV>0)
		{
			Init(SizeV);
		}
	}
	ObjectPoolT(int num):m_Num(0),m_Datas(NULL)
	{
		Init(num);
	}
	virtual ~ObjectPoolT(void)
	{
		Cleanup();
	}
public://�ڴ�صĿ��������븴�Ʋ���������
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
	DataT*						m_Datas;
	DataQueue					m_UnusedDatas;
	LockT						m_Lock;
	MemoryType					m_Memory;
};

#include "ObjectPool.inl"

#endif
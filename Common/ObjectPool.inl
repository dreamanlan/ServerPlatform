#ifndef __OBJECTPOOL_Inl__
#define __OBJECTPOOL_Inl__

template<typename DataT>
void ObjectPoolBaseT<DataT>::ConnectData(DataT* pData)
{
	if(NULL!=pData)
		pData->m_ObjectPool = this;
}

template<typename DataT>
void ObjectPoolBaseT<DataT>::UpdateDataAllocated(DataT* pData,int allocated)
{
	if(NULL!=pData)
		pData->m_Allocated = allocated;
}

template<typename DataT>
void PoolAllocatedObjectBaseT<DataT>::Recycle(void)
{
	if(FALSE==IsAllocated())
		return;
	OnRecycle();
	if(NULL!=m_ObjectPool)
		m_ObjectPool->Recycle(static_cast<DataT*>(this));	
}

template<typename DataT,typename LockT,int SizeV>
void ObjectPoolT<DataT,LockT,SizeV>::Recycle(DataT* pData)
{
	AutoLockT<LockT> lock(m_Lock);
	if(TRUE==m_UnusedDatas.Full())
		return;
	m_UnusedDatas.PushBack(pData);
	UpdateDataAllocated(pData,FALSE);
}

template<typename DataT,typename LockT,int SizeV>
void ObjectPoolT<DataT,LockT,SizeV>::Init(int num)
{
	Create(num);
	DebugAssert(m_Datas);
	m_UnusedDatas.Init(num);
	for(int i=0;i<num;++i)
	{
		ConnectData(&(m_Datas[i]));
		UpdateDataAllocated(&(m_Datas[i]),FALSE);
		m_UnusedDatas.PushBack(&(m_Datas[i]));
	}
	m_Num = num;
}

template<typename DataT,typename LockT,int SizeV>
void ObjectPoolT<DataT,LockT,SizeV>::Cleanup(void)
{
	m_UnusedDatas.Clear();
}

template<typename DataT,typename LockT,int SizeV>
DataT* ObjectPoolT<DataT,LockT,SizeV>::NewObject(void)
{
	AutoLockT<LockT> lock(m_Lock);
	if(TRUE==m_UnusedDatas.Empty())
		return NULL;
	DataT* pData = m_UnusedDatas.PopFront();
	UpdateDataAllocated(pData,TRUE);
//	pData->CleanData();	JT:由于共享内存直接初始数据，所以在这里不清理，有清除数据的需要放在外部做
	return pData;
}

template<typename DataT,typename LockT,int SizeV>
DataT* ObjectPoolT<DataT,LockT,SizeV>::GetObject(int index) const
{	
	if(index<0 || index>=m_Num)
		return NULL;
	DebugAssert(m_Datas);
	return &(m_Datas[index]);
}

#endif //__OBJECTPOOL_Inl__
#ifndef __ChainNodePool_Inl__
#define __ChainNodePool_Inl__

template<typename DataT,typename RecyclerT,typename LockT,int SizeV>
void CChainNodePoolT<DataT,RecyclerT,LockT,SizeV>::Recycle(DataNode* pNode)
{
	if(NULL==pNode)
		return;
	AutoLockT<LockT> lock(m_Lock);
	if(TRUE==m_UnusedDataNodes.Full())
		return;
	m_UnusedDataNodes.PushBack(pNode);
	pNode->GetRecycler().UpdateAllocated(FALSE);
}

template<typename DataT,typename RecyclerT,typename LockT,int SizeV>
void CChainNodePoolT<DataT,RecyclerT,LockT,SizeV>::Init(int num)
{
	Create(num);
	DebugAssert(m_DataNodes);
	m_UnusedDataNodes.Init(num);
	for(int i=0;i<num;++i)
	{
		m_DataNodes[i].GetRecycler().m_pPool = this;
		m_DataNodes[i].GetRecycler().UpdateAllocated(FALSE);
		m_UnusedDataNodes.PushBack(&(m_DataNodes[i]));
	}
	m_Num = num;
}

template<typename DataT,typename RecyclerT,typename LockT,int SizeV>
void CChainNodePoolT<DataT,RecyclerT,LockT,SizeV>::Cleanup(void)
{
	m_UnusedDataNodes.Clear();
}

template<typename DataT,typename RecyclerT,typename LockT,int SizeV>
const typename CChainNodePoolT<DataT,RecyclerT,LockT,SizeV>::Iterator CChainNodePoolT<DataT,RecyclerT,LockT,SizeV>::NewNode(void)
{
	AutoLockT<LockT> lock(m_Lock);
	if(TRUE==m_UnusedDataNodes.Empty())
		return Iterator();
	DataNode* pNode = m_UnusedDataNodes.PopFront();
	if(NULL==pNode)
		return Iterator();
	pNode->GetRecycler().UpdateAllocated(TRUE);
	return Iterator(pNode);
}

template<typename DataT,typename RecyclerT,typename LockT,int SizeV>
const typename CChainNodePoolT<DataT,RecyclerT,LockT,SizeV>::Iterator CChainNodePoolT<DataT,RecyclerT,LockT,SizeV>::GetNode(int index) const
{	
	if(index<0 || index>=m_Num)
		return Iterator();
	DebugAssert(m_DataNodes);
	return Iterator(&(m_DataNodes[index]));
}

#endif //__ChainNodePool_Inl__
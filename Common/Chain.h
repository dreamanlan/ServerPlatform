#ifndef		__CHAIN_H__
#define		__CHAIN_H__

#include "BaseType.h"
#include "Queue.h"

/**	Լ��CChainNode<DataT,RecyclerT>�����ݳط���,Recycle�������ڷ����������ݳأ�
*	Ҳ����˵Recycle��������֮�󣬽��Ӧ�ò����κ������ϣ����ݳ������Խ��ý��
*	��Ϊ�½����䡣
*	@Remarks
*	RecyclerT��Ҫ���ϣ�
*	1�����޲��������Ĭ�Ϲ��졣
*	2���ṩ���з���void Recycle(CChainNode<DataT,RecyclerT>* pNode)��
*/
template<typename DataT,typename RecyclerT>
class CChain;

template<typename DataT,typename RecyclerT>
class CChainNode
{
public:
	using ChainType = CChain<DataT,RecyclerT>;
	using ItemType = CChainNode<DataT,RecyclerT>;
	friend class CChain<DataT,RecyclerT>;
	friend class Iterator;
	/**	һ������STL�е������Ķ��������Ǹ����ϲ��ϸ�һ����
	*/
	class Iterator
	{
	public:
		inline ChainType*				GetChain(void) const
		{
			if(NULL==m_pItem)
				return NULL;
			return m_pItem->m_pChain;
		}
		inline const Iterator			GetNext(void) const
		{
			if(NULL==m_pItem)
				return Iterator();
			return Iterator(m_pItem->m_pNext);
		}
		inline const Iterator			GetPrevious(void) const
		{
			if(NULL==m_pItem)
				return Iterator();
			return Iterator(m_pItem->m_pPrevious);
		}
		inline int						IsNull(void) const
		{
			return m_pItem==NULL;
		}
	public:
		inline ItemType* GetPtr(void) const {return m_pItem;}
		inline void Recycle(void)
		{
			if(NULL==m_pItem)
				return;
			m_pItem->Recycle();
		}
		inline int						IsAllocated(void) const
		{
			if(NULL==m_pItem)
				return FALSE;
			return m_pItem->IsAllocated();
		}
	public:
		inline DataT& operator * (void) const {MyAssert(m_pItem);return m_pItem->m_Data;}
		inline DataT* operator -> (void) const {MyAssert(m_pItem);return &(m_pItem->m_Data);}	
	public:
		inline Iterator& operator++(void)
		{
			MyAssert(m_pItem);
			m_pItem=m_pItem->m_pNext;
			return *this;
		}
		inline const Iterator operator++(int)
		{
			MyAssert(m_pItem);
			Iterator tmp = *this;
			++(*this);
			return tmp;
		}
		inline Iterator& operator--(void)
		{
			MyAssert(m_pItem);
			m_pItem=m_pItem->m_pPrevious;
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
			return m_pItem==other.m_pItem;
		}
		inline int operator != (const Iterator& other) const
		{
			return !(operator ==(other));
		}
	public:
		Iterator(void)
		{
			m_pItem=NULL;
		}
		explicit Iterator(ItemType* ptr)
		{
			m_pItem=ptr;
		}
		Iterator(const Iterator& other)
		{
			m_pItem=other.m_pItem;
		}
		inline Iterator& operator = (const Iterator& other)
		{
			if(this==&other)
				return *this;
			m_pItem=other.m_pItem;
			return *this;
		}
		inline Iterator& operator = (ItemType* pItem)
		{
			m_pItem=pItem;
			return *this;
		}
	private:
		ItemType* m_pItem;
	};
protected:
	ChainType*					m_pChain;
	ItemType*					m_pNext;
	ItemType*					m_pPrevious;
protected:
	DataT						m_Data;
	RecyclerT					m_Recycler;
public:
	CChainNode(void)
	{
		m_pChain    = NULL;
		m_pNext     = NULL;
		m_pPrevious = NULL;
		m_Recycler.m_pNode = this;
	}
	virtual ~CChainNode(void){}
public:
	DataT& GetData(void){return m_Data;}
	const DataT& GetData(void) const {return m_Data;}
public:
	inline void Recycle(void);
	inline int IsAllocated(void)const;
public:
	RecyclerT& GetRecycler(void){return m_Recycler;}
	const RecyclerT& GetRecycler(void)const{return m_Recycler;}
private:
	//ʹ��ǰ�������������벢��������������ǰ�����Ϊ�ڽӣ��������ϵĽ��������һ��
	inline void DisConnect(void);
};

template<typename DataT,typename RecyclerT>
class CChain
{
public:
	using ChainType = CChain<DataT,RecyclerT>;
	using ItemType = CChainNode<DataT,RecyclerT>;
	using Iterator = typename ItemType::Iterator;
	friend class CChainNode<DataT,RecyclerT>;
private:
	unsigned int				m_uNum;
	ItemType*			m_pHead;
	ItemType*			m_pTail;
public:
	CChain(void)
	{
		m_uNum		= 0;
		m_pHead		= NULL;
		m_pTail     = NULL;
	}
	virtual ~CChain(void)
	{
		RemoveAll();
	}
	//��ȡ�����һ�����
	inline const Iterator	First(void) const
	{
		return Iterator(m_pHead);
	}
	//��ȡ�������һ�����
	inline const Iterator	Last(void) const
	{
		return Iterator(m_pTail);
	}
	//��ȡ�����Ͻ������
	inline unsigned int				GetNum(void) const
	{
		return m_uNum;
	}
	//��ȡ��i�����(��0��ʼ����)
	inline Iterator			GetIterator(int i) const;
	//�������ϲ���ָ����㣬���ؽ���������е�λ��
	inline int				IndexOf(const Iterator& it) const;
	//������ͷ���һ����㣬�˽�㽫��Ϊ�µ�����ͷ
	inline int				AddFirst(const Iterator& it);
	//������β���һ����㣬�˽�㽫��Ϊ�µ�����β
	inline int				AddLast(const Iterator& it);
	//��ָ�����ǰ����һ�����,ָ����㲻��Ϊ��
	inline int				InsertBefore(const Iterator& pos,const Iterator& it);
	//��ָ���������һ����㣬ָ����㲻��Ϊ��
	inline int				InsertAfter(const Iterator& pos,const Iterator& it);
	//ɾ����һ����㣬��㷵�������ݳأ�����ɾ��������ĵ�һ�����
	inline const Iterator	RemoveFirst(void);
	//ɾ�����һ����㣬��㷵�������ݳأ�����ɾ������������һ�����
	inline const Iterator	RemoveLast(void);
	//��������ɾ��һ����㣬��㷵�������ݳأ����ر�ɾ��������Ľ��
	inline const Iterator	Remove(const Iterator& it);
	//��������ɾ�����н�㣬ȫ����㷵�������ݳ�
	inline int				RemoveAll(void);
	//�ƶ�ָ�����Ϊ�����׽��
	inline const Iterator	MoveAsFirst(const Iterator& it);
	//�ƶ�ָ�����Ϊ����β���
	inline const Iterator	MoveAsLast(const Iterator& it);
};

template<typename DataT>
class NodeRecyclerForDataT
{
public:
	using Recycler = NodeRecyclerForDataT<DataT>;
	using DataNode = CChainNode<DataT,Recycler>;
	friend class CChainNode<DataT,Recycler>;
public:
	void Recycle(void)
	{
		if(NULL==m_pNode)
			return;
		m_pNode->GetData().Recycle();
	}
	int IsAllocated(void) const
	{
		return m_pNode->GetData().IsAllocated();
	}
private:
	NodeRecyclerForDataT(void):m_pNode(NULL)
	{}
private:
	DataNode* m_pNode;
};

#include "Chain.inl"

#endif
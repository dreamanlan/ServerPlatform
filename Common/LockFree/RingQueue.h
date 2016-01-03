#ifndef RINGEDQUEUE_H__
#define RINGEDQUEUE_H__

#include <stddef.h>
#include "Atomic.h"
#pragma warning( push )
#pragma warning( disable : 4311 4312 )

namespace LockFreeCollectionUtility
{
	template<class T,bool IsOK>
	struct TypeCheckerT
	{
		typedef T Type;
		static inline Type ToDataType(unsigned long val)
		{
			return static_cast<Type>(val);
		}
		static inline unsigned long& ToSimpleType(Type& val)
		{
			return static_cast<unsigned long&>(val);
		}
		static inline unsigned long ToSimpleType(const Type& val)
		{
			return static_cast<unsigned long>(val);
		}
	};
	template<class T>
	struct TypeCheckerT<T,false>
	{
		inline T ERROR_Ringed_Queue_Only_Support_Pointer_Type_Or_Not_Zero_Simple_Type_Please_Check_DataT_template_Arg(void)const;
		enum{v=sizeof(static_cast<long>(ERROR_Ringed_Queue_Only_Support_Pointer_Type_Or_Not_Zero_Simple_Type_Please_Check_DataT_template_Arg()))};
	};
	template<class T>
	struct TypeCheckerT<T*,true>
	{
		typedef T* Type;
		static inline Type ToDataType(unsigned long val)
		{
			return reinterpret_cast<Type>(val);
		}
		static inline unsigned long& ToSimpleType(Type& val)
		{
			return *reinterpret_cast<unsigned long*>(&val);
		}
		static inline unsigned long ToSimpleType(const Type& val)
		{
			return reinterpret_cast<unsigned long>(val);
		}
	};
	template<>
	struct TypeCheckerT<int,true>
	{
		typedef int Type;
		static inline int ToDataType(unsigned int val)
		{
			return static_cast<int>(val);
		}
		static inline unsigned int& ToSimpleType(int& val)
		{
			return *reinterpret_cast<unsigned int*>(&val);
		}
		static inline unsigned int ToSimpleType(const int& val)
		{
			return static_cast<unsigned int>(val);
		}
	};
	template<>
	struct TypeCheckerT<short,true>
	{
		typedef short Type;
		static inline short ToDataType(unsigned short val)
		{
			return static_cast<short>(val);
		}
		static inline unsigned short& ToSimpleType(short& val)
		{
			return *reinterpret_cast<unsigned short*>(&val);
		}
		static inline unsigned short ToSimpleType(const short& val)
		{
			return static_cast<unsigned short>(val);
		}
	};
	template<>
	struct TypeCheckerT<char,true>
	{
		typedef char Type;
		static inline char ToDataType(unsigned char val)
		{
			return static_cast<char>(val);
		}
		static inline unsigned char& ToSimpleType(char& val)
		{
			return *reinterpret_cast<unsigned char*>(&val);
		}
		static inline unsigned char ToSimpleType(const char& val)
		{
			return static_cast<unsigned char>(val);
		}
	};
	template<class T>
	struct TypeCheckerWrapT
	{
		typedef TypeCheckerT<T,sizeof(T)<=sizeof(long*)> TypeCheckerType;
		typedef typename TypeCheckerType::Type Type;
	};
	template<typename T,int CapacityV,unsigned long InvalidValueV>
	class StaticMemoryT
	{
		typedef typename LockFreeCollectionUtility::TypeCheckerWrapT<T>::TypeCheckerType TypeCheckerType;
		typedef typename LockFreeCollectionUtility::TypeCheckerWrapT<T>::Type ElementType;
	public:
		inline ElementType* Create(int& capacity)
		{
			if(capacity>CapacityV)
			{
				capacity = 1;
				return NULL;
			}
			return m_Data;
		}
		inline void Clear(void)
		{
			for(int i=0;i<CapacityV;++i)
			{
				m_Data[i]=TypeCheckerType::ToDataType(InvalidValueV);
			}
		}
		size_t GetMemoryInUsed(void) const
		{
			return sizeof(T) * CapacityV;
		}
	private:
		T	m_Data[CapacityV];
	};

	template<typename T,unsigned long InvalidValueV>
	class DynamicMemoryT
	{
		typedef typename LockFreeCollectionUtility::TypeCheckerWrapT<T>::TypeCheckerType TypeCheckerType;
		typedef typename LockFreeCollectionUtility::TypeCheckerWrapT<T>::Type ElementType;
	public:
		inline ElementType* Create(int& capacity)
		{
			Cleanup();
			m_Data = new ElementType[capacity];
			if(NULL == m_Data)
			{
				capacity = 1;
				return NULL;
			}
			m_Capacity = capacity;
			return m_Data;
		}
		inline void Clear(void)
		{
			if(m_Capacity>0 && NULL!=m_Data)
			{
				for(int i=0;i<m_Capacity;++i)
				{
					m_Data[i]=TypeCheckerType::ToDataType(InvalidValueV);
				}
			}
		}
		size_t GetMemoryInUsed(void) const
		{
			return (m_Data == NULL ? 0 : (sizeof(T) * m_Capacity));	
		}
	public:
		DynamicMemoryT(void):m_Data(NULL),m_Capacity(0)
		{}
		virtual ~DynamicMemoryT(void)
		{
			Cleanup();
		}
	private:
		inline void Cleanup(void)
		{
			if(NULL!=m_Data)
			{
				delete[] m_Data;
				m_Data=0;
			}
			m_Capacity = 0;
		}
	private:
		ElementType*	m_Data;
		int				m_Capacity;
	};
	template<typename T,int CapacityV,int InvalidValueV>
	class MemorySelectorT
	{
	public:
		typedef StaticMemoryT<T,CapacityV,InvalidValueV> Type;
	};
	template<typename T,int InvalidValueV>
	class MemorySelectorT<T,0,InvalidValueV>
	{
	public:
		typedef DynamicMemoryT<T,InvalidValueV> Type;
	};
}

/**
*	管理指针的lock-free队列，通过头指针、尾指针与数值空来保证数据正确性，对这三类数据的操作是原子的。
*	@remark
*	只用于管理指针与含无效值简单数据类型（Size小于机器字长，并且InvalidValueV为无效值，特别注意InvalidValueV为无效值）！！！
*/
template<class DataT,int SizeV=0,int InvalidValueV=0>
class LockFreeRingedQueueT
{
	static const int s_c_MaxRetryCount=256;

	typedef LockFreeRingedQueueT<DataT,SizeV,InvalidValueV>	ThisType;
	typedef typename LockFreeCollectionUtility::TypeCheckerWrapT<DataT>::TypeCheckerType TypeCheckerType;
	typedef typename LockFreeCollectionUtility::TypeCheckerWrapT<DataT>::Type ElementType;
	typedef typename LockFreeCollectionUtility::MemorySelectorT<DataT,(SizeV == 0 ? 0 : SizeV+1),InvalidValueV>::Type MemoryType;

#ifdef __WINDOWS__
	//32位下按8字节对齐
	__declspec(align(8))
#endif
	struct Pointer
	{
		volatile int			m_Pos;
		volatile unsigned int	m_Tag;

		inline Pointer(void)
		{
			Clear();
		}
		inline void Clear(void) volatile
		{
			m_Pos=0;
			m_Tag=0;
		}
	}
#ifndef __WINDOWS__
	//64位下按16字节对齐
	__attribute__ ((aligned (16)))
#endif
	;
	static inline bool IsInvalid(ElementType val)
	{
		return InvalidValueV==TypeCheckerType::ToSimpleType(val);
	}
	static inline ElementType GetInvalid(void)
	{
		return TypeCheckerType::ToDataType(InvalidValueV);
	}
public:
	//此方法多线程不安全
	inline void	Init(int size)
	{
		Create(size+1);
	}
	//此方法多线程不安全
	inline void	Clear(void)
	{
		m_Head.Clear();
		m_Tail.Clear();
		m_Memory.Clear();
	}
	//因为队列是lock-free的，此值仅提供参考信息，不能据此进行准确判断
	inline int	GetNum(void)const
	{
		return (m_Tail.m_Pos+m_Capacity-m_Head.m_Pos)%m_Capacity;
	}
	//此方法为lock-free的
	inline bool	Push(const ElementType& data)
	{
		bool ret=false;
		//先移动指针（相当于为本线程本次操作申请操作权）
		int oldPos=0;
		for(int i=0;i<s_c_MaxRetryCount;++i)
		{
			oldPos=m_Tail.m_Pos;
			unsigned int tag=m_Tail.m_Tag;
			int pos=(oldPos+1)%m_Capacity;
			if(pos==m_Head.m_Pos || !IsInvalid(m_pData[oldPos]))//满了或者其它线程还没完成pop，本次操作按失败处理
			{
				break;
			}
			if(lock_free_utility::compare_and_swap(reinterpret_cast<volatile unsigned int*>(&m_Tail),oldPos,tag,pos,tag+1))
			{
				ret=true;
				break;
			}
			else
			{
				lock_free_utility::pause();
			}
		}
		//然后修改数据
		if(ret)
		{
			ElementType _data=lock_free_utility::xchg(&(m_pData[oldPos]),data);
#if	defined(LockFreeDebugPrint)
			if(!IsInvalid(_data))
			{
				LockFreeDebugPrint("Data[%d]:%u!=0 <-> %u in RingedQueue::Push\n",oldPos,(unsigned int)_data,(unsigned int)data);
			}
#endif
		}
		return ret;
	}
	//此方法为lock-free的
	inline bool	Pop(ElementType& data)
	{
		bool ret=false;
		//先移动指针（相当于为本线程本次操作申请操作权）
		int oldPos=0;
		for(int i=0;i<s_c_MaxRetryCount;++i)
		{
			oldPos=m_Head.m_Pos;
			unsigned int tag=m_Head.m_Tag;
			int pos=(oldPos+1)%m_Capacity;			
			if(oldPos==m_Tail.m_Pos || IsInvalid(m_pData[oldPos]))//空了或者其它线程还没完成push，本次操作按失败处理
			{
				break;
			}
			if(lock_free_utility::compare_and_swap(reinterpret_cast<volatile unsigned int*>(&m_Head),oldPos,tag,pos,tag+1))
			{
				ret=true;
				break;
			}
			else
			{
				lock_free_utility::pause();
			}
		}
		//然后修改数据
		if(ret)
		{
			data=lock_free_utility::xchg(&(m_pData[oldPos]),GetInvalid());
#if	defined(LockFreeDebugPrint)
			if(IsInvalid(data))
			{
				LockFreeDebugPrint("Data[%d]:0==0 in RingedQueue::Pop\n",oldPos);
			}
#endif
		}
		return ret;
	}
	inline ElementType Pop(const ElementType& invalidValue=GetInvalid())
	{
		ElementType data;
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
	inline LockFreeRingedQueueT(void):m_pData(NULL),m_Capacity(0)
	{
		if(SizeV>0)
		{
			Init(SizeV);
		}
	}
	inline ~LockFreeRingedQueueT(void)
	{
		Clear();
		m_pData=NULL;
		m_Capacity=0;
	}
private:
	inline void Create(int size)
	{
		m_Capacity = size;
		m_pData = m_Memory.Create(m_Capacity);
		//CrashAssert(m_Data);
		Clear();
	}
private:
	LockFreeRingedQueueT(const LockFreeRingedQueueT& other);
	LockFreeRingedQueueT& operator=(const LockFreeRingedQueueT& other);
private:
	MemoryType				m_Memory;
	ElementType*			m_pData;
	volatile Pointer		m_Head;
	volatile Pointer		m_Tail;
	int						m_Capacity;
};

#pragma warning( pop )
#endif
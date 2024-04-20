#ifndef RINGEDQUEUE_H__
#define RINGEDQUEUE_H__

#include <stddef.h>
#include "Atomic.h"
#pragma warning( push )
#pragma warning( disable : 4311 4312 )

namespace LockFreeCollectionUtility
{
    template<class T, bool IsOK>
    struct TypeCheckerT
    {
        using Type = T;
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
    struct TypeCheckerT<T, false>
    {
        inline T ERROR_Ringed_Queue_Only_Support_Pointer_Type_Or_Not_Zero_Simple_Type_Please_Check_DataT_template_Arg()const;
        enum { v = sizeof(static_cast<long>(ERROR_Ringed_Queue_Only_Support_Pointer_Type_Or_Not_Zero_Simple_Type_Please_Check_DataT_template_Arg())) };
    };
    template<class T>
    struct TypeCheckerT<T*, true>
    {
        using Type = T*;
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
    struct TypeCheckerT<int, true>
    {
        using Type = int;
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
    struct TypeCheckerT<short, true>
    {
        using Type = short;
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
    struct TypeCheckerT<char, true>
    {
        using Type = char;
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
        using TypeCheckerType = TypeCheckerT<T, sizeof(T) <= sizeof(long*)>;
        using Type = typename TypeCheckerType::Type;
    };
    template<typename T, int CapacityV, unsigned long InvalidValueV>
    class StaticMemoryT
    {
        using TypeCheckerType = typename LockFreeCollectionUtility::TypeCheckerWrapT<T>::TypeCheckerType;
        using ElementType = typename LockFreeCollectionUtility::TypeCheckerWrapT<T>::Type;
    public:
        inline ElementType* Create(int& capacity)
        {
            if (capacity > CapacityV) {
                capacity = 1;
                return NULL;
            }
            return m_Data;
        }
        inline void Clear()
        {
            for (int i = 0; i < CapacityV; ++i) {
                m_Data[i] = TypeCheckerType::ToDataType(InvalidValueV);
            }
        }
        size_t GetMemoryInUsed() const
        {
            return sizeof(T) * CapacityV;
        }
    private:
        T	m_Data[CapacityV];
    };

    template<typename T, unsigned long InvalidValueV>
    class DynamicMemoryT
    {
        using TypeCheckerType = typename LockFreeCollectionUtility::TypeCheckerWrapT<T>::TypeCheckerType;
        using ElementType = typename LockFreeCollectionUtility::TypeCheckerWrapT<T>::Type;
    public:
        inline ElementType* Create(int& capacity)
        {
            Cleanup();
            m_Data = new ElementType[capacity];
            if (NULL == m_Data) {
                capacity = 1;
                return NULL;
            }
            m_Capacity = capacity;
            return m_Data;
        }
        inline void Clear()
        {
            if (m_Capacity > 0 && NULL != m_Data) {
                for (int i = 0; i < m_Capacity; ++i) {
                    m_Data[i] = TypeCheckerType::ToDataType(InvalidValueV);
                }
            }
        }
        size_t GetMemoryInUsed() const
        {
            return (m_Data == NULL ? 0 : (sizeof(T) * m_Capacity));
        }
    public:
        DynamicMemoryT() :m_Data(NULL), m_Capacity(0)
        {}
        virtual ~DynamicMemoryT()
        {
            Cleanup();
        }
    private:
        inline void Cleanup()
        {
            if (NULL != m_Data) {
                delete[] m_Data;
                m_Data = 0;
            }
            m_Capacity = 0;
        }
    private:
        ElementType* m_Data;
        int				m_Capacity;
    };
    template<typename T, int CapacityV, int InvalidValueV>
    class MemorySelectorT
    {
    public:
        using Type = StaticMemoryT<T, CapacityV, InvalidValueV>;
    };
    template<typename T, int InvalidValueV>
    class MemorySelectorT<T, 0, InvalidValueV>
    {
    public:
        using Type = DynamicMemoryT<T, InvalidValueV>;
    };
}

/**
*	管理指针的lock-free队列，通过头指针、尾指针与数值空来保证数据正确性，对这三类数据的操作是原子的。
*	@remark
*	只用于管理指针与含无效值简单数据类型（Size小于机器字长，并且InvalidValueV为无效值，特别注意InvalidValueV为无效值）！！！
*/
template<class DataT, int SizeV = 0, int InvalidValueV = 0>
class LockFreeRingedQueueT
{
    static const int s_c_MaxRetryCount = 256;

    using ThisType = LockFreeRingedQueueT<DataT, SizeV, InvalidValueV>;
    using TypeCheckerType = typename LockFreeCollectionUtility::TypeCheckerWrapT<DataT>::TypeCheckerType;
    using ElementType = typename LockFreeCollectionUtility::TypeCheckerWrapT<DataT>::Type;
    using MemoryType = typename LockFreeCollectionUtility::MemorySelectorT<DataT, (SizeV == 0 ? 0 : SizeV + 1), InvalidValueV>::Type;

#ifdef __WINDOWS__
    //Aligned by 8 bytes in 32-bit mode
    __declspec(align(8))
#endif
        struct Pointer
    {
        volatile int			m_Pos;
        volatile unsigned int	m_Tag;

        inline Pointer()
        {
            Clear();
        }
        inline void Clear() volatile
        {
            m_Pos = 0;
            m_Tag = 0;
        }
    }
#ifndef __WINDOWS__
    //Aligned to 16 bytes in 64-bit
    __attribute__((aligned(16)))
#endif
        ;
    static inline bool IsInvalid(ElementType val)
    {
        return InvalidValueV == TypeCheckerType::ToSimpleType(val);
    }
    static inline ElementType GetInvalid()
    {
        return TypeCheckerType::ToDataType(InvalidValueV);
    }
public:
    //This method is not multi-thread safe
    inline void	Init(int size)
    {
        Create(size + 1);
    }
    //This method is not multi-thread safe
    inline void	Clear()
    {
        m_Head.Clear();
        m_Tail.Clear();
        m_Memory.Clear();
    }
    //Because the queue is lock-free, this value only provides reference information and cannot be used to make accurate judgments.
    inline int	GetNum()const
    {
        return (m_Tail.m_Pos + m_Capacity - m_Head.m_Pos) % m_Capacity;
    }
    //This method is lock-free
    inline bool	Push(const ElementType& data)
    {
        bool ret = false;
        //Move the pointer first (equivalent to applying for operation rights for this thread's current operation)
        int oldPos = 0;
        for (int i = 0; i < s_c_MaxRetryCount; ++i) {
            oldPos = m_Tail.m_Pos;
            unsigned int tag = m_Tail.m_Tag;
            int pos = (oldPos + 1) % m_Capacity;
            if (pos == m_Head.m_Pos || !IsInvalid(m_pData[oldPos]))//If it is full or other threads have not completed the pop, this operation will be treated as a failure.
            {
                break;
            }
            if (lock_free_utility::compare_and_swap(reinterpret_cast<volatile unsigned int*>(&m_Tail), oldPos, tag, pos, tag + 1)) {
                ret = true;
                break;
            }
            else {
                lock_free_utility::pause();
            }
        }
        //Then modify the data
        if (ret) {
            ElementType _data = lock_free_utility::xchg(&(m_pData[oldPos]), data);
#if	defined(LockFreeDebugPrint)
            if (!IsInvalid(_data)) {
                LockFreeDebugPrint("Data[%d]:%u!=0 <-> %u in RingedQueue::Push\n", oldPos, (unsigned int)_data, (unsigned int)data);
            }
#endif
        }
        return ret;
    }
    //This method is lock-free
    inline bool	Pop(ElementType& data)
    {
        bool ret = false;
        //Move the pointer first (equivalent to applying for operation rights for this thread's current operation)
        int oldPos = 0;
        for (int i = 0; i < s_c_MaxRetryCount; ++i) {
            oldPos = m_Head.m_Pos;
            unsigned int tag = m_Head.m_Tag;
            int pos = (oldPos + 1) % m_Capacity;
            if (oldPos == m_Tail.m_Pos || IsInvalid(m_pData[oldPos]))//If it is empty or other threads have not completed the push, this operation will be treated as a failure.
            {
                break;
            }
            if (lock_free_utility::compare_and_swap(reinterpret_cast<volatile unsigned int*>(&m_Head), oldPos, tag, pos, tag + 1)) {
                ret = true;
                break;
            }
            else {
                lock_free_utility::pause();
            }
        }
        //Then modify the data
        if (ret) {
            data = lock_free_utility::xchg(&(m_pData[oldPos]), GetInvalid());
#if	defined(LockFreeDebugPrint)
            if (IsInvalid(data)) {
                LockFreeDebugPrint("Data[%d]:0==0 in RingedQueue::Pop\n", oldPos);
            }
#endif
        }
        return ret;
    }
    inline ElementType Pop(const ElementType& invalidValue = GetInvalid())
    {
        ElementType data;
        if (Pop(data)) {
            return data;
        }
        else {
            return invalidValue;
        }
    }
public:
    inline LockFreeRingedQueueT() :m_pData(NULL), m_Capacity(0)
    {
        if (SizeV > 0) {
            Init(SizeV);
        }
    }
    inline ~LockFreeRingedQueueT()
    {
        Clear();
        m_pData = NULL;
        m_Capacity = 0;
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
    ElementType* m_pData;
    volatile Pointer		m_Head;
    volatile Pointer		m_Tail;
    int						m_Capacity;
};

#pragma warning( pop )
#endif
#ifndef __HEAP_H__
#define __HEAP_H__

#include "Type.h"
#include "Queue.h"

template<typename T>
struct SimpleLessT
{
    bool operator () (const T& a, const T& b) const
    {
        return a < b;
    }
};

template<typename T>
struct SimpleGreatT
{
    bool operator () (const T& a, const T& b) const
    {
        return a > b;
    }
};

//Heap based on the array representation of a complete binary tree, the algorithm is adapted from the Chinese translation of "Data Structures and Algorithms"
//Used when a priority queue is needed
template<typename T, typename LessT = SimpleLessT<T>, int SizeV = 0>
class HeapT
{
    using MemoryType = typename CollectionMemory::SelectorT<T, (SizeV == 0 ? 0 : SizeV + 1)>::Type;
public:
    HeapT() :m_CurrentSize(0), m_MaxSize(0), m_Heap(0)
    {
        if constexpr (SizeV > 0) {
            Init(SizeV);
        }
    }
    HeapT(int size) :m_CurrentSize(0), m_MaxSize(0), m_Heap(0)
    {
        Init(size);
    }
    ~HeapT() {}
public:
    HeapT(const HeapT& other)
    {
        Init(other.m_MaxSize);
        CopyFrom(other);
    }
    HeapT& operator=(const HeapT& other)
    {
        if (this == &other)
            return *this;
        Clear();
        Init(other.m_MaxSize);
        CopyFrom(other);
        return *this;
    }
public:
    inline void Init(int size)
    {
        m_MaxSize = size;
        Create(size + 1);
    }
    inline void Clear()
    {
        m_CurrentSize = 0;
        m_Memory.Clear();
    }
public:
    inline int GetSize()const { return m_CurrentSize; }
    inline int GetMaxSize()const { return m_MaxSize; }
    inline const T& Root()
    {
        if (NULL == m_Heap || 0 == m_CurrentSize)
            return GetInvalidRef();
        else
            return m_Heap[1];
    }
    inline int Empty()const { return (0 == GetSize() ? TRUE : FALSE); }
    inline int Full()const { return (GetSize() == GetMaxSize() ? TRUE : FALSE); }
    inline int Push(const T& x);
    inline int Pop();
    inline int Build(T vals[], int size);
protected:
    inline void Create(int size)
    {
        m_Heap = m_Memory.Create(size);
        MyAssert(m_Heap);
        Clear();
    }
private:
    void CopyFrom(const HeapT& other)
    {
        Build(other.m_Heap + 1, other.m_CurrentSize);
    }
private:
    MemoryType	m_Memory;
    T* m_Heap;

    int m_CurrentSize;
    int m_MaxSize;

    LessT m_IsLess;
public:
    static inline T& GetInvalidRef()
    {
        static T s_Ins;
        return s_Ins;
    }
};

template<typename T, typename LessT, int SizeV>
inline int HeapT<T, LessT, SizeV>::Push(const T& x)
{
    if (NULL == m_Heap || TRUE == Full())
        return FALSE;
    else {
        int i = ++m_CurrentSize;
        while (i != 1 && m_IsLess(m_Heap[i / 2], x)) {
            m_Heap[i] = m_Heap[i / 2];
            i /= 2;
        }
        m_Heap[i] = x;
        return TRUE;
    }
}

template<typename T, typename LessT, int SizeV>
inline int HeapT<T, LessT, SizeV>::Pop()
{
    if (NULL == m_Heap || TRUE == Empty())
        return FALSE;
    else {
        T x = m_Heap[m_CurrentSize--];//last element
        int i = 1;//cur node
        int ci = 2;//i's child node
        while (ci <= m_CurrentSize) {
            //m_Heap[ci] is the largest child of i
            if (ci < m_CurrentSize && m_IsLess(m_Heap[ci], m_Heap[ci + 1]))
                ++ci;
            if (!m_IsLess(x, m_Heap[ci]))
                break;
            m_Heap[i] = m_Heap[ci];//Move child node up
            i = ci;
            ci *= 2;//Move down one level
        }
        m_Heap[i] = x;
        return TRUE;
    }
}

template<typename T, typename LessT, int SizeV>
inline int HeapT<T, LessT, SizeV>::Build(T vals[], int size)
{
    if (NULL == m_Heap || size > m_MaxSize)
        return FALSE;
    else {
        for (int i = 0; i < size; ++i) {
            m_Heap[i + 1] = vals[i];
        }
        m_CurrentSize = size;
        for (int i = m_CurrentSize / 2; i >= 1; --i) {
            T x = m_Heap[i];//root of the sub tree
            //Find where to place y
            int c = 2 * i;//The parent node of c is the target position of y
            while (c <= m_CurrentSize) {
                //m_Head[c] should be the larger sibling node
                if (c < m_CurrentSize && m_IsLess(m_Heap[c], m_Heap[c + 1]))
                    ++c;
                if (!m_IsLess(x, m_Heap[c]))
                    break;
                m_Heap[c / 2] = m_Heap[c];//Move child node up
                c *= 2;//Move down one level
            }
            m_Heap[c / 2] = x;
        }
        return TRUE;
    }
}

#endif //__HEAP_H__
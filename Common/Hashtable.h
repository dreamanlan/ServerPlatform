#ifndef __HASHTABLE_H__
#define __HASHTABLE_H__

#include "BaseType.h"
#include "Chain.h"

//�ַ���hashֵ����
unsigned int CalcStringHashCode(const char* str);

//��װc�ַ�����Ϊhash���key��hashcode�ļ��������Լ��ľ����㷨����Ҫ
//���������㷨ʱ����HashtableT�ĵ�����ģ������ṩ������KeyWorker�����е�
//GetHashCodeʵ�ֲ������㷨��Equal��Clean����StringKeyT����Ӧ��Ա���ɡ�
template<int CapacityV>
struct StringKeyT
{
    static const int MAX_CHAR_CAPACITY = CapacityV;
public://����HashtableT�ĵ�3��ģ���������Ľӿ�
    static unsigned int GetHashCode(const StringKeyT& key)
    {
        return (unsigned int)CalcStringHashCode(key.m_pString);
    }
    static int Equal(const StringKeyT& key1, const StringKeyT& key2)
    {
        return 0 == strcmp(key1.m_pString, key2.m_pString);
    }
    static void Clean(StringKeyT& key)
    {
        key.Clean();
    }
public://����HashtableT��1��ģ��������װ��׼c�ַ����Ľӿ�,֧�ֱ�׼c�ַ�����StringKeyT����ʽת��
    StringKeyT()
    {
        Clean();
    }
    //����֧�ֱ�׼c�ַ�����StringKeyT����ʽת�����˹��첻�����ַ������ݵĿ�������Ҫֱ��ʹ��
    StringKeyT(const char* p)
    {
        //�˺������⣬��ʹ��StringKeyT���ڲ��洢��ֱ�������ⲿc�ַ�����������HashtableT::Add,Get,Remove�Ĳ���ֱ��
        //ʹ��c�ַ���ʱ����ʽת��,StringKeyT�����������븳ֵ������ʹ���ڲ��洢�����������ⲿ�洢�ռ䡣
        m_pString = p;
    }
    StringKeyT(const StringKeyT& other)
    {
        CopyFrom(other);
    }
    StringKeyT& operator= (const StringKeyT& other)
    {
        if (this == &other)
            return *this;
        CopyFrom(other);
        return *this;
    }
    const char* GetString()const
    {
        return m_pString;
    }
private:
    void Clean()
    {
        memset(m_String, 0, sizeof(m_String));
        m_pString = m_String;
    }
    void CopyFrom(const StringKeyT& other)
    {
        Clean();
        strncpy(m_String, other.m_pString, MAX_CHAR_CAPACITY - 1);
    }
private:
    char		m_String[MAX_CHAR_CAPACITY];
    const char* m_pString;
};

//��ϣ��Ļ������ܣ�������ȡ, ����hash���㣬��ͻ����ȣ�
class HashtableBase
{
protected:
    static const unsigned int INVALID_HASH_INDEX = 0xFFFFFFFF;
    static const unsigned int MAX_HASH_INDEX_VALUE = 0x7FFFFFFF;
    enum
    {
        IDTS_EMPTY = 0,
        IDTS_USED = 1,
        IDTS_REMOVED = 2,
    };
    class ISlotKey
    {
    public:
        virtual int KeyEqual(unsigned int index) const = 0;
        virtual unsigned int GetHashCode() const = 0;
    };
    class ISlot
    {
    public:
        virtual unsigned int GetHashCode() const = 0;
        virtual void SetStatus(unsigned int) = 0;
        virtual unsigned int GetStatus() const = 0;
        virtual void Cleanup() = 0;
    public:
        virtual ~ISlot() {}
    };
protected:
    //����һ������
    unsigned int		PrepareAddIndex(const ISlotKey& key);
    //��ȡ��Ϣ
    unsigned int		Find(const ISlotKey& key) const;
    //ɾ������
    unsigned int		Remove(const ISlotKey& key);
    //�����������
    void		Cleanup(void);
protected:
    virtual unsigned int			GetSlotCount() const = 0;
    virtual ISlot& GetSlot(unsigned int index) = 0;
    virtual const ISlot& GetSlot(unsigned int index) const = 0;
protected:
    virtual ~HashtableBase() {};
protected:
    //�õ�һ�����ڵ���ָ��ֵ������,code����С��0x7FFFFFFF(���з���32λ���������ֵ)
    static unsigned int	GetPrime(unsigned int code);
private:
    //����hash����ֵ,hashSize--��ϣ������ߴ磬code--���ڼ����ϣֵ��ԭʼֵ��incr--�ڹ�ϣ����������ͻ�����ϣֵ������
    static unsigned int	ToIndex(unsigned int hashSize, unsigned int code, unsigned int& incr);
    //�ж�ĳ���Ƿ�������
    static int IsPrime(unsigned int val);
};

//������ֵ��������hash��ʱ��HashtableT�ĵ�����ģ�����
template<typename KeyT>
class DefKeyWorkerT
{
public:
    static unsigned int GetHashCode(const KeyT& key)
    {
        return (unsigned int)key;
    }
    static int Equal(const KeyT& key1, const KeyT& key2)
    {
        return key1 == key2;
    }
    static void Clean(KeyT& key)
    {
        key = (KeyT)INVALID_ID;
    }
};

template<typename KeyT>
class DefKeyWorkerT<KeyT*>
{
public:
    static unsigned int GetHashCode(const KeyT* key)
    {
        unsigned int val = 0;
        memcpy(&val, &key, sizeof(unsigned int));
        return val;
    }
    static int Equal(const KeyT* key1, const KeyT* key2)
    {
        return key1 == key2;
    }
    static void Clean(KeyT*& key)
    {
        key = NULL;
    }
};

//HashtableT�ĵ��ĸ�ģ�������Ĭ��ʵ�֡�
template<typename ValT>
class DefValueWorkerT
{
public:
    static void Clean(ValT& val)
    {
        val = ValT();
    }
};

template<typename ValT>
class DefValueWorkerT<ValT*>
{
public:
    static void Clean(ValT*& val)
    {
        val = NULL;
    }
};

//HashtableT�ĵ��ĸ�ģ�������ʵ�֣�������INVALID_ID��ʾ��Чֵ�����Ρ�
template<typename ValT, ValT InvalidVal = INVALID_ID>
class IntegerValueWorkerT
{
public:
    static void Clean(ValT& val)
    {
        val = InvalidVal;
    }
};

//���ڱ���ʱ���Ҵ��ڵ���ָ��ֵ�������Ĵ��루ʵ��û��İ취�ˣ������������ֱȽϻ�ɬ�ļ������벻ҪԹ�ң���
namespace HashtableUtility
{
    template<bool v, int V,
        int K1 = 0, int K2 = 0, int K3 = 0, int K4 = 0, int K5 = 0, int K6 = 0, int K7 = 0, int K8 = 0, int K9 = 0, int K10 = 0, int K11 = 0, int K12 = 0,
        int K13 = 0, int K14 = 0, int K15 = 0, int K16 = 0, int K17 = 0, int K18 = 0, int K19 = 0, int K20 = 0, int K21 = 0, int K22 = 0, int K23 = 0, int K24 = 0,
        int K25 = 0, int K26 = 0, int K27 = 0, int K28 = 0, int K29 = 0, int K30 = 0, int K31 = 0, int K32 = 0, int K33 = 0, int K34 = 0, int K35 = 0, int K36 = 0,
        int K37 = 0, int K38 = 0, int K39 = 0, int K40 = 0, int K41 = 0, int K42 = 0, int K43 = 0, int K44 = 0, int K45 = 0, int K46 = 0, int K47 = 0, int K48 = 0,
        int K49 = 0, int K50 = 0, int K51 = 0, int K52 = 0, int K53 = 0, int K54 = 0, int K55 = 0, int K56 = 0, int K57 = 0, int K58 = 0, int K59 = 0, int K60 = 0,
        int K61 = 0, int K62 = 0, int K63 = 0, int K64 = 0, int K65 = 0, int K66 = 0, int K67 = 0, int K68 = 0, int K69 = 0, int K70 = 0, int K71 = 0, int K72 = 0>
    class FindImpl
    {
    public:
        static const int Value = K1;
    };
    template<int V,
        int K1, int K2, int K3, int K4, int K5, int K6, int K7, int K8, int K9, int K10, int K11, int K12,
        int K13, int K14, int K15, int K16, int K17, int K18, int K19, int K20, int K21, int K22, int K23, int K24,
        int K25, int K26, int K27, int K28, int K29, int K30, int K31, int K32, int K33, int K34, int K35, int K36,
        int K37, int K38, int K39, int K40, int K41, int K42, int K43, int K44, int K45, int K46, int K47, int K48,
        int K49, int K50, int K51, int K52, int K53, int K54, int K55, int K56, int K57, int K58, int K59, int K60,
        int K61, int K62, int K63, int K64, int K65, int K66, int K67, int K68, int K69, int K70, int K71, int K72>
    class FindImpl<false, V, K1, K2, K3, K4, K5, K6, K7, K8, K9, K10, K11, K12, K13, K14, K15, K16, K17, K18, K19, K20, K21, K22, K23, K24, K25, K26, K27, K28, K29, K30, K31, K32, K33, K34, K35, K36, K37, K38, K39, K40, K41, K42, K43, K44, K45, K46, K47, K48, K49, K50, K51, K52, K53, K54, K55, K56, K57, K58, K59, K60, K61, K62, K63, K64, K65, K66, K67, K68, K69, K70, K71, K72>
    {
    public:
        static const int Value = FindImpl<(V <= K2), V, K2, K3, K4, K5, K6, K7, K8, K9, K10, K11, K12, K13, K14, K15, K16, K17, K18, K19, K20, K21, K22, K23, K24, K25, K26, K27, K28, K29, K30, K31, K32, K33, K34, K35, K36, K37, K38, K39, K40, K41, K42, K43, K44, K45, K46, K47, K48, K49, K50, K51, K52, K53, K54, K55, K56, K57, K58, K59, K60, K61, K62, K63, K64, K65, K66, K67, K68, K69, K70, K71, K72>::Value;
    };
    template<int V,
        int K1 = 0, int K2 = 0, int K3 = 0, int K4 = 0, int K5 = 0, int K6 = 0, int K7 = 0, int K8 = 0, int K9 = 0, int K10 = 0, int K11 = 0, int K12 = 0,
        int K13 = 0, int K14 = 0, int K15 = 0, int K16 = 0, int K17 = 0, int K18 = 0, int K19 = 0, int K20 = 0, int K21 = 0, int K22 = 0, int K23 = 0, int K24 = 0,
        int K25 = 0, int K26 = 0, int K27 = 0, int K28 = 0, int K29 = 0, int K30 = 0, int K31 = 0, int K32 = 0, int K33 = 0, int K34 = 0, int K35 = 0, int K36 = 0,
        int K37 = 0, int K38 = 0, int K39 = 0, int K40 = 0, int K41 = 0, int K42 = 0, int K43 = 0, int K44 = 0, int K45 = 0, int K46 = 0, int K47 = 0, int K48 = 0,
        int K49 = 0, int K50 = 0, int K51 = 0, int K52 = 0, int K53 = 0, int K54 = 0, int K55 = 0, int K56 = 0, int K57 = 0, int K58 = 0, int K59 = 0, int K60 = 0,
        int K61 = 0, int K62 = 0, int K63 = 0, int K64 = 0, int K65 = 0, int K66 = 0, int K67 = 0, int K68 = 0, int K69 = 0, int K70 = 0, int K71 = 0, int K72 = 0>
    class Find
    {
    public:
        static const int Value = FindImpl<(V <= K1), V, K1, K2, K3, K4, K5, K6, K7, K8, K9, K10, K11, K12, K13, K14, K15, K16, K17, K18, K19, K20, K21, K22, K23, K24, K25, K26, K27, K28, K29, K30, K31, K32, K33, K34, K35, K36, K37, K38, K39, K40, K41, K42, K43, K44, K45, K46, K47, K48, K49, K50, K51, K52, K53, K54, K55, K56, K57, K58, K59, K60, K61, K62, K63, K64, K65, K66, K67, K68, K69, K70, K71, K72>::Value;
    };
    template<int v>
    class Prime
    {
    public:
        static const int Value = Find<v,
            3, 7, 11, 0x11, 0x17, 0x1d, 0x25, 0x2f, 0x3b, 0x47, 0x59, 0x6b, 0x83, 0xa3, 0xc5, 0xef,
            0x125, 0x161, 0x1af, 0x209, 0x277, 0x2f9, 0x397, 0x44f, 0x52f, 0x63d, 0x78b, 0x91d, 0xaf1, 0xd2b, 0xfd1, 0x12fd,
            0x16cf, 0x1b65, 0x20e3, 0x2777, 0x2f6f, 0x38ff, 0x446f, 0x521f, 0x628d, 0x7655, 0x8e01, 0xaa6b, 0xcc89, 0xf583, 0x126a7, 0x1619b,
            0x1a857, 0x1fd3b, 0x26315, 0x2dd67, 0x3701b, 0x42023, 0x4f361, 0x5f0ed, 0x72125, 0x88e31, 0xa443b, 0xc51eb, 0xec8c1, 0x11bdbf, 0x154a3f, 0x198c4f,
            0x1ea867, 0x24ca19, 0x2c25c1, 0x34fa1b, 0x3f928f, 0x4c4987, 0x5b8b6f, 0x6dda89
        >::Value;
    };
}

//��ϣ��ģ����(���֧��0x7FFFFFFF��Ԫ��)
template<typename KeyT, typename ValT, typename KeyWorkerT = DefKeyWorkerT<KeyT>, typename ValueWorkerT = DefValueWorkerT<ValT>, int SizeV = 0 >
class HashtableT : private HashtableBase
{
    using HashtableType = HashtableT<KeyT, ValT, KeyWorkerT, ValueWorkerT, SizeV>;
    friend class SlotKey;
    class SlotKey : public ISlotKey
    {
    public:
        virtual int KeyEqual(unsigned int index) const
        {
            return KeyWorkerT::Equal(m_Key, m_pHashtable->GetSlotImpl(index).m_ID);
        }
        virtual unsigned int GetHashCode() const
        {
            return KeyWorkerT::GetHashCode(m_Key);
        }
    public:
        SlotKey(const HashtableType* ptr, const KeyT& key) :m_pHashtable(ptr), m_Key(key)
        {
            MyAssert(m_pHashtable);
        }
    private://���ÿ����븳ֵ������ʹ�����ð�װ��ֻ�������޳���
        SlotKey(const SlotKey&);
        SlotKey& operator= (const SlotKey&);
    private:
        const KeyT& m_Key;
        const HashtableType* m_pHashtable;
    };
    class Slot : public ISlot
    {
    public:
        virtual unsigned int GetHashCode() const
        {
            return KeyWorkerT::GetHashCode(m_ID);
        }
        virtual void SetStatus(unsigned int status)
        {
            m_Status = status;
        }
        virtual unsigned int GetStatus() const
        {
            return m_Status;
        }
        virtual void Cleanup()
        {
            Clear();
        }
    public:
        Slot()
        {
            Clear();
        }
    private:
        void Clear()
        {
            m_Status = IDTS_EMPTY;
            KeyWorkerT::Clean(m_ID);
            ValueWorkerT::Clean(m_Value);
        }
    public:
        unsigned int	m_Status;
        KeyT	m_ID;
        ValT	m_Value;
    };
public:
    class KeyValueData
    {
        friend class HashtableT<KeyT, ValT, KeyWorkerT, ValueWorkerT, SizeV>;
    public:
        const KeyT& GetKey()const
        {
            return m_Slot.m_ID;
        }
        const ValT& GetValue()const
        {
            return m_Slot.m_Value;
        }
        ValT& GetValue()
        {
            return m_Slot.m_Value;
        }
    public:
        void		Recycle() { m_Allocated = FALSE; }
        int		IsAllocated()const { return m_Allocated; }
    public:
        KeyValueData() :m_Allocated(FALSE) {}
    private:
        const Slot& GetSlot()const { return m_Slot; }
        Slot& GetSlot() { return m_Slot; }
        void		UpdateAllocated(int allocated) { m_Allocated = allocated; }
    private:
        Slot		m_Slot;
        int		m_Allocated;
    };
    using Recycler = NodeRecyclerForDataT<KeyValueData>;
    using KeyValueDataNode = CChainNode<KeyValueData, Recycler>;
    using KeyValueDataChain = CChain<KeyValueData, Recycler>;
    using Iterator = typename KeyValueDataNode::Iterator;
    using MemoryType = typename CollectionMemory::SelectorT<KeyValueDataNode, (SizeV == 0 ? 0 : HashtableUtility::Prime<SizeV>::Value)>::Type;
public:
    HashtableT() :m_pTable(NULL), m_Count(0)
    {
        if constexpr (SizeV > 0) {
            InitTable(SizeV);
        }
    }
    HashtableT(unsigned int maxItem) :m_pTable(NULL), m_Count(0)
    {
        InitTable(maxItem);
    }
    virtual ~HashtableT()
    {
        CleanUp();
        m_pTable = NULL;
        m_Count = 0;
    }
    int IsInited()const
    {
        return (NULL != m_pTable && 0 < m_Count ? TRUE : FALSE);
    }
public:
    HashtableT(const HashtableT& other)
    {
        InitTable(other.m_Count);
        CopyFrom(other);
    }
    HashtableT& operator=(const HashtableT& other)
    {
        if (this == &other)
            return *this;
        CleanUp();
        InitTable(other.m_Count);
        CopyFrom(other);
        return *this;
    }
public:
    //��ʼ����
    inline void				InitTable(unsigned int maxItem)
    {
        unsigned int maxVal = HashtableBase::GetPrime(maxItem);
        if (maxVal == INVALID_HASH_INDEX) {
            MyAssert(FALSE);
            return;
        }
        Create(maxVal);
    }
    //����һ������
    inline int				Add(const KeyT& id, const ValT& val)
    {
        SlotKey key(this, id);
        unsigned int index = Find(key);
        if (index != INVALID_HASH_INDEX)
            return FALSE;
        index = PrepareAddIndex(key);
        if (index == INVALID_HASH_INDEX || index >= m_Count)
            return FALSE;
        GetSlotImpl(index).m_ID = id;
        GetSlotImpl(index).m_Value = val;
        Iterator it = GetChainIterator(index);
        it->UpdateAllocated(TRUE);
        m_DataChain.AddLast(it);
        return TRUE;
    }
    //��ȡ��Ϣ
    inline const ValT& Get(const KeyT& id) const
    {
        SlotKey key(this, id);
        unsigned int index = HashtableBase::Find(key);
        if (index == INVALID_HASH_INDEX || index >= m_Count)
            return GetInvalidValueRef();
        return GetSlotImpl(index).m_Value;
    }
    //��ȡ��Ϣ
    inline ValT& Get(const KeyT& id)
    {
        SlotKey key(this, id);
        unsigned int index = HashtableBase::Find(key);
        if (index == INVALID_HASH_INDEX || index >= m_Count)
            return GetInvalidValueRef();
        return GetSlotImpl(index).m_Value;
    }
    //ɾ������
    inline const Iterator	Remove(const KeyT& id)
    {
        SlotKey key(this, id);
        unsigned int index = HashtableBase::Remove(key);
        if (index == INVALID_HASH_INDEX || index >= m_Count)
            return Iterator();
        return m_DataChain.Remove(GetChainIterator(index));
    }
    //ɾ������
    inline const Iterator	Remove(const Iterator& it)
    {
        SlotKey key(this, it->GetKey());
        unsigned int index = HashtableBase::Remove(key);
        if (index == INVALID_HASH_INDEX || index >= m_Count)
            return Iterator();
        return m_DataChain.Remove(GetChainIterator(index));
    }
    //�ƶ�ָ�����Ϊ��������׽��
    inline const Iterator	MoveAsFirst(const Iterator& it)
    {
        return m_DataChain.MoveAsFirst(it);
    }
    //�ƶ�ָ�����Ϊ�������β���
    inline const Iterator	MoveAsLast(const Iterator& it)
    {
        return m_DataChain.MoveAsLast(it);
    }
    //�����������
    inline void				CleanUp(void)
    {
        HashtableBase::Cleanup();
        m_DataChain.RemoveAll();
    }
    //
    inline const Iterator	First() const
    {
        return m_DataChain.First();
    }
    //
    inline const Iterator	Last() const
    {
        return m_DataChain.Last();
    }
    //
    inline const Iterator	GetIterator(int i) const
    {
        return m_DataChain.GetIterator(i);
    }
    //
    inline int				IndexOf(const KeyT& id) const
    {
        SlotKey key(this, id);
        unsigned int index = HashtableBase::Find(key);
        if (index == INVALID_HASH_INDEX || index >= m_Count)
            return INVALID_INDEX;
        return m_DataChain.IndexOf(GetChainIterator(index));
    }
    //
    inline unsigned int				GetNum() const
    {
        return m_DataChain.GetNum();
    }
    //
    inline int				Empty() const
    {
        if (GetNum() == 0)
            return TRUE;
        else
            return FALSE;
    }
    //
    inline int				Full() const
    {
        if (GetNum() >= m_Count)
            return TRUE;
        else
            return FALSE;
    }
    inline size_t				GetMemoryInUsed() const
    {
        return m_Memory.GetMemoryInUsed();
    }
protected:
    virtual unsigned int			GetSlotCount() const
    {
        return m_Count;
    }
    virtual ISlot& GetSlot(unsigned int index)
    {
        return GetSlotImpl(index);
    }
    virtual const ISlot& GetSlot(unsigned int index) const
    {
        return GetSlotImpl(index);
    }
protected:
    inline void Create(unsigned int size)
    {
        int v = (int)size;
        m_pTable = m_Memory.Create(v);
        m_Count = (unsigned int)v;
        MyAssert(m_pTable);
        CleanUp();
    }
private:
    inline Slot& GetSlotImpl(unsigned int index)
    {
        return const_cast<Slot&>(static_cast<const HashtableType*>(this)->GetSlotImpl(index));
    }
    inline const Slot& GetSlotImpl(unsigned int index)const
    {
        if (index >= m_Count) {
            MyAssert(FALSE);
        }
        MyAssertEx(m_pTable, "You must call InitTable first!");
        return m_pTable[index].GetData().GetSlot();
    }
    inline const typename KeyValueDataNode::Iterator GetChainIterator(unsigned int index) const
    {
        if (index >= m_Count) {
            MyAssert(FALSE);
        }
        MyAssertEx(m_pTable, "You must call InitTable first!");
        return typename KeyValueDataNode::Iterator(&(m_pTable[index]));
    }
private:
    void CopyFrom(const HashtableT& other)
    {
        //		CleanUp();
        for (Iterator it = other.First(); FALSE == it.IsNull(); ++it) {
            Add(it->GetKey(), it->GetValue());
        }
    }
private:
    unsigned int				m_Count;
    KeyValueDataNode* m_pTable;
    KeyValueDataChain	m_DataChain;
    MemoryType			m_Memory;
public:
    static inline KeyT& GetInvalidKeyRef()
    {
        static KeyT s_Key;
        KeyWorkerT::Clean(s_Key);
        return s_Key;
    }
    static inline ValT& GetInvalidValueRef()
    {
        static ValT s_Val;
        ValueWorkerT::Clean(s_Val);
        return s_Val;
    }
};

//��ϣ����ģ����(���֧��0x7FFFFFFF��Ԫ��)
template<typename KeyT, typename KeyWorkerT = DefKeyWorkerT<KeyT>, int SizeV = 0 >
class HashsetT : private HashtableBase
{
    using HashsetType = HashsetT<KeyT, KeyWorkerT, SizeV>;
    friend class SlotKey;
    class SlotKey : public ISlotKey
    {
    public:
        virtual int KeyEqual(unsigned int index) const
        {
            return KeyWorkerT::Equal(m_Key, m_pHashset->GetSlotImpl(index).m_ID);
        }
        virtual unsigned int GetHashCode() const
        {
            return KeyWorkerT::GetHashCode(m_Key);
        }
    public:
        SlotKey(const HashsetType* ptr, const KeyT& key) :m_pHashset(ptr), m_Key(key)
        {
            MyAssert(m_pHashset);
        }
    private://���ÿ����븳ֵ������ʹ�����ð�װ��ֻ�������޳���
        SlotKey(const SlotKey&);
        SlotKey& operator= (const SlotKey&);
    private:
        const KeyT& m_Key;
        const HashsetType* m_pHashset;
    };
    class Slot : public ISlot
    {
    public:
        virtual unsigned int GetHashCode() const
        {
            return KeyWorkerT::GetHashCode(m_ID);
        }
        virtual void SetStatus(unsigned int status)
        {
            m_Status = status;
        }
        virtual unsigned int GetStatus() const
        {
            return m_Status;
        }
        virtual void Cleanup()
        {
            Clear();
        }
    public:
        Slot()
        {
            Clear();
        }
    private:
        void Clear()
        {
            m_Status = IDTS_EMPTY;
            KeyWorkerT::Clean(m_ID);
        }
    public:
        unsigned int	m_Status;
        KeyT	m_ID;
    };
public:
    class KeyData
    {
        friend class HashsetT<KeyT, KeyWorkerT, SizeV>;
    public:
        const KeyT& GetKey()const
        {
            return m_Slot.m_ID;
        }
    public:
        void		Recycle() { m_Allocated = FALSE; }
        int		IsAllocated()const { return m_Allocated; }
    public:
        KeyData() :m_Allocated(FALSE) {}
    private:
        const Slot& GetSlot()const { return m_Slot; }
        Slot& GetSlot() { return m_Slot; }
        void		UpdateAllocated(int allocated) { m_Allocated = allocated; }
    private:
        Slot		m_Slot;
        int		m_Allocated;
    };
    using Recycler = NodeRecyclerForDataT<KeyData>;
    using KeyDataNode = CChainNode<KeyData, Recycler>;
    using KeyDataChain = CChain<KeyData, Recycler>;
    using Iterator = typename KeyDataNode::Iterator;
    using MemoryType = typename CollectionMemory::SelectorT<KeyDataNode, (SizeV == 0 ? 0 : HashtableUtility::Prime<SizeV>::Value)>::Type;
public:
    HashsetT() :m_pTable(NULL), m_Count(0)
    {
        if (SizeV > 0) {
            InitSet(SizeV);
        }
    }
    HashsetT(unsigned int maxItem) :m_pTable(NULL), m_Count(0)
    {
        InitSet(maxItem);
    }
    virtual ~HashsetT()
    {
        CleanUp();
        m_pTable = NULL;
        m_Count = 0;
    }
    int IsInited()const
    {
        return (NULL != m_pTable && 0 < m_Count ? TRUE : FALSE);
    }
public:
    HashsetT(const HashsetT& other)
    {
        InitSet(other.m_Count);
        CopyFrom(other);
    }
    HashsetT& operator=(const HashsetT& other)
    {
        if (this == &other)
            return *this;
        CleanUp();
        InitSet(other.m_Count);
        CopyFrom(other);
        return *this;
    }
public:
    //��ʼ����
    inline void				InitSet(unsigned int maxItem)
    {
        unsigned int maxVal = HashtableBase::GetPrime(maxItem);
        if (maxVal == INVALID_HASH_INDEX) {
            MyAssert(FALSE);
            return;
        }
        Create(maxVal);
    }
    //����һ������
    inline int				Insert(const KeyT& id)
    {
        SlotKey key(this, id);
        unsigned int index = Find(key);
        if (index != INVALID_HASH_INDEX)
            return FALSE;
        index = PrepareAddIndex(key);
        if (index == INVALID_HASH_INDEX || index >= m_Count)
            return FALSE;
        GetSlotImpl(index).m_ID = id;
        Iterator it = GetChainIterator(index);
        it->UpdateAllocated(TRUE);
        m_DataChain.AddLast(it);
        return TRUE;
    }
    //��ȡ��Ϣ
    inline int				Exist(const KeyT& id) const
    {
        SlotKey key(this, id);
        unsigned int index = HashtableBase::Find(key);
        if (index == INVALID_HASH_INDEX || index >= m_Count)
            return FALSE;
        return TRUE;
    }
    //ɾ������
    inline const Iterator	Remove(const KeyT& id)
    {
        SlotKey key(this, id);
        unsigned int index = HashtableBase::Remove(key);
        if (index == INVALID_HASH_INDEX || index >= m_Count)
            return Iterator();
        return m_DataChain.Remove(GetChainIterator(index));
    }
    //ɾ������
    inline const Iterator	Remove(const Iterator& it)
    {
        SlotKey key(this, it->GetKey());
        unsigned int index = HashtableBase::Remove(key);
        if (index == INVALID_HASH_INDEX || index >= m_Count)
            return Iterator();
        return m_DataChain.Remove(GetChainIterator(index));
    }
    //�ƶ�ָ�����Ϊ��������׽��
    inline const Iterator	MoveAsFirst(const Iterator& it)
    {
        return m_DataChain.MoveAsFirst(it);
    }
    //�ƶ�ָ�����Ϊ�������β���
    inline const Iterator	MoveAsLast(const Iterator& it)
    {
        return m_DataChain.MoveAsLast(it);
    }
    //�����������
    inline void				CleanUp(void)
    {
        HashtableBase::Cleanup();
        m_DataChain.RemoveAll();
    }
    //
    inline const Iterator	First() const
    {
        return m_DataChain.First();
    }
    //
    inline const Iterator	Last() const
    {
        return m_DataChain.Last();
    }
    //
    inline const Iterator	GetIterator(int i) const
    {
        return m_DataChain.GetIterator(i);
    }
    //
    inline int				IndexOf(const KeyT& id) const
    {
        SlotKey key(this, id);
        unsigned int index = HashtableBase::Find(key);
        if (index == INVALID_HASH_INDEX || index >= m_Count)
            return INVALID_INDEX;
        return m_DataChain.IndexOf(GetChainIterator(index));
    }
    //
    inline unsigned int				GetNum() const
    {
        return m_DataChain.GetNum();
    }
    //
    inline int				Empty() const
    {
        if (GetNum() == 0)
            return TRUE;
        else
            return FALSE;
    }
    //
    inline int				Full() const
    {
        if (GetNum() >= m_Count)
            return TRUE;
        else
            return FALSE;
    }
protected:
    virtual unsigned int			GetSlotCount() const
    {
        return m_Count;
    }
    virtual ISlot& GetSlot(unsigned int index)
    {
        return GetSlotImpl(index);
    }
    virtual const ISlot& GetSlot(unsigned int index) const
    {
        return GetSlotImpl(index);
    }
protected:
    inline void Create(unsigned int size)
    {
        int v = (int)size;
        m_pTable = m_Memory.Create(v);
        m_Count = (unsigned int)v;
        MyAssert(m_pTable);
        CleanUp();
    }
private:
    inline Slot& GetSlotImpl(unsigned int index)
    {
        return const_cast<Slot&>(static_cast<const HashsetType*>(this)->GetSlotImpl(index));
    }
    inline const Slot& GetSlotImpl(unsigned int index)const
    {
        if (index >= m_Count) {
            MyAssert(FALSE);
        }
        MyAssertEx(m_pTable, "You must call InitSet first!");
        return m_pTable[index].GetData().GetSlot();
    }
    inline const typename KeyDataNode::Iterator GetChainIterator(unsigned int index) const
    {
        if (index >= m_Count) {
            MyAssert(FALSE);
        }
        MyAssertEx(m_pTable, "You must call InitSet first!");
        return typename KeyDataNode::Iterator(&(m_pTable[index]));
    }
private:
    void CopyFrom(const HashsetT& other)
    {
        //		CleanUp();
        for (Iterator it = other.First(); FALSE == it.IsNull(); ++it) {
            Insert(it->GetKey());
        }
    }
private:
    unsigned int			m_Count;
    KeyDataNode* m_pTable;
    KeyDataChain	m_DataChain;
    MemoryType		m_Memory;
public:
    static inline KeyT& GetInvalidKeyRef()
    {
        static KeyT s_Key;
        KeyWorkerT::Clean(s_Key);
        return s_Key;
    }
};

#endif

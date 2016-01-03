#ifndef ATOMIC_H__
#define ATOMIC_H__

#define REG_OP32 "l"
#define REG_OP64 "q"
#define REG_OPL REG_OP64 //在linux编译为64位指令

#define LockFreeDebugPrint	printf

static const int	s_c_RetryCountForSpinOperation = 256;

static inline unsigned int* _increment(unsigned int* volatile *p);
static inline unsigned long _increment(volatile unsigned long *p);
static inline unsigned int _increment(volatile unsigned int *p);
static inline unsigned short _increment(volatile unsigned short *p);
static inline unsigned char _increment(volatile unsigned char *p);
static inline unsigned int* _decrement(unsigned int* volatile *p);
static inline unsigned long _decrement(volatile unsigned long *p);
static inline unsigned int _decrement(volatile unsigned int *p);
static inline unsigned short _decrement(volatile unsigned short *p);
static inline unsigned char _decrement(volatile unsigned char *p);
static inline unsigned int* _fetch_and_add(unsigned int* volatile *p, unsigned int* add);
static inline unsigned long _fetch_and_add(volatile unsigned long *p, unsigned long add);
static inline unsigned int _fetch_and_add(volatile unsigned int *p, unsigned int add);
static inline unsigned short _fetch_and_add(volatile unsigned short *p, unsigned short add);
static inline unsigned char _fetch_and_add(volatile unsigned char *p, unsigned char add);
static inline char _compare_and_swap(unsigned int* volatile *p, unsigned int* val_old, unsigned int* val_new);
static inline char _compare_and_swap(volatile unsigned long* p, unsigned long val_old, unsigned long val_new);
static inline char _compare_and_swap(volatile unsigned int* p, unsigned int val_old, unsigned int val_new);
static inline char _compare_and_swap(volatile unsigned short* p, unsigned short val_old, unsigned short val_new);
static inline char _compare_and_swap(volatile unsigned char* p, unsigned char val_old, unsigned char val_new);
static inline char _compare_double_and_swap_double(volatile unsigned int *p, unsigned int val_old_m1, unsigned int val_old_m2, unsigned int val_new_m1, unsigned int val_new_m2);
static inline char _compare_double_and_swap_double(volatile unsigned long *p, unsigned long val_old_m1, unsigned long val_old_m2, unsigned long val_new_m1, unsigned long val_new_m2);
static inline unsigned int* _xchg(unsigned int* volatile *p,unsigned int* val);
static inline unsigned long _xchg(volatile unsigned long* p,unsigned long val);
static inline unsigned int _xchg(volatile unsigned int* p,unsigned int val);
static inline unsigned short _xchg(volatile unsigned short* p,unsigned short val);
static inline unsigned char _xchg(volatile unsigned char* p,unsigned char val);
static inline void _pause(void);

namespace lock_free_utility
{
	namespace _impl_private
	{
		template<class T>
		struct DeduceArgTypeT
		{
			typedef T Type;
		};
		template<class T>
		struct DeduceArgTypeT<T*>
		{
			typedef unsigned int* Type;
		};
		template<>
		struct DeduceArgTypeT<char>
		{
			typedef unsigned char Type;
		};
		template<>
		struct DeduceArgTypeT<short>
		{
			typedef unsigned short Type;
		};
		template<>
		struct DeduceArgTypeT<int>
		{
			typedef unsigned int Type;
		};
		template<>
		struct DeduceArgTypeT<long>
		{
			typedef unsigned long Type;
		};
		template<>
		struct DeduceArgTypeT<size_t>
		{
			typedef unsigned long Type;
		};
	}

	template<class t>
		inline t increment(volatile t* p)
	{
		return (t)(_increment(reinterpret_cast<volatile typename _impl_private::DeduceArgTypeT<t>::Type*>(p)));
	}
	template<class t>
		inline t decrement(volatile t* p)
	{
		return (t)(_decrement(reinterpret_cast<volatile typename _impl_private::DeduceArgTypeT<t>::Type*>(p)));
	}
	template<class t,class vt>
		inline t fetch_and_add(volatile t *p, vt _add)
	{
		return (t)(_fetch_and_add(reinterpret_cast<volatile typename _impl_private::DeduceArgTypeT<t>::Type*>(p), (typename _impl_private::DeduceArgTypeT<t>::Type)(_add)));
	}
	template<class t,class vt1,class vt2>
		inline bool compare_and_swap(volatile t *p, vt1 val_old, vt2 val_new)
	{
		return 1 == _compare_and_swap(reinterpret_cast<volatile typename _impl_private::DeduceArgTypeT<t>::Type*>(p),(typename _impl_private::DeduceArgTypeT<t>::Type)(val_old),(typename _impl_private::DeduceArgTypeT<t>::Type)(val_new));
	}

	inline bool compare_and_swap(volatile unsigned int * p, unsigned int val_old_m1, unsigned int val_old_m2, unsigned int val_new_m1, unsigned int val_new_m2)
	{
		return 1==_compare_double_and_swap_double(p,val_old_m1,val_old_m2,val_new_m1,val_new_m2);
	}

	inline bool compare_and_swap(volatile unsigned long * p, unsigned long val_old_m1, unsigned long val_old_m2, unsigned long val_new_m1, unsigned long val_new_m2)
	{
		return 1==_compare_double_and_swap_double(p,val_old_m1,val_old_m2,val_new_m1,val_new_m2);
	}

	template<class t,class vt>
		inline t xchg(volatile t* p,vt val)
	{
		return (t)_xchg(reinterpret_cast<volatile typename _impl_private::DeduceArgTypeT<t>::Type*>(const_cast<t*>(p)),(typename _impl_private::DeduceArgTypeT<t>::Type)(val));
	}

	inline void pause(void)
	{
		_pause();
	}
}

#include "Atomic.inl"

#endif
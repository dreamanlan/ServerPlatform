#ifndef ATOMIC_INL__
#define ATOMIC_INL__

#ifdef __WINDOWS__
#include <intrin.h>
#endif 

static inline unsigned int* _increment(unsigned int* volatile* p)
{
    unsigned int* _ret = 0;
#ifdef __WINDOWS__
#ifndef _WIN64
    _ret = (unsigned int*)_InterlockedIncrement((long*)p);
#else
    _ret = (unsigned int*)_InterlockedIncrement64((__int64*)p);
#endif
#else
    /* xaddl or xaddq */
    __asm__ __volatile__("lock; xadd" REG_OPL " %0, %1;inc %0"
        :"=r"(_ret), "=m"(*p)
        : "0"(1), "m"(*p)
        : "memory");
#endif
    return _ret;

}

static inline unsigned long _increment(volatile unsigned long* p)
{
    unsigned long _ret = 0;
#ifdef __WINDOWS__
    _ret = (unsigned long)_InterlockedIncrement((long*)p);
#else
    /* xaddl or xaddq */
    __asm__ __volatile__("lock; xadd" REG_OPL " %0, %1;inc %0"
        :"=r"(_ret), "=m"(*p)
        : "0"(1), "m"(*p)
        : "memory");
#endif
    return _ret;
}

static inline unsigned int _increment(volatile unsigned int* p)
{
    unsigned int _ret = 0;
#ifdef __WINDOWS__
    _ret = (unsigned int)_InterlockedIncrement((long*)p);
#else
    /* xaddl or xaddq */
    __asm__ __volatile__("lock; xadd" REG_OP32 " %0, %1;inc %0"
        :"=r"(_ret), "=m"(*p)
        : "0"(1), "m"(*p)
        : "memory");
#endif
    return _ret;

}

static inline unsigned short _increment(volatile unsigned short* p)
{
    unsigned short _ret = 0;
#ifdef __WINDOWS__
    _ret = (unsigned short)_InterlockedIncrement16((short*)p);
#else
    /* xaddl or xaddq */
    __asm__ __volatile__("lock; xaddw %0, %1;inc %0"
        :"=r"(_ret), "=m"(*p)
        : "0"(1), "m"(*p)
        : "memory");
#endif
    return _ret;

}

static inline unsigned char _increment(volatile unsigned char* p)
{
    unsigned char _ret = 0;
#ifdef __WINDOWS__
#ifndef _WIN64
    __asm
    {
        mov ecx, p
        mov al, 1
        lock xadd[ecx], al
        inc al
        mov _ret, al
    }
#else
    for (;;) {
        unsigned char pv = *p;
        if (pv == _InterlockedCompareExchange8((char*)p, pv + 1, pv)) {
            _ret = pv + 1;
            break;
        }
    }
#endif
#else
    /* xaddl or xaddq */
    __asm__ __volatile__("lock; xaddb %0, %1;inc %0"
        :"=r"(_ret), "=m"(*p)
        : "0"(1), "m"(*p)
        : "memory");
#endif
    return _ret;

}

static inline unsigned int* _decrement(unsigned int* volatile* p)
{
    unsigned int* _ret = 0;
#ifdef __WINDOWS__
#ifndef _WIN64
    _ret = (unsigned int*)_InterlockedDecrement((long*)p);
#else
    _ret = (unsigned int*)_InterlockedDecrement64((__int64*)p);
#endif
#else
    /* xaddl or xaddq */
    __asm__ __volatile__("lock; xadd" REG_OPL " %0, %1;dec %0"
        :"=r"(_ret), "=m"(*p)
        : "0"(-1), "m"(*p)
        : "memory");
#endif
    return _ret;
}

static inline unsigned long _decrement(volatile unsigned long* p)
{
    unsigned long _ret = 0;
#ifdef __WINDOWS__
    _ret = (unsigned long)_InterlockedDecrement((long*)p);
#else
    /* xaddl or xaddq */
    __asm__ __volatile__("lock; xadd" REG_OPL " %0, %1;dec %0"
        :"=r"(_ret), "=m"(*p)
        : "0"(-1), "m"(*p)
        : "memory");
#endif
    return _ret;
}

static inline unsigned int _decrement(volatile unsigned int* p)
{
    unsigned int _ret = 0;
#ifdef __WINDOWS__
    _ret = (unsigned int)_InterlockedDecrement((long*)p);
#else
    /* xaddl or xaddq */
    __asm__ __volatile__("lock; xadd" REG_OP32 " %0, %1;dec %0"
        :"=r"(_ret), "=m"(*p)
        : "0"(-1), "m"(*p)
        : "memory");
#endif
    return _ret;
}

static inline unsigned short _decrement(volatile unsigned short* p)
{
    unsigned short _ret = 0;
#ifdef __WINDOWS__
    _ret = (unsigned short)_InterlockedDecrement16((short*)p);
#else
    /* xaddl or xaddq */
    __asm__ __volatile__("lock; xaddw %0, %1;dec %0"
        :"=r"(_ret), "=m"(*p)
        : "0"(-1), "m"(*p)
        : "memory");
#endif
    return _ret;
}

static inline unsigned char _decrement(volatile unsigned char* p)
{
    unsigned char _ret = 0;
#ifdef __WINDOWS__
#ifndef _WIN64
    __asm
    {
        mov ecx, p
        mov al, -1
        lock xadd[ecx], al
        dec al
        mov _ret, al
    }
#else
    for (;;) {
        unsigned char pv = *p;
        if (pv == _InterlockedCompareExchange8((char*)p, (char)(pv - 1), (char)pv)) {
            _ret = pv - 1;
            break;
        }
    }
#endif
#else
    /* xaddl or xaddq */
    __asm__ __volatile__("lock; xaddb %0, %1;dec %0"
        :"=r"(_ret), "=m"(*p)
        : "0"(-1), "m"(*p)
        : "memory");
#endif
    return _ret;
}

static inline
unsigned int* _fetch_and_add(unsigned int* volatile* p,
    unsigned int* _add)
{
    unsigned int* _ret = 0;
#ifdef __WINDOWS__
#ifndef _WIN64
    __asm
    {
        mov eax, _add
        mov ecx, p
        lock xadd[ecx], eax
        mov _ret, eax
    }
#else
    for (;;) {
        void* pv = (void*)*p;
        void* add = (void*)_add;
        __int64 nv = (__int64)pv + (__int64)add;
        if (pv == _InterlockedCompareExchangePointer((void**)p, add, pv)) {
            _ret = (unsigned int*)nv;
            break;
        }
    }
#endif
#else
    /* xaddl or xaddq */
    __asm__ __volatile__("lock; xadd" REG_OPL " %0, %1" :
    "=r" (_ret), "=m" (*p) : "0" (_add), "m" (*p)
        : "memory");
#endif
    return _ret;
}

static inline
unsigned char _fetch_and_add(volatile unsigned char* p,
    unsigned char _add)
{
    unsigned char _ret = 0;
#ifdef __WINDOWS__
#ifndef _WIN64
    __asm
    {
        mov al, _add
        mov ecx, p
        lock xadd[ecx], al
        mov _ret, al
    }
#else
    for (;;) {
        unsigned char pv = *p;
        if (pv == _InterlockedCompareExchange8((char*)p, (char)(pv + _add), (char)pv)) {
            _ret = (unsigned char)(pv + _add);
            break;
        }
    }
#endif
#else
    __asm__ __volatile__("lock; xaddb %0, %1" :
    "=r" (_ret), "=m" (*p) : "0" (_add), "m" (*p)
        : "memory");
#endif
    return _ret;
}

static inline
unsigned short _fetch_and_add(volatile unsigned short* p,
    unsigned short _add)
{
    unsigned short _ret = 0;
#ifdef __WINDOWS__
#ifndef _WIN64
    __asm
    {
        mov ax, _add
        mov ecx, p
        lock xadd[ecx], ax
        mov _ret, ax
    }
#else
    for (;;) {
        unsigned short pv = *p;
        if (pv == _InterlockedCompareExchange16((short*)p, (short)(pv + _add), (short)pv)) {
            _ret = (unsigned short)(pv + _add);
            break;
        }
    }
#endif
#else
    __asm__ __volatile__("lock; xaddw %0, %1" :
    "=r" (_ret), "=m" (*p) : "0" (_add), "m" (*p)
        : "memory");
#endif
    return _ret;
}

static inline
unsigned int _fetch_and_add(volatile unsigned int* p,
    unsigned int _add)
{
    unsigned int _ret = 0;
#ifdef __WINDOWS__
#ifndef _WIN64
    __asm
    {
        mov eax, _add
        mov ecx, p
        lock xadd[ecx], eax
        mov _ret, eax
    }
#else
    for (;;) {
        unsigned int pv = *p;
        if (pv == (unsigned int)_InterlockedCompareExchange((long*)p, (long)(pv + _add), (long)pv)) {
            _ret = (unsigned int)(pv + _add);
            break;
        }
    }
#endif
#else
    /* xaddl */
    __asm__ __volatile__("lock; xadd" REG_OP32 " %0, %1" :
    "=r" (_ret), "=m" (*p) : "0" (_add), "m" (*p)
        : "memory");
#endif
    return _ret;
}

static inline
unsigned long _fetch_and_add(volatile unsigned long* p,
    unsigned long _add)
{
    unsigned long _ret = 0;
#ifdef __WINDOWS__
#ifndef _WIN64
    __asm
    {
        mov eax, _add
        mov ecx, p
        lock xadd[ecx], eax
        mov _ret, eax
    }
#else
    for (;;) {
        unsigned long pv = *p;
        if (pv == (unsigned long)_InterlockedCompareExchange((long*)p, (long)(pv + _add), (long)pv)) {
            _ret = (unsigned long)(pv + _add);
            break;
        }
    }
#endif
#else
    /* xaddl */
    __asm__ __volatile__("lock; xadd" REG_OPL " %0, %1" :
    "=r" (_ret), "=m" (*p) : "0" (_add), "m" (*p)
        : "memory");
#endif
    return _ret;
}

static inline
char _compare_and_swap(unsigned int* volatile* p,
    unsigned int* val_old,
    unsigned int* val_new)
{
    char _ret;
#ifdef __WINDOWS__
#ifndef _WIN64
    __asm
    {
        mov eax, val_old
        mov ecx, p
        mov edx, val_new
        lock cmpxchg DWORD PTR[ecx], edx
        setz _ret
    }
#else
    void** ptr = (void**)p;
    void* oldV = (void*)val_old;
    void* newV = (void*)val_new;
    if (oldV == _InterlockedCompareExchangePointer(ptr, newV, oldV)) {
        _ret = 0;
    }
    else {
        _ret = '\xff';
    };
#endif
#else
    /* cmpxchgq or cmpxchgl */
    __asm__ __volatile__("lock; cmpxchg" REG_OPL " %3, %0; setz %1"
        : "=m"(*p), "=q"(_ret)
        : "m"(*p), "r" (val_new), "a"(val_old) : "memory");
#endif
    return _ret;
}

static inline
char _compare_and_swap(volatile unsigned long* p, unsigned long val_old, unsigned long val_new)
{
    char _ret;
#ifdef __WINDOWS__
#ifndef _WIN64
    __asm
    {
        mov eax, val_old
        mov ecx, p
        mov edx, val_new
        lock cmpxchg DWORD PTR[ecx], edx
        setz _ret
    }
#else
    long* ptr = (long*)p;
    long oldV = (long)val_old;
    long newV = (long)val_new;
    if (oldV == _InterlockedCompareExchange(ptr, newV, oldV)) {
        _ret = 0;
    }
    else {
        _ret = '\xff';
    };
#endif
#else
    /* cmpxchgq or cmpxchgl */
    __asm__ __volatile__("lock; cmpxchg" REG_OPL " %3, %0; setz %1"
        : "=m"(*p), "=q"(_ret)
        : "m"(*p), "r" (val_new), "a"(val_old) : "memory");
#endif
    return _ret;
}

static inline
char _compare_and_swap(volatile unsigned int* p, unsigned int val_old, unsigned int val_new)
{
    char _ret;
#ifdef __WINDOWS__
#ifndef _WIN64
    __asm
    {
        mov eax, val_old
        mov ecx, p
        mov edx, val_new
        lock cmpxchg DWORD PTR[ecx], edx
        setz _ret
    }
#else
    long* ptr = (long*)p;
    long oldV = (long)val_old;
    long newV = (long)val_new;
    if (oldV == _InterlockedCompareExchange(ptr, newV, oldV)) {
        _ret = 0;
    }
    else {
        _ret = '\xff';
    };
#endif
#else
    /* cmpxchgq or cmpxchgl */
    __asm__ __volatile__("lock; cmpxchg" REG_OP32 " %3, %0; setz %1"
        : "=m"(*p), "=q"(_ret)
        : "m"(*p), "r" (val_new), "a"(val_old) : "memory");
#endif
    return _ret;
}

static inline
char _compare_and_swap(volatile unsigned short* p, unsigned short val_old, unsigned short val_new)
{
    char _ret;
#ifdef __WINDOWS__
#ifndef _WIN64
    __asm
    {
        mov ax, val_old
        mov ecx, p
        mov dx, val_new
        lock cmpxchg WORD PTR[ecx], dx
        setz _ret
    }
#else
    short* ptr = (short*)p;
    short oldV = (short)val_old;
    short newV = (short)val_new;
    if (oldV == _InterlockedCompareExchange16(ptr, newV, oldV)) {
        _ret = 0;
    }
    else {
        _ret = '\xff';
    };
#endif
#else
    /* cmpxchgq or cmpxchgl */
    __asm__ __volatile__("lock; cmpxchgw %3, %0; setz %1"
        : "=m"(*p), "=q"(_ret)
        : "m"(*p), "r" (val_new), "a"(val_old) : "memory");
#endif
    return _ret;
}

static inline
char _compare_and_swap(volatile unsigned char* p, unsigned char val_old, unsigned char val_new)
{
    char _ret;
#ifdef __WINDOWS__
#ifndef _WIN64
    __asm
    {
        mov al, val_old
        mov ecx, p
        mov dl, val_new
        lock cmpxchg BYTE PTR[ecx], dl
        setz _ret
    }
#else
    char* ptr = (char*)p;
    char oldV = (char)val_old;
    char newV = (char)val_new;
    if (oldV == _InterlockedCompareExchange8(ptr, newV, oldV)) {
        _ret = 0;
    }
    else {
        _ret = '\xff';
    };
#endif
#else
    /* cmpxchgq or cmpxchgl */
    __asm__ __volatile__("lock; cmpxchgb %3, %0; setz %1"
        : "=m"(*p), "=q"(_ret)
        : "m"(*p), "r" (val_new), "a"(val_old) : "memory");
#endif
    return _ret;
}

static inline
char _compare_double_and_swap_double(volatile unsigned int* p,
    unsigned int val_old_m1, unsigned int val_old_m2,
    unsigned int val_new_m1, unsigned int val_new_m2)
{
    char _ret;
#ifdef __WINDOWS__
#ifndef _WIN64
    __asm
    {
        mov eax, val_old_m1;
        mov edx, val_old_m2;
        mov ebx, val_new_m1;
        mov ecx, val_new_m2;
        mov esi, p;
        //lock CMPXCHG8B [esi] is equivalent to the following except
        //that it's atomic:
        //ZeroFlag = (edx:eax == *esi);
        //if (ZeroFlag) *esi = ecx:ebx;
        //else edx:eax = *esi;
        lock CMPXCHG8B[esi];
        setz _ret
    }
#else
    __int64* ptr = (__int64*)p;
    __int64  oldV = (__int64)val_old_m1 + ((__int64)val_old_m2 << 32);
    __int64  newV = (__int64)val_new_m1 + ((__int64)val_new_m2 << 32);
    if (oldV == _InterlockedCompareExchange64(ptr, newV, oldV)) {
        _ret = 0;
    }
    else {
        _ret = '\xff';
    };
#endif
#else
    __asm__ __volatile__("lock; cmpxchg8b %0; setz %1"
        : "=m"(*p), "=q"(_ret)
        : "m"(*p), "d" (val_old_m2), "a" (val_old_m1),
        "c" (val_new_m2), "b" (val_new_m1) : "memory");
#endif
    return _ret;
}

static inline
char _compare_double_and_swap_double(volatile unsigned long* p,
    unsigned long val_old_m1, unsigned long val_old_m2,
    unsigned long val_new_m1, unsigned long val_new_m2)
{
    char _ret;
#ifdef __WINDOWS__
#ifndef _WIN64
    __asm
    {
        mov eax, val_old_m1;
        mov edx, val_old_m2;
        mov ebx, val_new_m1;
        mov ecx, val_new_m2;
        mov esi, p;
        //lock CMPXCHG8B [esi] is equivalent to the following except
        //that it's atomic:
        //ZeroFlag = (edx:eax == *esi);
        //if (ZeroFlag) *esi = ecx:ebx;
        //else edx:eax = *esi;
        lock CMPXCHG8B[esi];
        setz _ret
    }
#else
    __int64* ptr = (__int64*)p;
    __int64  oldV = (__int64)val_old_m1 + ((__int64)val_old_m2 << 32);
    __int64  newV = (__int64)val_new_m1 + ((__int64)val_new_m2 << 32);
    if (oldV == _InterlockedCompareExchange64(ptr, newV, oldV)) {
        _ret = 0;
    }
    else {
        _ret = '\xff';
    };
#endif
#else
    //64bit cmpxchg
    //mov %0,%%rsi;lock; .byte 0x48,0x0f,0xc7,0x0e;
    //mov %0,%%rsi;lock; cmpxchg16b (%%rsi);
    __asm__ __volatile__("mov %0,%%rsi;lock; .byte 0x48,0x0f,0xc7,0x0e; setz %1"
        : "=m"(p), "=q"(_ret)
        : "m"(p), "d" (val_old_m2), "a" (val_old_m1),
        "c" (val_new_m2), "b" (val_new_m1) : "memory", "%rsi");
#endif
    return _ret;
}

static inline
unsigned int* _xchg(unsigned int* volatile* p, unsigned int* val)
{
    unsigned int* _ret = 0;
#ifdef __WINDOWS__
#ifndef _WIN64
    __asm
    {
        mov eax, val
        mov ecx, p
        xchg[ecx], eax
        mov _ret, eax
    }
#else
    _ret = (unsigned int*)_InterlockedExchangePointer((void**)p, (void*)val);
#endif
#else
    /* xaddl */
    __asm__ __volatile__("xchg" REG_OPL " %0, %1" :
    "=r" (_ret), "=m" (*p) : "0" (val), "m" (*p)
        : "memory");
#endif
    return _ret;
}

static inline
unsigned long _xchg(volatile unsigned long* p, unsigned long val)
{
    unsigned long _ret = 0;
#ifdef __WINDOWS__
#ifndef _WIN64
    __asm
    {
        mov eax, val
        mov ecx, p
        xchg[ecx], eax
        mov _ret, eax
    }
#else
    _ret = (unsigned long)_InterlockedExchange((long*)p, (long)val);
#endif
#else
    /* xaddl */
    __asm__ __volatile__("xchg" REG_OPL " %0, %1" :
    "=r" (_ret), "=m" (*p) : "0" (val), "m" (*p)
        : "memory");
#endif
    return _ret;
}

static inline
unsigned int _xchg(volatile unsigned int* p, unsigned int val)
{
    unsigned int _ret = 0;
#ifdef __WINDOWS__
#ifndef _WIN64
    __asm
    {
        mov eax, val
        mov ecx, p
        xchg[ecx], eax
        mov _ret, eax
    }
#else
    _ret = (unsigned int)_InterlockedExchange((long*)p, (long)val);
#endif
#else
    /* xaddl */
    __asm__ __volatile__("xchg" REG_OP32 " %0, %1" :
    "=r" (_ret), "=m" (*p) : "0" (val), "m" (*p)
        : "memory");
#endif
    return _ret;
}

static inline
unsigned short _xchg(volatile unsigned short* p, unsigned short val)
{
    unsigned short _ret = 0;
#ifdef __WINDOWS__
#ifndef _WIN64
    __asm
    {
        mov ax, val
        mov ecx, p
        xchg[ecx], ax
        mov _ret, ax
    }
#else
    _ret = (unsigned short)_InterlockedExchange16((short*)p, (short)val);
#endif
#else
    /* xaddl */
    __asm__ __volatile__("xchgw %0, %1" :
    "=r" (_ret), "=m" (*p) : "0" (val), "m" (*p)
        : "memory");
#endif
    return _ret;
}

static inline
unsigned char _xchg(volatile unsigned char* p, unsigned char val)
{
    unsigned char _ret = 0;
#ifdef __WINDOWS__
#ifndef _WIN64
    __asm
    {
        mov al, val
        mov ecx, p
        xchg[ecx], al
        mov _ret, al
    }
#else
    _ret = (unsigned char)_InterlockedExchange8((char*)p, (char)val);
#endif
#else
    /* xaddl */
    __asm__ __volatile__("xchgb %0, %1" :
    "=r" (_ret), "=m" (*p) : "0" (val), "m" (*p)
        : "memory");
#endif
    return _ret;
}

static inline
void _pause()
{
#ifdef __WINDOWS__
#ifndef _WIN64
    __asm pause
#else
    __nop();
#endif
#else
    __asm__ __volatile__("pause":::);
#endif
}

#endif


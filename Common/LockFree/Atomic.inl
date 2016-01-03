#ifndef ATOMIC_INL__
#define ATOMIC_INL__

static inline unsigned int* _increment(unsigned int* volatile *p)
{
	unsigned int* _ret=0;
#ifdef __WINDOWS__
	__asm
	{
		mov ecx,p
		mov eax,1
		lock xadd [ecx],eax
		inc eax
		mov _ret,eax
	}
#else
	/* xaddl or xaddq */
	__asm__ __volatile__ ("lock; xadd"REG_OPL" %0, %1;inc %0"
		:"=r"(_ret),"=m"(*p)
		:"0"(1),"m"(*p)
		:"memory");
#endif
	return _ret;

}

static inline unsigned long _increment(volatile unsigned long *p)
{
	unsigned long _ret=0;
#ifdef __WINDOWS__
	__asm
	{
		mov ecx,p
		mov eax,1
		lock xadd [ecx],eax
		inc eax
		mov _ret,eax
	}
#else
	/* xaddl or xaddq */
	__asm__ __volatile__ ("lock; xadd"REG_OPL" %0, %1;inc %0"
		:"=r"(_ret),"=m"(*p)
		:"0"(1),"m"(*p)
		:"memory");
#endif
	return _ret;
}

static inline unsigned int _increment(volatile unsigned int *p)
{
	unsigned int _ret=0;
#ifdef __WINDOWS__
	__asm
	{
		mov ecx,p
		mov eax,1
		lock xadd [ecx],eax
		inc eax
		mov _ret,eax
	}
#else
	/* xaddl or xaddq */
	__asm__ __volatile__ ("lock; xadd"REG_OP32" %0, %1;inc %0"
		:"=r"(_ret),"=m"(*p)
		:"0"(1),"m"(*p)
		:"memory");
#endif
	return _ret;

}

static inline unsigned short _increment(volatile unsigned short *p)
{
	unsigned short _ret=0;
#ifdef __WINDOWS__
	__asm
	{
		mov ecx,p
		mov ax,1
		lock xadd [ecx],ax
		inc ax
		mov _ret,ax
	}
#else
	/* xaddl or xaddq */
	__asm__ __volatile__ ("lock; xaddw %0, %1;inc %0"
		:"=r"(_ret),"=m"(*p)
		:"0"(1),"m"(*p)
		:"memory");
#endif
	return _ret;

}

static inline unsigned char _increment(volatile unsigned char *p)
{
	unsigned char _ret=0;
#ifdef __WINDOWS__
	__asm
	{
		mov ecx,p
		mov al,1
		lock xadd [ecx],al
		inc al
		mov _ret,al
	}
#else
	/* xaddl or xaddq */
	__asm__ __volatile__ ("lock; xaddb %0, %1;inc %0"
		:"=r"(_ret),"=m"(*p)
		:"0"(1),"m"(*p)
		:"memory");
#endif
	return _ret;

}

static inline unsigned int* _decrement(unsigned int* volatile *p)
{
	unsigned int* _ret=0;
#ifdef __WINDOWS__
	__asm
	{
		mov ecx,p
		mov eax,-1
		lock xadd [ecx],eax
		dec eax
		mov _ret,eax
	}
#else
	/* xaddl or xaddq */
	__asm__ __volatile__ ("lock; xadd"REG_OPL" %0, %1;dec %0"
		:"=r"(_ret),"=m"(*p)
		:"0"(-1),"m"(*p)
		:"memory");
#endif
	return _ret;
}

static inline unsigned long _decrement(volatile unsigned long *p)
{
	unsigned long _ret=0;
#ifdef __WINDOWS__
	__asm
	{
		mov ecx,p
		mov eax,-1
		lock xadd [ecx],eax
		dec eax
		mov _ret,eax
	}
#else
	/* xaddl or xaddq */
	__asm__ __volatile__ ("lock; xadd"REG_OPL" %0, %1;dec %0"
		:"=r"(_ret),"=m"(*p)
		:"0"(-1),"m"(*p)
		:"memory");
#endif
	return _ret;
}

static inline unsigned int _decrement(volatile unsigned int *p)
{
	unsigned int _ret=0;
#ifdef __WINDOWS__
	__asm
	{
		mov ecx,p
		mov eax,-1
		lock xadd [ecx],eax
		dec eax
		mov _ret,eax
	}
#else
	/* xaddl or xaddq */
	__asm__ __volatile__ ("lock; xadd"REG_OP32" %0, %1;dec %0"
		:"=r"(_ret),"=m"(*p)
		:"0"(-1),"m"(*p)
		:"memory");
#endif
	return _ret;
}

static inline unsigned short _decrement(volatile unsigned short *p)
{
	unsigned short _ret=0;
#ifdef __WINDOWS__
	__asm
	{
		mov ecx,p
		mov ax,-1
		lock xadd [ecx],ax
		dec ax
		mov _ret,ax
	}
#else
	/* xaddl or xaddq */
	__asm__ __volatile__ ("lock; xaddw %0, %1;dec %0"
		:"=r"(_ret),"=m"(*p)
		:"0"(-1),"m"(*p)
		:"memory");
#endif
	return _ret;
}

static inline unsigned char _decrement(volatile unsigned char *p)
{
	unsigned char _ret=0;
#ifdef __WINDOWS__
	__asm
	{
		mov ecx,p
		mov al,-1
		lock xadd [ecx],al
		dec al
		mov _ret,al
	}
#else
	/* xaddl or xaddq */
	__asm__ __volatile__ ("lock; xaddb %0, %1;dec %0"
		:"=r"(_ret),"=m"(*p)
		:"0"(-1),"m"(*p)
		:"memory");
#endif
	return _ret;
}

static inline 
unsigned int* _fetch_and_add(unsigned int* volatile *p, 
	unsigned int* _add)
{
	unsigned int* _ret=0;
#ifdef __WINDOWS__
	__asm
	{
		mov eax,_add
		mov ecx,p
		lock xadd [ecx],eax
		mov _ret,eax
	}
#else
	/* xaddl or xaddq */
	__asm__ __volatile__ ("lock; xadd"REG_OPL" %0, %1" :
			"=r" (_ret), "=m" (*p) : "0" (_add), "m" (*p)
			: "memory");
#endif
	return _ret;
}

static inline 
unsigned char _fetch_and_add(volatile unsigned char *p, 
	unsigned char _add)
{
	unsigned char _ret=0;
#ifdef __WINDOWS__
	__asm
	{
		mov al,_add
		mov ecx,p
		lock xadd [ecx],al
		mov _ret,al
	}
#else
	__asm__ __volatile__ ("lock; xaddb %0, %1" :
			"=r" (_ret), "=m" (*p) : "0" (_add), "m" (*p)
			: "memory");
#endif
	return _ret;
}

static inline 
unsigned short _fetch_and_add(volatile unsigned short *p, 
	unsigned short _add)
{
	unsigned short _ret=0;
#ifdef __WINDOWS__
	__asm
	{
		mov ax,_add
		mov ecx,p
		lock xadd [ecx],ax
		mov _ret,ax
	}
#else
	__asm__ __volatile__ ("lock; xaddw %0, %1" :
			"=r" (_ret), "=m" (*p) : "0" (_add), "m" (*p)
			: "memory");
#endif
	return _ret;
}

static inline 
unsigned int _fetch_and_add(volatile unsigned int *p, 
	unsigned int _add)
{
	unsigned int _ret=0;
#ifdef __WINDOWS__
	__asm
	{
		mov eax,_add
		mov ecx,p
		lock xadd [ecx],eax
		mov _ret,eax
	}
#else
	/* xaddl */
	__asm__ __volatile__ ("lock; xadd"REG_OP32" %0, %1" :
			"=r" (_ret), "=m" (*p) : "0" (_add), "m" (*p)
			: "memory");
#endif
	return _ret;
}

static inline 
unsigned long _fetch_and_add(volatile unsigned long *p, 
							unsigned long _add)
{
	unsigned long _ret=0;
#ifdef __WINDOWS__
	__asm
	{
		mov eax,_add
		mov ecx,p
		lock xadd [ecx],eax
		mov _ret,eax
	}
#else
	/* xaddl */
	__asm__ __volatile__ ("lock; xadd"REG_OPL" %0, %1" :
	"=r" (_ret), "=m" (*p) : "0" (_add), "m" (*p)
		: "memory");
#endif
	return _ret;
}

static inline 
char _compare_and_swap(unsigned int* volatile *p, 
					   unsigned int* val_old, 
					   unsigned int* val_new)
{
	char _ret;
#ifdef __WINDOWS__
	__asm
	{
		mov eax, val_old
		mov ecx, p
		mov edx, val_new
		lock cmpxchg DWORD PTR [ecx], edx
		setz _ret
	}
#else
	/* cmpxchgq or cmpxchgl */
	__asm__ __volatile__("lock; cmpxchg"REG_OPL" %3, %0; setz %1"
		: "=m"(*p), "=q"(_ret)
		: "m"(*p), "r" (val_new), "a"(val_old) : "memory");
#endif
	return _ret;
}

static inline
char _compare_and_swap(volatile unsigned long *p, unsigned long val_old, unsigned long val_new)
{
	char _ret;
#ifdef __WINDOWS__
	__asm
	{
		mov eax, val_old
		mov ecx, p
		mov edx, val_new
		lock cmpxchg DWORD PTR [ecx], edx
		setz _ret
	}
#else
	/* cmpxchgq or cmpxchgl */
	__asm__ __volatile__("lock; cmpxchg"REG_OPL" %3, %0; setz %1"
		: "=m"(*p), "=q"(_ret)
		: "m"(*p), "r" (val_new), "a"(val_old) : "memory");
#endif
	return _ret;
}

static inline
char _compare_and_swap(volatile unsigned int *p, unsigned int val_old, unsigned int val_new)
{
	char _ret;
#ifdef __WINDOWS__
	__asm
	{
		mov eax, val_old
			mov ecx, p
			mov edx, val_new
			lock cmpxchg DWORD PTR [ecx], edx
			setz _ret
	}
#else
	/* cmpxchgq or cmpxchgl */
	__asm__ __volatile__("lock; cmpxchg"REG_OP32" %3, %0; setz %1"
		: "=m"(*p), "=q"(_ret)
		: "m"(*p), "r" (val_new), "a"(val_old) : "memory");
#endif
	return _ret;
}

static inline
char _compare_and_swap(volatile unsigned short *p, unsigned short val_old, unsigned short val_new)
{
	char _ret;
#ifdef __WINDOWS__
	__asm
	{
		mov ax, val_old
		mov ecx, p
		mov dx, val_new
		lock cmpxchg WORD PTR [ecx], dx
		setz _ret
	}
#else
	/* cmpxchgq or cmpxchgl */
	__asm__ __volatile__("lock; cmpxchgw %3, %0; setz %1"
		: "=m"(*p), "=q"(_ret)
		: "m"(*p), "r" (val_new), "a"(val_old) : "memory");
#endif
	return _ret;
}

static inline
char _compare_and_swap(volatile unsigned char *p, unsigned char val_old, unsigned char val_new)
{
	char _ret;
#ifdef __WINDOWS__
	__asm
	{
		mov al, val_old
		mov ecx, p
		mov dl, val_new
		lock cmpxchg BYTE PTR [ecx], dl
		setz _ret
	}
#else
	/* cmpxchgq or cmpxchgl */
	__asm__ __volatile__("lock; cmpxchgb %3, %0; setz %1"
		: "=m"(*p), "=q"(_ret)
		: "m"(*p), "r" (val_new), "a"(val_old) : "memory");
#endif
	return _ret;
}

//低位为1，高位为2
static inline 
char _compare_double_and_swap_double(volatile unsigned int *p,
	unsigned int val_old_m1, unsigned int val_old_m2,
	unsigned int val_new_m1, unsigned int val_new_m2)
{
	char _ret;
#ifdef __WINDOWS__
	__asm
	{
		mov eax,val_old_m1;
		mov edx,val_old_m2;
		mov ebx,val_new_m1;
		mov ecx,val_new_m2;
		mov esi,p;
		//lock CMPXCHG8B [esi] is equivalent to the following except
		//that it's atomic:
		//ZeroFlag = (edx:eax == *esi);
		//if (ZeroFlag) *esi = ecx:ebx;
		//else edx:eax = *esi;
		lock CMPXCHG8B [esi];	
		setz _ret
	}
#else
	__asm__ __volatile__("lock; cmpxchg8b %0; setz %1"
	    	   : "=m"(*p), "=q"(_ret)
		       : "m"(*p), "d" (val_old_m2), "a" (val_old_m1),
		         "c" (val_new_m2), "b" (val_new_m1) : "memory");
#endif
	return _ret;
}

//低位为1，高位为2
static inline 
char _compare_double_and_swap_double(volatile unsigned long *p,
	unsigned long val_old_m1, unsigned long val_old_m2,
	unsigned long val_new_m1, unsigned long val_new_m2)
{
	char _ret;
#ifdef __WINDOWS__
	__asm
	{
		mov eax,val_old_m1;
		mov edx,val_old_m2;
		mov ebx,val_new_m1;
		mov ecx,val_new_m2;
		mov esi,p;
		//lock CMPXCHG8B [esi] is equivalent to the following except
		//that it's atomic:
		//ZeroFlag = (edx:eax == *esi);
		//if (ZeroFlag) *esi = ecx:ebx;
		//else edx:eax = *esi;
		lock CMPXCHG8B [esi];	
		setz _ret
	}
#else
	//64位cmpxchg16b指令要求目标存储区按16字节对齐！！！
	//mov %0,%%rsi;lock; .byte 0x48,0x0f,0xc7,0x0e;
	//mov %0,%%rsi;lock; cmpxchg16b (%%rsi);
	__asm__ __volatile__("mov %0,%%rsi;lock; .byte 0x48,0x0f,0xc7,0x0e; setz %1"
		: "=m"(p), "=q"(_ret)
		: "m"(p), "d" (val_old_m2), "a" (val_old_m1),
		"c" (val_new_m2), "b" (val_new_m1) : "memory","%rsi");
#endif
	return _ret;
}

static inline
unsigned int* _xchg(unsigned int* volatile *p,unsigned int* val)
{
	unsigned int* _ret=0;
#ifdef __WINDOWS__
	__asm
	{
		mov eax,val
		mov ecx,p
		xchg [ecx],eax
		mov _ret,eax
	}
#else
	/* xaddl */
	__asm__ __volatile__ ("xchg"REG_OPL" %0, %1" :
	"=r" (_ret), "=m" (*p) : "0" (val), "m" (*p)
		: "memory");
#endif
	return _ret;
}

static inline
unsigned long _xchg(volatile unsigned long* p,unsigned long val)
{
	unsigned long _ret=0;
#ifdef __WINDOWS__
	__asm
	{
		mov eax,val
		mov ecx,p
		xchg [ecx],eax
		mov _ret,eax
	}
#else
	/* xaddl */
	__asm__ __volatile__ ("xchg"REG_OPL" %0, %1" :
	"=r" (_ret), "=m" (*p) : "0" (val), "m" (*p)
		: "memory");
#endif
	return _ret;
}

static inline
unsigned int _xchg(volatile unsigned int* p,unsigned int val)
{
	unsigned int _ret=0;
#ifdef __WINDOWS__
	__asm
	{
		mov eax,val
		mov ecx,p
		xchg [ecx],eax
		mov _ret,eax
	}
#else
	/* xaddl */
	__asm__ __volatile__ ("xchg"REG_OP32" %0, %1" :
	"=r" (_ret), "=m" (*p) : "0" (val), "m" (*p)
		: "memory");
#endif
	return _ret;
}

static inline
unsigned short _xchg(volatile unsigned short* p,unsigned short val)
{
	unsigned short _ret=0;
#ifdef __WINDOWS__
	__asm
	{
		mov ax,val
		mov ecx,p
		xchg [ecx],ax
		mov _ret,ax
	}
#else
	/* xaddl */
	__asm__ __volatile__ ("xchgw %0, %1" :
	"=r" (_ret), "=m" (*p) : "0" (val), "m" (*p)
		: "memory");
#endif
	return _ret;
}

static inline
unsigned char _xchg(volatile unsigned char* p,unsigned char val)
{
	unsigned char _ret=0;
#ifdef __WINDOWS__
	__asm
	{
		mov al,val
		mov ecx,p
		xchg [ecx],al
		mov _ret,al
	}
#else
	/* xaddl */
	__asm__ __volatile__ ("xchgb %0, %1" :
	"=r" (_ret), "=m" (*p) : "0" (val), "m" (*p)
		: "memory");
#endif
	return _ret;
}

static inline
void _pause(void)
{
#ifdef __WINDOWS__
	__asm pause
#else
	__asm__ __volatile__("pause":::);
#endif
}

#endif


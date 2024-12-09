	switch (n % 4) {
		/* tail */
	case 0:
		return to;
	case 1:
		asm volatile("movsb"
			     : "=&D"(edi), "=&S"(esi)
			     : "0"(edi), "1"(esi)
			     : "memory");
		return to;
	case 2:
		asm volatile("movsw"
			     : "=&D"(edi), "=&S"(esi)
			     : "0"(edi), "1"(esi)
			     : "memory");
		return to;
	default:
		asm volatile("movsw\n\tmovsb"
			     : "=&D"(edi), "=&S"(esi)
			     : "0"(edi), "1"(esi)
			     : "memory");
		return to;
	}
}

#define __HAVE_ARCH_MEMCPY

#ifdef CONFIG_X86_USE_3DNOW

#include <asm/mmx.h>

/*
 *	This CPU favours 3DNow strongly (eg AMD Athlon)
 */

static inline void *__constant_memcpy3d(void *to, const void *from, size_t len)
{
	if (len < 512)
		return __constant_memcpy(to, from, len);
	return _mmx_memcpy(to, from, len);
}
{
	if (len < 512)
		return __memcpy(to, from, len);
	return _mmx_memcpy(to, from, len);
}

#define memcpy(t, f, n)				\
	(__builtin_constant_p((n))		\
	 ? __constant_memcpy3d((t), (f), (n))	\
	 : __memcpy3d((t), (f), (n)))

#else

/*
 *	No 3D Now!
 */

#ifndef CONFIG_KMEMCHECK

#if (__GNUC__ >= 4)
#define memcpy(t, f, n) __builtin_memcpy(t, f, n)
#else
#define memcpy(t, f, n)				\
	(__builtin_constant_p((n))		\
	 ? __constant_memcpy((t), (f), (n))	\
	 : __memcpy((t), (f), (n)))
#endif
#else
/*
 * kmemcheck becomes very happy if we use the REP instructions unconditionally,
 * because it means that we know both memory operands in advance.
 */
#define memcpy(t, f, n) __memcpy((t), (f), (n))
#endif

#endif

#define __HAVE_ARCH_MEMMOVE
void *memmove(void *dest, const void *src, size_t n);

#define memcmp __builtin_memcmp

#define __HAVE_ARCH_MEMCHR
extern void *memchr(const void *cs, int c, size_t count);

static inline void *__memset_generic(void *s, char c, size_t count)
{
	int d0, d1;
	asm volatile("rep\n\t"
		     "stosb"
		     : "=&c" (d0), "=&D" (d1)
		     : "a" (c), "1" (s), "0" (count)
		     : "memory");
	return s;
}
		switch (count % 4) {
		case 0:
			COMMON("");
			return s;
		case 1:
			COMMON("\n\tstosb");
			return s;
		case 2:
			COMMON("\n\tstosw");
			return s;
		default:
			COMMON("\n\tstosw\n\tstosb");
			return s;
		}
	}

#undef COMMON
}

#define __constant_c_x_memset(s, c, count)			\
	(__builtin_constant_p(count)				\
	 ? __constant_c_and_count_memset((s), (c), (count))	\
	 : __constant_c_memset((s), (c), (count)))

#define __memset(s, c, count)				\
	(__builtin_constant_p(count)			\
	 ? __constant_count_memset((s), (c), (count))	\
	 : __memset_generic((s), (c), (count)))

#define __HAVE_ARCH_MEMSET
#if (__GNUC__ >= 4)
#define memset(s, c, count) __builtin_memset(s, c, count)
#else
#define memset(s, c, count)						\
	(__builtin_constant_p(c)					\
	 ? __constant_c_x_memset((s), (0x01010101UL * (unsigned char)(c)), \
				 (count))				\
	 : __memset((s), (c), (count)))
#endif

/*
 * find the first occurrence of byte 'c', or 1 past the area if none
 */
#define __HAVE_ARCH_MEMSCAN
extern void *memscan(void *addr, int c, size_t size);

#endif /* __KERNEL__ */

#endif /* _ASM_X86_STRING_32_H */
		switch (count % 4) {
		case 0:
			COMMON("");
			return s;
		case 1:
			COMMON("\n\tstosb");
			return s;
		case 2:
			COMMON("\n\tstosw");
			return s;
		default:
			COMMON("\n\tstosw\n\tstosb");
			return s;
		}
	}

#undef COMMON
}

#define __constant_c_x_memset(s, c, count)			\
	(__builtin_constant_p(count)				\
	 ? __constant_c_and_count_memset((s), (c), (count))	\
	 : __constant_c_memset((s), (c), (count)))

#define __memset(s, c, count)				\
	(__builtin_constant_p(count)			\
	 ? __constant_count_memset((s), (c), (count))	\
	 : __memset_generic((s), (c), (count)))

#define __HAVE_ARCH_MEMSET
#if (__GNUC__ >= 4)
#define memset(s, c, count) __builtin_memset(s, c, count)
#else
#define memset(s, c, count)						\
	(__builtin_constant_p(c)					\
	 ? __constant_c_x_memset((s), (0x01010101UL * (unsigned char)(c)), \
				 (count))				\
	 : __memset((s), (c), (count)))
#endif

/*
 * find the first occurrence of byte 'c', or 1 past the area if none
 */
#define __HAVE_ARCH_MEMSCAN
extern void *memscan(void *addr, int c, size_t size);

#endif /* __KERNEL__ */

#endif /* _ASM_X86_STRING_32_H */
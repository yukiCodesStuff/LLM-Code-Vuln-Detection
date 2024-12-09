}

#define __HAVE_ARCH_MEMCPY

#ifdef CONFIG_X86_USE_3DNOW

#include <asm/mmx.h>

#endif

#endif

#define __HAVE_ARCH_MEMMOVE
void *memmove(void *dest, const void *src, size_t n);

#define memcmp __builtin_memcmp

#define __HAVE_ARCH_MEMCHR
extern void *memchr(const void *cs, int c, size_t count);

	 : __memset_generic((s), (c), (count)))

#define __HAVE_ARCH_MEMSET
#if (__GNUC__ >= 4)
#define memset(s, c, count) __builtin_memset(s, c, count)
#else
#define memset(s, c, count)						\
				 (count))				\
	 : __memset((s), (c), (count)))
#endif

/*
 * find the first occurrence of byte 'c', or 1 past the area if none
 */
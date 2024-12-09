}

#define __HAVE_ARCH_MEMCPY
extern void *memcpy(void *, const void *, size_t);

#ifndef CONFIG_FORTIFY_SOURCE
#ifdef CONFIG_X86_USE_3DNOW

#include <asm/mmx.h>

#endif

#endif
#endif /* !CONFIG_FORTIFY_SOURCE */

#define __HAVE_ARCH_MEMMOVE
void *memmove(void *dest, const void *src, size_t n);

extern int memcmp(const void *, const void *, size_t);
#ifndef CONFIG_FORTIFY_SOURCE
#define memcmp __builtin_memcmp
#endif

#define __HAVE_ARCH_MEMCHR
extern void *memchr(const void *cs, int c, size_t count);

	 : __memset_generic((s), (c), (count)))

#define __HAVE_ARCH_MEMSET
extern void *memset(void *, int, size_t);
#ifndef CONFIG_FORTIFY_SOURCE
#if (__GNUC__ >= 4)
#define memset(s, c, count) __builtin_memset(s, c, count)
#else
#define memset(s, c, count)						\
				 (count))				\
	 : __memset((s), (c), (count)))
#endif
#endif /* !CONFIG_FORTIFY_SOURCE */

/*
 * find the first occurrence of byte 'c', or 1 past the area if none
 */
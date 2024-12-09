/* SPDX-License-Identifier: GPL-2.0 */
#ifndef _LINUX_FORTIFY_STRING_H_
#define _LINUX_FORTIFY_STRING_H_

#define __FORTIFY_INLINE extern __always_inline __attribute__((gnu_inline))
#define __RENAME(x) __asm__(#x)

void fortify_panic(const char *name) __noreturn __cold;
void __read_overflow(void) __compiletime_error("detected read beyond size of object (1st parameter)");
void __read_overflow2(void) __compiletime_error("detected read beyond size of object (2nd parameter)");
void __write_overflow(void) __compiletime_error("detected write beyond size of object (1st parameter)");

#define __compiletime_strlen(p)					\
({								\
	unsigned char *__p = (unsigned char *)(p);		\
	size_t __ret = (size_t)-1;				\
	size_t __p_size = __builtin_object_size(p, 1);		\
	if (__p_size != (size_t)-1) {				\
		size_t __p_len = __p_size - 1;			\
		if (__builtin_constant_p(__p[__p_len]) &&	\
		    __p[__p_len] == '\0')			\
			__ret = __builtin_strlen(__p);		\
	}							\
	__ret;							\
})

#if defined(CONFIG_KASAN_GENERIC) || defined(CONFIG_KASAN_SW_TAGS)
extern void *__underlying_memchr(const void *p, int c, __kernel_size_t size) __RENAME(memchr);
extern int __underlying_memcmp(const void *p, const void *q, __kernel_size_t size) __RENAME(memcmp);
extern void *__underlying_memcpy(void *p, const void *q, __kernel_size_t size) __RENAME(memcpy);
extern void *__underlying_memmove(void *p, const void *q, __kernel_size_t size) __RENAME(memmove);
extern void *__underlying_memset(void *p, int c, __kernel_size_t size) __RENAME(memset);
extern char *__underlying_strcat(char *p, const char *q) __RENAME(strcat);
extern char *__underlying_strcpy(char *p, const char *q) __RENAME(strcpy);
extern __kernel_size_t __underlying_strlen(const char *p) __RENAME(strlen);
extern char *__underlying_strncat(char *p, const char *q, __kernel_size_t count) __RENAME(strncat);
extern char *__underlying_strncpy(char *p, const char *q, __kernel_size_t size) __RENAME(strncpy);
#else
#define __underlying_memchr	__builtin_memchr
#define __underlying_memcmp	__builtin_memcmp
#define __underlying_memcpy	__builtin_memcpy
#define __underlying_memmove	__builtin_memmove
#define __underlying_memset	__builtin_memset
#define __underlying_strcat	__builtin_strcat
#define __underlying_strcpy	__builtin_strcpy
#define __underlying_strlen	__builtin_strlen
#define __underlying_strncat	__builtin_strncat
#define __underlying_strncpy	__builtin_strncpy
#endif

__FORTIFY_INLINE char *strncpy(char *p, const char *q, __kernel_size_t size)
{
	size_t p_size = __builtin_object_size(p, 1);

	if (__builtin_constant_p(size) && p_size < size)
		__write_overflow();
	if (p_size < size)
		fortify_panic(__func__);
	return __underlying_strncpy(p, q, size);
}
{
	size_t p_size = __builtin_object_size(p, 0);

	if (__builtin_constant_p(size) && p_size < size)
		__write_overflow();
	if (p_size < size)
		fortify_panic(__func__);
	return __underlying_memset(p, c, size);
}

__FORTIFY_INLINE void *memcpy(void *p, const void *q, __kernel_size_t size)
{
	size_t p_size = __builtin_object_size(p, 0);
	size_t q_size = __builtin_object_size(q, 0);

	if (__builtin_constant_p(size)) {
		if (p_size < size)
			__write_overflow();
		if (q_size < size)
			__read_overflow2();
	}
{
	size_t p_size = __builtin_object_size(p, 0);

	if (__builtin_constant_p(size) && p_size < size)
		__read_overflow();
	if (p_size < size)
		fortify_panic(__func__);
	return __real_kmemdup(p, size, gfp);
}

/* defined after fortified strlen and memcpy to reuse them */
__FORTIFY_INLINE char *strcpy(char *p, const char *q)
{
	size_t p_size = __builtin_object_size(p, 1);
	size_t q_size = __builtin_object_size(q, 1);
	size_t size;

	if (p_size == (size_t)-1 && q_size == (size_t)-1)
		return __underlying_strcpy(p, q);
	size = strlen(q) + 1;
	/* Compile-time check for const size overflow. */
	if (__builtin_constant_p(size) && p_size < size)
		__write_overflow();
	/* Run-time check for dynamic size overflow. */
	if (p_size < size)
		fortify_panic(__func__);
	memcpy(p, q, size);
	return p;
}

/* Don't use these outside the FORITFY_SOURCE implementation */
#undef __underlying_memchr
#undef __underlying_memcmp
#undef __underlying_memcpy
#undef __underlying_memmove
#undef __underlying_memset
#undef __underlying_strcat
#undef __underlying_strcpy
#undef __underlying_strlen
#undef __underlying_strncat
#undef __underlying_strncpy

#endif /* _LINUX_FORTIFY_STRING_H_ */
{
	size_t p_size = __builtin_object_size(p, 1);
	size_t q_size = __builtin_object_size(q, 1);
	size_t size;

	if (p_size == (size_t)-1 && q_size == (size_t)-1)
		return __underlying_strcpy(p, q);
	size = strlen(q) + 1;
	/* Compile-time check for const size overflow. */
	if (__builtin_constant_p(size) && p_size < size)
		__write_overflow();
	/* Run-time check for dynamic size overflow. */
	if (p_size < size)
		fortify_panic(__func__);
	memcpy(p, q, size);
	return p;
}

/* Don't use these outside the FORITFY_SOURCE implementation */
#undef __underlying_memchr
#undef __underlying_memcmp
#undef __underlying_memcpy
#undef __underlying_memmove
#undef __underlying_memset
#undef __underlying_strcat
#undef __underlying_strcpy
#undef __underlying_strlen
#undef __underlying_strncat
#undef __underlying_strncpy

#endif /* _LINUX_FORTIFY_STRING_H_ */
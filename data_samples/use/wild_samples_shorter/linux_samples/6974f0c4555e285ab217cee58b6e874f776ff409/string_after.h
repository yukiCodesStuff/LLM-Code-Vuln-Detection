	return tail ? tail + 1 : path;
}

#define __FORTIFY_INLINE extern __always_inline __attribute__((gnu_inline))
#define __RENAME(x) __asm__(#x)

void fortify_panic(const char *name) __noreturn __cold;
void __read_overflow(void) __compiletime_error("detected read beyond size of object passed as 1st parameter");
void __read_overflow2(void) __compiletime_error("detected read beyond size of object passed as 2nd parameter");
void __write_overflow(void) __compiletime_error("detected write beyond size of object passed as 1st parameter");

#if !defined(__NO_FORTIFY) && defined(__OPTIMIZE__) && defined(CONFIG_FORTIFY_SOURCE)
__FORTIFY_INLINE char *strcpy(char *p, const char *q)
{
	size_t p_size = __builtin_object_size(p, 0);
	size_t q_size = __builtin_object_size(q, 0);
	if (p_size == (size_t)-1 && q_size == (size_t)-1)
		return __builtin_strcpy(p, q);
	if (strscpy(p, q, p_size < q_size ? p_size : q_size) < 0)
		fortify_panic(__func__);
	return p;
}

__FORTIFY_INLINE char *strncpy(char *p, const char *q, __kernel_size_t size)
{
	size_t p_size = __builtin_object_size(p, 0);
	if (__builtin_constant_p(size) && p_size < size)
		__write_overflow();
	if (p_size < size)
		fortify_panic(__func__);
	return __builtin_strncpy(p, q, size);
}

__FORTIFY_INLINE char *strcat(char *p, const char *q)
{
	size_t p_size = __builtin_object_size(p, 0);
	if (p_size == (size_t)-1)
		return __builtin_strcat(p, q);
	if (strlcat(p, q, p_size) >= p_size)
		fortify_panic(__func__);
	return p;
}

__FORTIFY_INLINE __kernel_size_t strlen(const char *p)
{
	__kernel_size_t ret;
	size_t p_size = __builtin_object_size(p, 0);
	if (p_size == (size_t)-1)
		return __builtin_strlen(p);
	ret = strnlen(p, p_size);
	if (p_size <= ret)
		fortify_panic(__func__);
	return ret;
}

extern __kernel_size_t __real_strnlen(const char *, __kernel_size_t) __RENAME(strnlen);
__FORTIFY_INLINE __kernel_size_t strnlen(const char *p, __kernel_size_t maxlen)
{
	size_t p_size = __builtin_object_size(p, 0);
	__kernel_size_t ret = __real_strnlen(p, maxlen < p_size ? maxlen : p_size);
	if (p_size <= ret && maxlen != ret)
		fortify_panic(__func__);
	return ret;
}

/* defined after fortified strlen to reuse it */
extern size_t __real_strlcpy(char *, const char *, size_t) __RENAME(strlcpy);
__FORTIFY_INLINE size_t strlcpy(char *p, const char *q, size_t size)
{
	size_t ret;
	size_t p_size = __builtin_object_size(p, 0);
	size_t q_size = __builtin_object_size(q, 0);
	if (p_size == (size_t)-1 && q_size == (size_t)-1)
		return __real_strlcpy(p, q, size);
	ret = strlen(q);
	if (size) {
		size_t len = (ret >= size) ? size - 1 : ret;
		if (__builtin_constant_p(len) && len >= p_size)
			__write_overflow();
		if (len >= p_size)
			fortify_panic(__func__);
		__builtin_memcpy(p, q, len);
		p[len] = '\0';
	}
	return ret;
}

/* defined after fortified strlen and strnlen to reuse them */
__FORTIFY_INLINE char *strncat(char *p, const char *q, __kernel_size_t count)
{
	size_t p_len, copy_len;
	size_t p_size = __builtin_object_size(p, 0);
	size_t q_size = __builtin_object_size(q, 0);
	if (p_size == (size_t)-1 && q_size == (size_t)-1)
		return __builtin_strncat(p, q, count);
	p_len = strlen(p);
	copy_len = strnlen(q, count);
	if (p_size < p_len + copy_len + 1)
		fortify_panic(__func__);
	__builtin_memcpy(p + p_len, q, copy_len);
	p[p_len + copy_len] = '\0';
	return p;
}

__FORTIFY_INLINE void *memset(void *p, int c, __kernel_size_t size)
{
	size_t p_size = __builtin_object_size(p, 0);
	if (__builtin_constant_p(size) && p_size < size)
		__write_overflow();
	if (p_size < size)
		fortify_panic(__func__);
	return __builtin_memset(p, c, size);
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
	if (p_size < size || q_size < size)
		fortify_panic(__func__);
	return __builtin_memcpy(p, q, size);
}

__FORTIFY_INLINE void *memmove(void *p, const void *q, __kernel_size_t size)
{
	size_t p_size = __builtin_object_size(p, 0);
	size_t q_size = __builtin_object_size(q, 0);
	if (__builtin_constant_p(size)) {
		if (p_size < size)
			__write_overflow();
		if (q_size < size)
			__read_overflow2();
	}
	if (p_size < size || q_size < size)
		fortify_panic(__func__);
	return __builtin_memmove(p, q, size);
}

extern void *__real_memscan(void *, int, __kernel_size_t) __RENAME(memscan);
__FORTIFY_INLINE void *memscan(void *p, int c, __kernel_size_t size)
{
	size_t p_size = __builtin_object_size(p, 0);
	if (__builtin_constant_p(size) && p_size < size)
		__read_overflow();
	if (p_size < size)
		fortify_panic(__func__);
	return __real_memscan(p, c, size);
}

__FORTIFY_INLINE int memcmp(const void *p, const void *q, __kernel_size_t size)
{
	size_t p_size = __builtin_object_size(p, 0);
	size_t q_size = __builtin_object_size(q, 0);
	if (__builtin_constant_p(size)) {
		if (p_size < size)
			__read_overflow();
		if (q_size < size)
			__read_overflow2();
	}
	if (p_size < size || q_size < size)
		fortify_panic(__func__);
	return __builtin_memcmp(p, q, size);
}

__FORTIFY_INLINE void *memchr(const void *p, int c, __kernel_size_t size)
{
	size_t p_size = __builtin_object_size(p, 0);
	if (__builtin_constant_p(size) && p_size < size)
		__read_overflow();
	if (p_size < size)
		fortify_panic(__func__);
	return __builtin_memchr(p, c, size);
}

void *__real_memchr_inv(const void *s, int c, size_t n) __RENAME(memchr_inv);
__FORTIFY_INLINE void *memchr_inv(const void *p, int c, size_t size)
{
	size_t p_size = __builtin_object_size(p, 0);
	if (__builtin_constant_p(size) && p_size < size)
		__read_overflow();
	if (p_size < size)
		fortify_panic(__func__);
	return __real_memchr_inv(p, c, size);
}

extern void *__real_kmemdup(const void *src, size_t len, gfp_t gfp) __RENAME(kmemdup);
__FORTIFY_INLINE void *kmemdup(const void *p, size_t size, gfp_t gfp)
{
	size_t p_size = __builtin_object_size(p, 0);
	if (__builtin_constant_p(size) && p_size < size)
		__read_overflow();
	if (p_size < size)
		fortify_panic(__func__);
	return __real_kmemdup(p, size, gfp);
}
#endif

#endif /* _LINUX_STRING_H_ */
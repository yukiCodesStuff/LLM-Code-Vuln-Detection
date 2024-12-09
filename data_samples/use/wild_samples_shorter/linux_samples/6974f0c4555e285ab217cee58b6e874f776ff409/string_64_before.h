extern void *memcpy(void *to, const void *from, size_t len);
extern void *__memcpy(void *to, const void *from, size_t len);

#ifndef CONFIG_KMEMCHECK
#if (__GNUC__ == 4 && __GNUC_MINOR__ < 3) || __GNUC__ < 4
#define memcpy(dst, src, len)					\
({								\
 */
#define memcpy(dst, src, len) __inline_memcpy((dst), (src), (len))
#endif

#define __HAVE_ARCH_MEMSET
void *memset(void *s, int c, size_t n);
void *__memset(void *s, int c, size_t n);
#define memcpy(dst, src, len) __memcpy(dst, src, len)
#define memmove(dst, src, len) __memmove(dst, src, len)
#define memset(s, c, n) __memset(s, c, n)
#endif

#define __HAVE_ARCH_MEMCPY_MCSAFE 1
__must_check int memcpy_mcsafe_unrolled(void *dst, const void *src, size_t cnt);
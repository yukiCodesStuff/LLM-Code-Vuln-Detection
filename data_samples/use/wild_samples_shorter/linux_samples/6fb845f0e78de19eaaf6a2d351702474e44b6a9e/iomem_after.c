#include <linux/module.h>
#include <linux/io.h>

#define movs(type,to,from) \
	asm volatile("movs" type:"=&D" (to), "=&S" (from):"0" (to), "1" (from):"memory")

/* Originally from i386/string.h */
static __always_inline void rep_movs(void *to, const void *from, size_t n)
{
	unsigned long d0, d1, d2;
	asm volatile("rep ; movsl\n\t"
		     "testb $2,%b4\n\t"

void memcpy_fromio(void *to, const volatile void __iomem *from, size_t n)
{
	if (unlikely(!n))
		return;

	/* Align any unaligned source IO */
	if (unlikely(1 & (unsigned long)from)) {
		movs("b", to, from);
		n--;
	}
	if (n > 1 && unlikely(2 & (unsigned long)from)) {
		movs("w", to, from);
		n-=2;
	}
	rep_movs(to, (const void *)from, n);
}
EXPORT_SYMBOL(memcpy_fromio);

void memcpy_toio(volatile void __iomem *to, const void *from, size_t n)
{
	if (unlikely(!n))
		return;

	/* Align any unaligned destination IO */
	if (unlikely(1 & (unsigned long)to)) {
		movs("b", to, from);
		n--;
	}
	if (n > 1 && unlikely(2 & (unsigned long)to)) {
		movs("w", to, from);
		n-=2;
	}
	rep_movs((void *)to, (const void *) from, n);
}
EXPORT_SYMBOL(memcpy_toio);

void memset_io(volatile void __iomem *a, int b, size_t c)
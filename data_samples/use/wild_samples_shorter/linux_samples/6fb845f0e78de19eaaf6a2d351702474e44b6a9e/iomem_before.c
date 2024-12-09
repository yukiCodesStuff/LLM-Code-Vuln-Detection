#include <linux/module.h>
#include <linux/io.h>

/* Originally from i386/string.h */
static __always_inline void __iomem_memcpy(void *to, const void *from, size_t n)
{
	unsigned long d0, d1, d2;
	asm volatile("rep ; movsl\n\t"
		     "testb $2,%b4\n\t"

void memcpy_fromio(void *to, const volatile void __iomem *from, size_t n)
{
	__iomem_memcpy(to, (const void *)from, n);
}
EXPORT_SYMBOL(memcpy_fromio);

void memcpy_toio(volatile void __iomem *to, const void *from, size_t n)
{
	__iomem_memcpy((void *)to, (const void *) from, n);
}
EXPORT_SYMBOL(memcpy_toio);

void memset_io(volatile void __iomem *a, int b, size_t c)
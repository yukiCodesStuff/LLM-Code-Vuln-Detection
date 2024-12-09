#define THREAD_SIZE		(1 << THREAD_SHIFT)

#ifdef CONFIG_PPC64
#define CURRENT_THREAD_INFO(dest, sp)	clrrdi dest, sp, THREAD_SHIFT
#else
#define CURRENT_THREAD_INFO(dest, sp)	rlwinm dest, sp, 0, 0, 31-THREAD_SHIFT
#endif

#ifndef __ASSEMBLY__
#include <linux/cache.h>
#define THREAD_SIZE_ORDER	(THREAD_SHIFT - PAGE_SHIFT)

/* how to get the thread information struct from C */
register unsigned long __current_r1 asm("r1");
static inline struct thread_info *current_thread_info(void)
{
	/* gcc4, at least, is smart enough to turn this into a single
	 * rlwinm for ppc32 and clrrdi for ppc64 */
	return (struct thread_info *)(__current_r1 & ~(THREAD_SIZE-1));
}

#endif /* __ASSEMBLY__ */

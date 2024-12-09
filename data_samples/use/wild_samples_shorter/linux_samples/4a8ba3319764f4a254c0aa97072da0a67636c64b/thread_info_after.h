#define THREAD_SIZE		(1 << THREAD_SHIFT)

#ifdef CONFIG_PPC64
#define CURRENT_THREAD_INFO(dest, sp)	stringify_in_c(clrrdi dest, sp, THREAD_SHIFT)
#else
#define CURRENT_THREAD_INFO(dest, sp)	stringify_in_c(rlwinm dest, sp, 0, 0, 31-THREAD_SHIFT)
#endif

#ifndef __ASSEMBLY__
#include <linux/cache.h>
#define THREAD_SIZE_ORDER	(THREAD_SHIFT - PAGE_SHIFT)

/* how to get the thread information struct from C */
static inline struct thread_info *current_thread_info(void)
{
	unsigned long val;

	asm (CURRENT_THREAD_INFO(%0,1) : "=r" (val));

	return (struct thread_info *)val;
}

#endif /* __ASSEMBLY__ */

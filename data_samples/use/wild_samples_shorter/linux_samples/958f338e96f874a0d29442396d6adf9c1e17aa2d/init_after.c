#include <linux/swap.h>
#include <linux/memblock.h>
#include <linux/bootmem.h>	/* for max_low_pfn */
#include <linux/swapfile.h>
#include <linux/swapops.h>

#include <asm/set_memory.h>
#include <asm/e820/api.h>
#include <asm/init.h>
	__cachemode2pte_tbl[cache] = __cm_idx2pte(entry);
	__pte2cachemode_tbl[entry] = cache;
}

unsigned long max_swapfile_size(void)
{
	unsigned long pages;

	pages = generic_max_swapfile_size();

	if (boot_cpu_has_bug(X86_BUG_L1TF)) {
		/* Limit the swap file size to MAX_PA/2 for L1TF workaround */
		unsigned long l1tf_limit = l1tf_pfn_limit() + 1;
		/*
		 * We encode swap offsets also with 3 bits below those for pfn
		 * which makes the usable limit higher.
		 */
#if CONFIG_PGTABLE_LEVELS > 2
		l1tf_limit <<= PAGE_SHIFT - SWP_OFFSET_FIRST_BIT;
#endif
		pages = min_t(unsigned long, l1tf_limit, pages);
	}
	return pages;
}
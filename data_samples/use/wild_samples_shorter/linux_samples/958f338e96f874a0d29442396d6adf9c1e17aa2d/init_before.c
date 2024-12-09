#include <linux/swap.h>
#include <linux/memblock.h>
#include <linux/bootmem.h>	/* for max_low_pfn */

#include <asm/set_memory.h>
#include <asm/e820/api.h>
#include <asm/init.h>
	__cachemode2pte_tbl[cache] = __cm_idx2pte(entry);
	__pte2cachemode_tbl[entry] = cache;
}
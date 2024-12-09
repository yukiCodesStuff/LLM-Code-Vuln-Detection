#include <linux/cpu.h>
#include <linux/dma-mapping.h>
#include <linux/gfp.h>
#include <linux/highmem.h>
#include <linux/export.h>
#include <linux/memblock.h>
#include <linux/of_address.h>
#include <linux/slab.h>
#include <linux/types.h>
#include <linux/dma-mapping.h>
#include <linux/vmalloc.h>
#include <linux/swiotlb.h>

#include <xen/xen.h>
#include <xen/interface/grant_table.h>
#include <xen/interface/memory.h>
#include <xen/page.h>
#include <xen/swiotlb-xen.h>

#include <asm/cacheflush.h>
#include <asm/xen/hypercall.h>
#include <asm/xen/interface.h>

unsigned long xen_get_swiotlb_free_pages(unsigned int order)
{
	struct memblock_region *reg;
	gfp_t flags = __GFP_NOWARN|__GFP_KSWAPD_RECLAIM;

	for_each_memblock(memory, reg) {
		if (reg->base < (phys_addr_t)0xffffffff) {
			flags |= __GFP_DMA;
			break;
		}
	}
	return __get_free_pages(flags, order);
}
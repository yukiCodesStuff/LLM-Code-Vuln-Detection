 */
#include "type_support.h"
#include "mmu/isp_mmu.h"
#include "memory_access/memory_access.h"
#include "atomisp_compat.h"

#define MERR_VALID_PTE_MASK	0x80000000
	return (phys_addr_t)((pte & ~mask) << ISP_PAGE_OFFSET);
}

/*
 * set page directory base address (physical address).
 *
 * must be provided.
 */
static int sh_set_pd_base(struct isp_mmu *mmu,
			  phys_addr_t phys)
{
	unsigned int pte = sh_phys_to_pte(mmu, phys);
	/*mmgr_set_base_address(HOST_ADDRESS(pte));*/
	atomisp_css_mmu_set_page_table_base_index(HOST_ADDRESS(pte));
	return 0;
}

static unsigned int sh_get_pd_base(struct isp_mmu *mmu,
				   phys_addr_t phys)
{
	unsigned int pte = sh_phys_to_pte(mmu, phys);
	.name = "Silicon Hive ISP3000 MMU",
	.pte_valid_mask = MERR_VALID_PTE_MASK,
	.null_pte = ~MERR_VALID_PTE_MASK,
	.set_pd_base = sh_set_pd_base,
	.get_pd_base = sh_get_pd_base,
	.tlb_flush_all = sh_tlb_flush,
	.phys_to_pte = sh_phys_to_pte,
	.pte_to_phys = sh_pte_to_phys,
 */
#include "type_support.h"
#include "mmu/isp_mmu.h"
#include "mmu/sh_mmu_mrfld.h"
#include "memory_access/memory_access.h"
#include "atomisp_compat.h"

#define MERR_VALID_PTE_MASK	0x80000000
	return (phys_addr_t)((pte & ~mask) << ISP_PAGE_OFFSET);
}

static unsigned int sh_get_pd_base(struct isp_mmu *mmu,
				   phys_addr_t phys)
{
	unsigned int pte = sh_phys_to_pte(mmu, phys);
	.name = "Silicon Hive ISP3000 MMU",
	.pte_valid_mask = MERR_VALID_PTE_MASK,
	.null_pte = ~MERR_VALID_PTE_MASK,
	.get_pd_base = sh_get_pd_base,
	.tlb_flush_all = sh_tlb_flush,
	.phys_to_pte = sh_phys_to_pte,
	.pte_to_phys = sh_pte_to_phys,
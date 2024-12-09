#define _ASM_ARC_HUGEPAGE_H

#include <linux/types.h>
#include <asm-generic/pgtable-nopmd.h>

static inline pte_t pmd_pte(pmd_t pmd)
{
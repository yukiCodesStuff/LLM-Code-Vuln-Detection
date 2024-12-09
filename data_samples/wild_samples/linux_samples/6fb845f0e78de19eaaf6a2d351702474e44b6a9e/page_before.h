typedef struct {
	unsigned long pgprot;
} pgprot_t;

typedef struct page *pgtable_t;

#define pte_val(x)	((x).pte)
#define pgd_val(x)	((x).pgd)
#define pgprot_val(x)	((x).pgprot)

#define __pte(x)	((pte_t) { (x) })
#define __pgd(x)	((pgd_t) { (x) })
#define __pgprot(x)	((pgprot_t) { (x) })

#ifdef CONFIG_64BITS
#define PTE_FMT "%016lx"
#else
#define PTE_FMT "%08lx"
#endif

extern unsigned long va_pa_offset;
extern unsigned long pfn_base;

extern unsigned long max_low_pfn;
extern unsigned long min_low_pfn;

#define __pa(x)		((unsigned long)(x) - va_pa_offset)
#define __va(x)		((void *)((unsigned long) (x) + va_pa_offset))

#define phys_to_pfn(phys)	(PFN_DOWN(phys))
#define pfn_to_phys(pfn)	(PFN_PHYS(pfn))

#define virt_to_pfn(vaddr)	(phys_to_pfn(__pa(vaddr)))
#define pfn_to_virt(pfn)	(__va(pfn_to_phys(pfn)))

#define virt_to_page(vaddr)	(pfn_to_page(virt_to_pfn(vaddr)))
#define page_to_virt(page)	(pfn_to_virt(page_to_pfn(page)))

#define page_to_phys(page)	(pfn_to_phys(page_to_pfn(page)))
#define page_to_bus(page)	(page_to_phys(page))
#define phys_to_page(paddr)	(pfn_to_page(phys_to_pfn(paddr)))

#define pfn_valid(pfn) \
	(((pfn) >= pfn_base) && (((pfn)-pfn_base) < max_mapnr))

#define ARCH_PFN_OFFSET		(pfn_base)

#endif /* __ASSEMBLY__ */

#define virt_addr_valid(vaddr)	(pfn_valid(virt_to_pfn(vaddr)))

#define VM_DATA_DEFAULT_FLAGS	(VM_READ | VM_WRITE | \
				 VM_MAYREAD | VM_MAYWRITE | VM_MAYEXEC)

#include <asm-generic/memory_model.h>
#include <asm-generic/getorder.h>

/* vDSO support */
/* We do define AT_SYSINFO_EHDR but don't use the gate mechanism */
#define __HAVE_ARCH_GATE_AREA

#endif /* _ASM_RISCV_PAGE_H */
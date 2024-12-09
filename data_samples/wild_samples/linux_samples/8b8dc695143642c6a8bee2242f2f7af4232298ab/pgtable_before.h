{
	/*
	 * A pmd is considerent present if _PAGE_PRESENT is set.
	 * We also need to consider the pmd present which is marked
	 * invalid during a split. Hence we look for _PAGE_INVALID
	 * if we find _PAGE_PRESENT cleared.
	 */
	if (pmd_raw(pmd) & cpu_to_be64(_PAGE_PRESENT | _PAGE_INVALID))
		return true;

	return false;
}

static inline int pmd_bad(pmd_t pmd)
{
	if (radix_enabled())
		return radix__pmd_bad(pmd);
	return hash__pmd_bad(pmd);
}

static inline void pud_clear(pud_t *pudp)
{
	*pudp = __pud(0);
}

static inline int pud_none(pud_t pud)
{
	return !pud_raw(pud);
}

static inline int pud_present(pud_t pud)
{
	return !!(pud_raw(pud) & cpu_to_be64(_PAGE_PRESENT));
}

extern struct page *pud_page(pud_t pud);
extern struct page *pmd_page(pmd_t pmd);
static inline pte_t pud_pte(pud_t pud)
{
	return __pte_raw(pud_raw(pud));
}

static inline pud_t pte_pud(pte_t pte)
{
	return __pud_raw(pte_raw(pte));
}
#define pud_write(pud)		pte_write(pud_pte(pud))

static inline int pud_bad(pud_t pud)
{
	if (radix_enabled())
		return radix__pud_bad(pud);
	return hash__pud_bad(pud);
}

#define pud_access_permitted pud_access_permitted
static inline bool pud_access_permitted(pud_t pud, bool write)
{
	return pte_access_permitted(pud_pte(pud), write);
}

#define pgd_write(pgd)		pte_write(pgd_pte(pgd))

static inline void pgd_clear(pgd_t *pgdp)
{
	*pgdp = __pgd(0);
}

static inline int pgd_none(pgd_t pgd)
{
	return !pgd_raw(pgd);
}

static inline int pgd_present(pgd_t pgd)
{
	return !!(pgd_raw(pgd) & cpu_to_be64(_PAGE_PRESENT));
}

static inline pte_t pgd_pte(pgd_t pgd)
{
	return __pte_raw(pgd_raw(pgd));
}

static inline pgd_t pte_pgd(pte_t pte)
{
	return __pgd_raw(pte_raw(pte));
}

static inline int pgd_bad(pgd_t pgd)
{
	if (radix_enabled())
		return radix__pgd_bad(pgd);
	return hash__pgd_bad(pgd);
}

#define pgd_access_permitted pgd_access_permitted
static inline bool pgd_access_permitted(pgd_t pgd, bool write)
{
	return pte_access_permitted(pgd_pte(pgd), write);
}

extern struct page *pgd_page(pgd_t pgd);

/* Pointers in the page table tree are physical addresses */
#define __pgtable_ptr_val(ptr)	__pa(ptr)

#define pmd_page_vaddr(pmd)	__va(pmd_val(pmd) & ~PMD_MASKED_BITS)
#define pud_page_vaddr(pud)	__va(pud_val(pud) & ~PUD_MASKED_BITS)
#define pgd_page_vaddr(pgd)	__va(pgd_val(pgd) & ~PGD_MASKED_BITS)

#define pgd_index(address) (((address) >> (PGDIR_SHIFT)) & (PTRS_PER_PGD - 1))
#define pud_index(address) (((address) >> (PUD_SHIFT)) & (PTRS_PER_PUD - 1))
#define pmd_index(address) (((address) >> (PMD_SHIFT)) & (PTRS_PER_PMD - 1))
#define pte_index(address) (((address) >> (PAGE_SHIFT)) & (PTRS_PER_PTE - 1))

/*
 * Find an entry in a page-table-directory.  We combine the address region
 * (the high order N bits) and the pgd portion of the address.
 */

#define pgd_offset(mm, address)	 ((mm)->pgd + pgd_index(address))

#define pud_offset(pgdp, addr)	\
	(((pud_t *) pgd_page_vaddr(*(pgdp))) + pud_index(addr))
#define pmd_offset(pudp,addr) \
	(((pmd_t *) pud_page_vaddr(*(pudp))) + pmd_index(addr))
#define pte_offset_kernel(dir,addr) \
	(((pte_t *) pmd_page_vaddr(*(dir))) + pte_index(addr))

#define pte_offset_map(dir,addr)	pte_offset_kernel((dir), (addr))

static inline void pte_unmap(pte_t *pte) { }

/* to find an entry in a kernel page-table-directory */
/* This now only contains the vmalloc pages */
#define pgd_offset_k(address) pgd_offset(&init_mm, address)

#define pte_ERROR(e) \
	pr_err("%s:%d: bad pte %08lx.\n", __FILE__, __LINE__, pte_val(e))
#define pmd_ERROR(e) \
	pr_err("%s:%d: bad pmd %08lx.\n", __FILE__, __LINE__, pmd_val(e))
#define pud_ERROR(e) \
	pr_err("%s:%d: bad pud %08lx.\n", __FILE__, __LINE__, pud_val(e))
#define pgd_ERROR(e) \
	pr_err("%s:%d: bad pgd %08lx.\n", __FILE__, __LINE__, pgd_val(e))

static inline int map_kernel_page(unsigned long ea, unsigned long pa, pgprot_t prot)
{
	if (radix_enabled()) {
#if defined(CONFIG_PPC_RADIX_MMU) && defined(DEBUG_VM)
		unsigned long page_size = 1 << mmu_psize_defs[mmu_io_psize].shift;
		WARN((page_size != PAGE_SIZE), "I/O page size != PAGE_SIZE");
#endif
		return radix__map_kernel_page(ea, pa, prot, PAGE_SIZE);
	}
	return hash__map_kernel_page(ea, pa, prot);
}

static inline int __meminit vmemmap_create_mapping(unsigned long start,
						   unsigned long page_size,
						   unsigned long phys)
{
	if (radix_enabled())
		return radix__vmemmap_create_mapping(start, page_size, phys);
	return hash__vmemmap_create_mapping(start, page_size, phys);
}

#ifdef CONFIG_MEMORY_HOTPLUG
static inline void vmemmap_remove_mapping(unsigned long start,
					  unsigned long page_size)
{
	if (radix_enabled())
		return radix__vmemmap_remove_mapping(start, page_size);
	return hash__vmemmap_remove_mapping(start, page_size);
}
#endif

static inline pte_t pmd_pte(pmd_t pmd)
{
	return __pte_raw(pmd_raw(pmd));
}

static inline pmd_t pte_pmd(pte_t pte)
{
	return __pmd_raw(pte_raw(pte));
}

static inline pte_t *pmdp_ptep(pmd_t *pmd)
{
	return (pte_t *)pmd;
}
#define pmd_pfn(pmd)		pte_pfn(pmd_pte(pmd))
#define pmd_dirty(pmd)		pte_dirty(pmd_pte(pmd))
#define pmd_young(pmd)		pte_young(pmd_pte(pmd))
#define pmd_mkold(pmd)		pte_pmd(pte_mkold(pmd_pte(pmd)))
#define pmd_wrprotect(pmd)	pte_pmd(pte_wrprotect(pmd_pte(pmd)))
#define pmd_mkdirty(pmd)	pte_pmd(pte_mkdirty(pmd_pte(pmd)))
#define pmd_mkclean(pmd)	pte_pmd(pte_mkclean(pmd_pte(pmd)))
#define pmd_mkyoung(pmd)	pte_pmd(pte_mkyoung(pmd_pte(pmd)))
#define pmd_mkwrite(pmd)	pte_pmd(pte_mkwrite(pmd_pte(pmd)))
#define pmd_mk_savedwrite(pmd)	pte_pmd(pte_mk_savedwrite(pmd_pte(pmd)))
#define pmd_clear_savedwrite(pmd)	pte_pmd(pte_clear_savedwrite(pmd_pte(pmd)))

#ifdef CONFIG_HAVE_ARCH_SOFT_DIRTY
#define pmd_soft_dirty(pmd)    pte_soft_dirty(pmd_pte(pmd))
#define pmd_mksoft_dirty(pmd)  pte_pmd(pte_mksoft_dirty(pmd_pte(pmd)))
#define pmd_clear_soft_dirty(pmd) pte_pmd(pte_clear_soft_dirty(pmd_pte(pmd)))

#ifdef CONFIG_ARCH_ENABLE_THP_MIGRATION
#define pmd_swp_mksoft_dirty(pmd)	pte_pmd(pte_swp_mksoft_dirty(pmd_pte(pmd)))
#define pmd_swp_soft_dirty(pmd)		pte_swp_soft_dirty(pmd_pte(pmd))
#define pmd_swp_clear_soft_dirty(pmd)	pte_pmd(pte_swp_clear_soft_dirty(pmd_pte(pmd)))
#endif
#endif /* CONFIG_HAVE_ARCH_SOFT_DIRTY */

#ifdef CONFIG_NUMA_BALANCING
static inline int pmd_protnone(pmd_t pmd)
{
	return pte_protnone(pmd_pte(pmd));
}
#endif /* CONFIG_NUMA_BALANCING */

#define pmd_write(pmd)		pte_write(pmd_pte(pmd))
#define __pmd_write(pmd)	__pte_write(pmd_pte(pmd))
#define pmd_savedwrite(pmd)	pte_savedwrite(pmd_pte(pmd))

#define pmd_access_permitted pmd_access_permitted
static inline bool pmd_access_permitted(pmd_t pmd, bool write)
{
	return pte_access_permitted(pmd_pte(pmd), write);
}

#ifdef CONFIG_TRANSPARENT_HUGEPAGE
extern pmd_t pfn_pmd(unsigned long pfn, pgprot_t pgprot);
extern pmd_t mk_pmd(struct page *page, pgprot_t pgprot);
extern pmd_t pmd_modify(pmd_t pmd, pgprot_t newprot);
extern void set_pmd_at(struct mm_struct *mm, unsigned long addr,
		       pmd_t *pmdp, pmd_t pmd);
extern void update_mmu_cache_pmd(struct vm_area_struct *vma, unsigned long addr,
				 pmd_t *pmd);
extern int hash__has_transparent_hugepage(void);
static inline int has_transparent_hugepage(void)
{
	if (radix_enabled())
		return radix__has_transparent_hugepage();
	return hash__has_transparent_hugepage();
}
#define has_transparent_hugepage has_transparent_hugepage

static inline unsigned long
pmd_hugepage_update(struct mm_struct *mm, unsigned long addr, pmd_t *pmdp,
		    unsigned long clr, unsigned long set)
{
	if (radix_enabled())
		return radix__pmd_hugepage_update(mm, addr, pmdp, clr, set);
	return hash__pmd_hugepage_update(mm, addr, pmdp, clr, set);
}

/*
 * returns true for pmd migration entries, THP, devmap, hugetlb
 * But compile time dependent on THP config
 */
static inline int pmd_large(pmd_t pmd)
{
	return !!(pmd_raw(pmd) & cpu_to_be64(_PAGE_PTE));
}

static inline pmd_t pmd_mknotpresent(pmd_t pmd)
{
	return __pmd(pmd_val(pmd) & ~_PAGE_PRESENT);
}
/*
 * For radix we should always find H_PAGE_HASHPTE zero. Hence
 * the below will work for radix too
 */
static inline int __pmdp_test_and_clear_young(struct mm_struct *mm,
					      unsigned long addr, pmd_t *pmdp)
{
	unsigned long old;

	if ((pmd_raw(*pmdp) & cpu_to_be64(_PAGE_ACCESSED | H_PAGE_HASHPTE)) == 0)
		return 0;
	old = pmd_hugepage_update(mm, addr, pmdp, _PAGE_ACCESSED, 0);
	return ((old & _PAGE_ACCESSED) != 0);
}

#define __HAVE_ARCH_PMDP_SET_WRPROTECT
static inline void pmdp_set_wrprotect(struct mm_struct *mm, unsigned long addr,
				      pmd_t *pmdp)
{
	if (__pmd_write((*pmdp)))
		pmd_hugepage_update(mm, addr, pmdp, _PAGE_WRITE, 0);
	else if (unlikely(pmd_savedwrite(*pmdp)))
		pmd_hugepage_update(mm, addr, pmdp, 0, _PAGE_PRIVILEGED);
}

/*
 * Only returns true for a THP. False for pmd migration entry.
 * We also need to return true when we come across a pte that
 * in between a thp split. While splitting THP, we mark the pmd
 * invalid (pmdp_invalidate()) before we set it with pte page
 * address. A pmd_trans_huge() check against a pmd entry during that time
 * should return true.
 * We should not call this on a hugetlb entry. We should check for HugeTLB
 * entry using vma->vm_flags
 * The page table walk rule is explained in Documentation/vm/transhuge.rst
 */
static inline int pmd_trans_huge(pmd_t pmd)
{
	if (!pmd_present(pmd))
		return false;

	if (radix_enabled())
		return radix__pmd_trans_huge(pmd);
	return hash__pmd_trans_huge(pmd);
}

#define __HAVE_ARCH_PMD_SAME
static inline int pmd_same(pmd_t pmd_a, pmd_t pmd_b)
{
	if (radix_enabled())
		return radix__pmd_same(pmd_a, pmd_b);
	return hash__pmd_same(pmd_a, pmd_b);
}

static inline pmd_t pmd_mkhuge(pmd_t pmd)
{
	if (radix_enabled())
		return radix__pmd_mkhuge(pmd);
	return hash__pmd_mkhuge(pmd);
}

#define __HAVE_ARCH_PMDP_SET_ACCESS_FLAGS
extern int pmdp_set_access_flags(struct vm_area_struct *vma,
				 unsigned long address, pmd_t *pmdp,
				 pmd_t entry, int dirty);

#define __HAVE_ARCH_PMDP_TEST_AND_CLEAR_YOUNG
extern int pmdp_test_and_clear_young(struct vm_area_struct *vma,
				     unsigned long address, pmd_t *pmdp);

#define __HAVE_ARCH_PMDP_HUGE_GET_AND_CLEAR
static inline pmd_t pmdp_huge_get_and_clear(struct mm_struct *mm,
					    unsigned long addr, pmd_t *pmdp)
{
	if (radix_enabled())
		return radix__pmdp_huge_get_and_clear(mm, addr, pmdp);
	return hash__pmdp_huge_get_and_clear(mm, addr, pmdp);
}

static inline pmd_t pmdp_collapse_flush(struct vm_area_struct *vma,
					unsigned long address, pmd_t *pmdp)
{
	if (radix_enabled())
		return radix__pmdp_collapse_flush(vma, address, pmdp);
	return hash__pmdp_collapse_flush(vma, address, pmdp);
}
#define pmdp_collapse_flush pmdp_collapse_flush

#define __HAVE_ARCH_PGTABLE_DEPOSIT
static inline void pgtable_trans_huge_deposit(struct mm_struct *mm,
					      pmd_t *pmdp, pgtable_t pgtable)
{
	if (radix_enabled())
		return radix__pgtable_trans_huge_deposit(mm, pmdp, pgtable);
	return hash__pgtable_trans_huge_deposit(mm, pmdp, pgtable);
}

#define __HAVE_ARCH_PGTABLE_WITHDRAW
static inline pgtable_t pgtable_trans_huge_withdraw(struct mm_struct *mm,
						    pmd_t *pmdp)
{
	if (radix_enabled())
		return radix__pgtable_trans_huge_withdraw(mm, pmdp);
	return hash__pgtable_trans_huge_withdraw(mm, pmdp);
}

#define __HAVE_ARCH_PMDP_INVALIDATE
extern pmd_t pmdp_invalidate(struct vm_area_struct *vma, unsigned long address,
			     pmd_t *pmdp);

#define pmd_move_must_withdraw pmd_move_must_withdraw
struct spinlock;
extern int pmd_move_must_withdraw(struct spinlock *new_pmd_ptl,
				  struct spinlock *old_pmd_ptl,
				  struct vm_area_struct *vma);
/*
 * Hash translation mode use the deposited table to store hash pte
 * slot information.
 */
#define arch_needs_pgtable_deposit arch_needs_pgtable_deposit
static inline bool arch_needs_pgtable_deposit(void)
{
	if (radix_enabled())
		return false;
	return true;
}
extern void serialize_against_pte_lookup(struct mm_struct *mm);


static inline pmd_t pmd_mkdevmap(pmd_t pmd)
{
	return __pmd(pmd_val(pmd) | (_PAGE_PTE | _PAGE_DEVMAP));
}

static inline int pmd_devmap(pmd_t pmd)
{
	return pte_devmap(pmd_pte(pmd));
}

static inline int pud_devmap(pud_t pud)
{
	return 0;
}

static inline int pgd_devmap(pgd_t pgd)
{
	return 0;
}
#endif /* CONFIG_TRANSPARENT_HUGEPAGE */

static inline int pud_pfn(pud_t pud)
{
	/*
	 * Currently all calls to pud_pfn() are gated around a pud_devmap()
	 * check so this should never be used. If it grows another user we
	 * want to know about it.
	 */
	BUILD_BUG();
	return 0;
}
#define __HAVE_ARCH_PTEP_MODIFY_PROT_TRANSACTION
pte_t ptep_modify_prot_start(struct vm_area_struct *, unsigned long, pte_t *);
void ptep_modify_prot_commit(struct vm_area_struct *, unsigned long,
			     pte_t *, pte_t, pte_t);

/*
 * Returns true for a R -> RW upgrade of pte
 */
static inline bool is_pte_rw_upgrade(unsigned long old_val, unsigned long new_val)
{
	if (!(old_val & _PAGE_READ))
		return false;

	if ((!(old_val & _PAGE_WRITE)) && (new_val & _PAGE_WRITE))
		return true;

	return false;
}

#endif /* __ASSEMBLY__ */
#endif /* _ASM_POWERPC_BOOK3S_64_PGTABLE_H_ */
{
	return pte_protnone(pmd_pte(pmd));
}
#endif /* CONFIG_NUMA_BALANCING */

#define pmd_write(pmd)		pte_write(pmd_pte(pmd))
#define __pmd_write(pmd)	__pte_write(pmd_pte(pmd))
#define pmd_savedwrite(pmd)	pte_savedwrite(pmd_pte(pmd))

#define pmd_access_permitted pmd_access_permitted
static inline bool pmd_access_permitted(pmd_t pmd, bool write)
{
	return pte_access_permitted(pmd_pte(pmd), write);
}
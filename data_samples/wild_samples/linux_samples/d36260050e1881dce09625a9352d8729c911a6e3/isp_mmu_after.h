struct isp_mmu_client {
	/*
	 * const value
	 *
	 * @name:
	 *      driver name
	 * @pte_valid_mask:
	 *      should be 1 bit valid data, meaning the value should
	 *      be power of 2.
	 */
	char *name;
	unsigned int pte_valid_mask;
	unsigned int null_pte;

	/*
	 * get page directory base address (physical address).
	 *
	 * must be provided.
	 */
	unsigned int (*get_pd_base) (struct isp_mmu *mmu, phys_addr_t pd_base);
	/*
	 * callback to flush tlb.
	 *
	 * tlb_flush_range will at least flush TLBs containing
	 * address mapping from addr to addr + size.
	 *
	 * tlb_flush_all will flush all TLBs.
	 *
	 * tlb_flush_all is must be provided. if tlb_flush_range is
	 * not valid, it will set to tlb_flush_all by default.
	 */
	void (*tlb_flush_range) (struct isp_mmu *mmu,
				 unsigned int addr, unsigned int size);
	void (*tlb_flush_all) (struct isp_mmu *mmu);
	unsigned int (*phys_to_pte) (struct isp_mmu *mmu,
				     phys_addr_t phys);
	phys_addr_t (*pte_to_phys) (struct isp_mmu *mmu,
				    unsigned int pte);

};

struct isp_mmu {
	struct isp_mmu_client *driver;
	unsigned int l1_pte;
	int l2_pgt_refcount[ISP_L1PT_PTES];
	phys_addr_t base_address;

	struct mutex pt_mutex;
	struct kmem_cache *tbl_cache;
};

/* flags for PDE and PTE */
#define	ISP_PTE_VALID_MASK(mmu)	\
	((mmu)->driver->pte_valid_mask)

#define	ISP_PTE_VALID(mmu, pte)	\
	((pte) & ISP_PTE_VALID_MASK(mmu))

#define	NULL_PAGE	((phys_addr_t)(-1) & ISP_PAGE_MASK)
#define	PAGE_VALID(page)	((page) != NULL_PAGE)

/*
 * init mmu with specific mmu driver.
 */
int isp_mmu_init(struct isp_mmu *mmu, struct isp_mmu_client *driver);
/*
 * cleanup all mmu related things.
 */
void isp_mmu_exit(struct isp_mmu *mmu);

/*
 * setup/remove address mapping for pgnr continous physical pages
 * and isp_virt.
 *
 * map/unmap is mutex lock protected, and caller does not have
 * to do lock/unlock operation.
 *
 * map/unmap will not flush tlb, and caller needs to deal with
 * this itself.
 */
int isp_mmu_map(struct isp_mmu *mmu, unsigned int isp_virt,
		phys_addr_t phys, unsigned int pgnr);

void isp_mmu_unmap(struct isp_mmu *mmu, unsigned int isp_virt,
		   unsigned int pgnr);

static inline void isp_mmu_flush_tlb_all(struct isp_mmu *mmu)
{
	if (mmu->driver && mmu->driver->tlb_flush_all)
		mmu->driver->tlb_flush_all(mmu);
}

#define isp_mmu_flush_tlb isp_mmu_flush_tlb_all

static inline void isp_mmu_flush_tlb_range(struct isp_mmu *mmu,
		unsigned int start, unsigned int size)
{
	if (mmu->driver && mmu->driver->tlb_flush_range)
		mmu->driver->tlb_flush_range(mmu, start, size);
}

#endif /* ISP_MMU_H_ */
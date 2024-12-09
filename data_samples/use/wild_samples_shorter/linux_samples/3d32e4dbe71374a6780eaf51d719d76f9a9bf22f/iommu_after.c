				gfn_t base_gfn, unsigned long npages);

static pfn_t kvm_pin_pages(struct kvm_memory_slot *slot, gfn_t gfn,
			   unsigned long npages)
{
	gfn_t end_gfn;
	pfn_t pfn;

	pfn     = gfn_to_pfn_memslot(slot, gfn);
	end_gfn = gfn + npages;
	gfn    += 1;

	if (is_error_noslot_pfn(pfn))
		return pfn;
		 * Pin all pages we are about to map in memory. This is
		 * important because we unmap and unpin in 4kb steps later.
		 */
		pfn = kvm_pin_pages(slot, gfn, page_size >> PAGE_SHIFT);
		if (is_error_noslot_pfn(pfn)) {
			gfn += 1;
			continue;
		}
		if (r) {
			printk(KERN_ERR "kvm_iommu_map_address:"
			       "iommu failed to map pfn=%llx\n", pfn);
			kvm_unpin_pages(kvm, pfn, page_size >> PAGE_SHIFT);
			goto unmap_pages;
		}

		gfn += page_size >> PAGE_SHIFT;
	return pfn;
}

static void kvm_unpin_pages(struct kvm *kvm, pfn_t pfn, unsigned long npages)
{
	unsigned long i;

	for (i = 0; i < npages; ++i)
		kvm_release_pfn_clean(pfn + i);
}

int kvm_iommu_map_pages(struct kvm *kvm, struct kvm_memory_slot *slot)
{
	gfn_t gfn, end_gfn;
	pfn_t pfn;
		if (r) {
			printk(KERN_ERR "kvm_iommu_map_address:"
			       "iommu failed to map pfn=%llx\n", pfn);
			kvm_unpin_pages(kvm, pfn, page_size);
			goto unmap_pages;
		}

		gfn += page_size >> PAGE_SHIFT;
	return 0;

unmap_pages:
	kvm_iommu_put_pages(kvm, slot->base_gfn, gfn - slot->base_gfn);
	return r;
}

static int kvm_iommu_map_memslots(struct kvm *kvm)
	return r;
}

static void kvm_iommu_put_pages(struct kvm *kvm,
				gfn_t base_gfn, unsigned long npages)
{
	struct iommu_domain *domain;
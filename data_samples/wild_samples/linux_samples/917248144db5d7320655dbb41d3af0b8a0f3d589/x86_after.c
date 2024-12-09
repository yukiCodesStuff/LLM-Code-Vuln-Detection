{
	void *wbinvd_dirty_mask = vcpu->arch.wbinvd_dirty_mask;
	struct gfn_to_pfn_cache *cache = &vcpu->arch.st.cache;

	kvm_release_pfn(cache->pfn, cache->dirty, cache);

	kvmclock_reset(vcpu);

	kvm_x86_ops->vcpu_free(vcpu);
	free_cpumask_var(wbinvd_dirty_mask);
}
	for (i = 0; i < KVM_NR_PAGE_SIZES; ++i) {
		kvfree(slot->arch.rmap[i]);
		slot->arch.rmap[i] = NULL;
		if (i == 0)
			continue;

		kvfree(slot->arch.lpage_info[i - 1]);
		slot->arch.lpage_info[i - 1] = NULL;
	}
	return -ENOMEM;
}

void kvm_arch_memslots_updated(struct kvm *kvm, u64 gen)
{
	struct kvm_vcpu *vcpu;
	int i;

	/*
	 * memslots->generation has been incremented.
	 * mmio generation may have reached its maximum value.
	 */
	kvm_mmu_invalidate_mmio_sptes(kvm, gen);

	/* Force re-initialization of steal_time cache */
	kvm_for_each_vcpu(i, vcpu, kvm)
		kvm_vcpu_kick(vcpu);
}
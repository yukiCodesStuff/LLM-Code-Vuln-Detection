{
	bool list_unstable, zapped_root = false;

	trace_kvm_mmu_prepare_zap_page(sp);
	++kvm->stat.mmu_shadow_zapped;
	*nr_zapped = mmu_zap_unsync_children(kvm, sp, invalid_list);
	*nr_zapped += kvm_mmu_page_unlink_children(kvm, sp, invalid_list);
	if (is_page_fault_stale(vcpu, fault, mmu_seq))
		goto out_unlock;

	r = make_mmu_pages_available(vcpu);
	if (r)
		goto out_unlock;

	if (is_tdp_mmu_fault)
		r = kvm_tdp_mmu_map(vcpu, fault);
	else
		r = __direct_map(vcpu, fault);

out_unlock:
	if (is_tdp_mmu_fault)
		read_unlock(&vcpu->kvm->mmu_lock);
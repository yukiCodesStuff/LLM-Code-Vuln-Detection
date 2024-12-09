{
	bool list_unstable, zapped_root = false;

	lockdep_assert_held_write(&kvm->mmu_lock);
	trace_kvm_mmu_prepare_zap_page(sp);
	++kvm->stat.mmu_shadow_zapped;
	*nr_zapped = mmu_zap_unsync_children(kvm, sp, invalid_list);
	*nr_zapped += kvm_mmu_page_unlink_children(kvm, sp, invalid_list);
	kvm_mmu_unlink_parents(sp);

	/* Zapping children means active_mmu_pages has become unstable. */
	list_unstable = *nr_zapped;

	if (!sp->role.invalid && sp_has_gptes(sp))
		unaccount_shadowed(kvm, sp);

	if (sp->unsync)
		kvm_unlink_unsync_page(kvm, sp);
	if (!sp->root_count) {
		/* Count self */
		(*nr_zapped)++;

		/*
		 * Already invalid pages (previously active roots) are not on
		 * the active page list.  See list_del() in the "else" case of
		 * !sp->root_count.
		 */
		if (sp->role.invalid)
			list_add(&sp->link, invalid_list);
		else
			list_move(&sp->link, invalid_list);
		kvm_unaccount_mmu_page(kvm, sp);
	} else {
		/*
		 * Remove the active root from the active page list, the root
		 * will be explicitly freed when the root_count hits zero.
		 */
		list_del(&sp->link);

		/*
		 * Obsolete pages cannot be used on any vCPUs, see the comment
		 * in kvm_mmu_zap_all_fast().  Note, is_obsolete_sp() also
		 * treats invalid shadow pages as being obsolete.
		 */
		zapped_root = !is_obsolete_sp(kvm, sp);
	}

	if (sp->lpage_disallowed)
		unaccount_huge_nx_page(kvm, sp);

	sp->role.invalid = 1;

	/*
	 * Make the request to free obsolete roots after marking the root
	 * invalid, otherwise other vCPUs may not see it as invalid.
	 */
	if (zapped_root)
		kvm_make_all_cpus_request(kvm, KVM_REQ_MMU_FREE_OBSOLETE_ROOTS);
	return list_unstable;
}
{
	bool is_tdp_mmu_fault = is_tdp_mmu(vcpu->arch.mmu);

	unsigned long mmu_seq;
	int r;

	fault->gfn = fault->addr >> PAGE_SHIFT;
	fault->slot = kvm_vcpu_gfn_to_memslot(vcpu, fault->gfn);

	if (page_fault_handle_page_track(vcpu, fault))
		return RET_PF_EMULATE;

	r = fast_page_fault(vcpu, fault);
	if (r != RET_PF_INVALID)
		return r;

	r = mmu_topup_memory_caches(vcpu, false);
	if (r)
		return r;

	mmu_seq = vcpu->kvm->mmu_invalidate_seq;
	smp_rmb();

	r = kvm_faultin_pfn(vcpu, fault);
	if (r != RET_PF_CONTINUE)
		return r;

	r = handle_abnormal_pfn(vcpu, fault, ACC_ALL);
	if (r != RET_PF_CONTINUE)
		return r;

	r = RET_PF_RETRY;

	if (is_tdp_mmu_fault)
		read_lock(&vcpu->kvm->mmu_lock);
	else
		write_lock(&vcpu->kvm->mmu_lock);

	if (is_page_fault_stale(vcpu, fault, mmu_seq))
		goto out_unlock;

	if (is_tdp_mmu_fault) {
		r = kvm_tdp_mmu_map(vcpu, fault);
	} else {
		r = make_mmu_pages_available(vcpu);
		if (r)
			goto out_unlock;
		r = __direct_map(vcpu, fault);
	}

out_unlock:
	if (is_tdp_mmu_fault)
		read_unlock(&vcpu->kvm->mmu_lock);
	else
		write_unlock(&vcpu->kvm->mmu_lock);
	kvm_release_pfn_clean(fault->pfn);
	return r;
}
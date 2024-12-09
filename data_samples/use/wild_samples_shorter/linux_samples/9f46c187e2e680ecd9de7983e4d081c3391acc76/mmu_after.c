	uint i;

	if (pcid == kvm_get_active_pcid(vcpu)) {
		if (mmu->invlpg)
			mmu->invlpg(vcpu, gva, mmu->root.hpa);
		tlb_flush = true;
	}

	for (i = 0; i < KVM_MMU_NUM_PREV_ROOTS; i++) {
		if (VALID_PAGE(mmu->prev_roots[i].hpa) &&
		    pcid == kvm_get_pcid(vcpu, mmu->prev_roots[i].pgd)) {
			if (mmu->invlpg)
				mmu->invlpg(vcpu, gva, mmu->prev_roots[i].hpa);
			tlb_flush = true;
		}
	}

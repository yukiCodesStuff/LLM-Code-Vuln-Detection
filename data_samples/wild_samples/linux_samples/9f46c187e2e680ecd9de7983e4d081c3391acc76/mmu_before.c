{
	struct kvm_mmu *mmu = vcpu->arch.mmu;
	bool tlb_flush = false;
	uint i;

	if (pcid == kvm_get_active_pcid(vcpu)) {
		mmu->invlpg(vcpu, gva, mmu->root.hpa);
		tlb_flush = true;
	}
	if (xchg(&st->preempted, 0) & KVM_VCPU_FLUSH_TLB)
		kvm_vcpu_flush_tlb(vcpu, false);

	vcpu->arch.st.steal.preempted = 0;

	if (st->version & 1)
		st->version += 1;  /* first time write, random junk */

		if (data & KVM_STEAL_RESERVED_MASK)
			return 1;

		if (kvm_gfn_to_hva_cache_init(vcpu->kvm, &vcpu->arch.st.stime,
						data & KVM_STEAL_VALID_BITS,
						sizeof(struct kvm_steal_time)))
			return 1;

		vcpu->arch.st.msr_val = data;

		if (!(data & KVM_MSR_ENABLED))
			break;
	if (!(vcpu->arch.st.msr_val & KVM_MSR_ENABLED))
		return;

	if (vcpu->arch.st.steal.preempted)
		return;

	if (kvm_map_gfn(vcpu, vcpu->arch.st.msr_val >> PAGE_SHIFT, &map,
			&vcpu->arch.st.cache, true))
	st = map.hva +
		offset_in_page(vcpu->arch.st.msr_val & KVM_STEAL_VALID_BITS);

	st->preempted = vcpu->arch.st.steal.preempted = KVM_VCPU_PREEMPTED;

	kvm_unmap_gfn(vcpu, &map, &vcpu->arch.st.cache, true, true);
}
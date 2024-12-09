
static void record_steal_time(struct kvm_vcpu *vcpu)
{
	if (!(vcpu->arch.st.msr_val & KVM_MSR_ENABLED))
		return;

	if (unlikely(kvm_read_guest_cached(vcpu->kvm, &vcpu->arch.st.stime,
		&vcpu->arch.st.steal, sizeof(struct kvm_steal_time))))
		return;

	/*
	 * Doing a TLB flush here, on the guest's behalf, can avoid
	 * expensive IPIs.
	 */
	trace_kvm_pv_tlb_flush(vcpu->vcpu_id,
		vcpu->arch.st.steal.preempted & KVM_VCPU_FLUSH_TLB);
	if (xchg(&vcpu->arch.st.steal.preempted, 0) & KVM_VCPU_FLUSH_TLB)
		kvm_vcpu_flush_tlb(vcpu, false);

	if (vcpu->arch.st.steal.version & 1)
		vcpu->arch.st.steal.version += 1;  /* first time write, random junk */

	vcpu->arch.st.steal.version += 1;

	kvm_write_guest_cached(vcpu->kvm, &vcpu->arch.st.stime,
		&vcpu->arch.st.steal, sizeof(struct kvm_steal_time));

	smp_wmb();

	vcpu->arch.st.steal.steal += current->sched_info.run_delay -
		vcpu->arch.st.last_steal;
	vcpu->arch.st.last_steal = current->sched_info.run_delay;

	kvm_write_guest_cached(vcpu->kvm, &vcpu->arch.st.stime,
		&vcpu->arch.st.steal, sizeof(struct kvm_steal_time));

	smp_wmb();

	vcpu->arch.st.steal.version += 1;

	kvm_write_guest_cached(vcpu->kvm, &vcpu->arch.st.stime,
		&vcpu->arch.st.steal, sizeof(struct kvm_steal_time));
}

int kvm_set_msr_common(struct kvm_vcpu *vcpu, struct msr_data *msr_info)
{
		if (data & KVM_STEAL_RESERVED_MASK)
			return 1;

		if (kvm_gfn_to_hva_cache_init(vcpu->kvm, &vcpu->arch.st.stime,
						data & KVM_STEAL_VALID_BITS,
						sizeof(struct kvm_steal_time)))
			return 1;

		vcpu->arch.st.msr_val = data;

		if (!(data & KVM_MSR_ENABLED))
			break;

static void kvm_steal_time_set_preempted(struct kvm_vcpu *vcpu)
{
	if (!(vcpu->arch.st.msr_val & KVM_MSR_ENABLED))
		return;

	vcpu->arch.st.steal.preempted = KVM_VCPU_PREEMPTED;

	kvm_write_guest_offset_cached(vcpu->kvm, &vcpu->arch.st.stime,
			&vcpu->arch.st.steal.preempted,
			offsetof(struct kvm_steal_time, preempted),
			sizeof(vcpu->arch.st.steal.preempted));
}

void kvm_arch_vcpu_put(struct kvm_vcpu *vcpu)
{

void kvm_arch_vcpu_destroy(struct kvm_vcpu *vcpu)
{
	int idx;

	kvmclock_reset(vcpu);

	kvm_x86_ops->vcpu_free(vcpu);


void kvm_arch_memslots_updated(struct kvm *kvm, u64 gen)
{
	/*
	 * memslots->generation has been incremented.
	 * mmio generation may have reached its maximum value.
	 */
	kvm_mmu_invalidate_mmio_sptes(kvm, gen);
}

int kvm_arch_prepare_memory_region(struct kvm *kvm,
				struct kvm_memory_slot *memslot,
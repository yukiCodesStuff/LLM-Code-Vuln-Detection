
static void record_steal_time(struct kvm_vcpu *vcpu)
{
	struct kvm_host_map map;
	struct kvm_steal_time *st;

	if (!(vcpu->arch.st.msr_val & KVM_MSR_ENABLED))
		return;

	/* -EAGAIN is returned in atomic context so we can just return. */
	if (kvm_map_gfn(vcpu, vcpu->arch.st.msr_val >> PAGE_SHIFT,
			&map, &vcpu->arch.st.cache, false))
		return;

	st = map.hva +
		offset_in_page(vcpu->arch.st.msr_val & KVM_STEAL_VALID_BITS);

	/*
	 * Doing a TLB flush here, on the guest's behalf, can avoid
	 * expensive IPIs.
	 */
	trace_kvm_pv_tlb_flush(vcpu->vcpu_id,
		st->preempted & KVM_VCPU_FLUSH_TLB);
	if (xchg(&st->preempted, 0) & KVM_VCPU_FLUSH_TLB)
		kvm_vcpu_flush_tlb(vcpu, false);

	vcpu->arch.st.preempted = 0;

	if (st->version & 1)
		st->version += 1;  /* first time write, random junk */

	st->version += 1;

	smp_wmb();

	st->steal += current->sched_info.run_delay -
		vcpu->arch.st.last_steal;
	vcpu->arch.st.last_steal = current->sched_info.run_delay;

	smp_wmb();

	st->version += 1;

	kvm_unmap_gfn(vcpu, &map, &vcpu->arch.st.cache, true, false);
}

int kvm_set_msr_common(struct kvm_vcpu *vcpu, struct msr_data *msr_info)
{
		if (data & KVM_STEAL_RESERVED_MASK)
			return 1;

		vcpu->arch.st.msr_val = data;

		if (!(data & KVM_MSR_ENABLED))
			break;

static void kvm_steal_time_set_preempted(struct kvm_vcpu *vcpu)
{
	struct kvm_host_map map;
	struct kvm_steal_time *st;

	if (!(vcpu->arch.st.msr_val & KVM_MSR_ENABLED))
		return;

	if (vcpu->arch.st.preempted)
		return;

	if (kvm_map_gfn(vcpu, vcpu->arch.st.msr_val >> PAGE_SHIFT, &map,
			&vcpu->arch.st.cache, true))
		return;

	st = map.hva +
		offset_in_page(vcpu->arch.st.msr_val & KVM_STEAL_VALID_BITS);

	st->preempted = vcpu->arch.st.preempted = KVM_VCPU_PREEMPTED;

	kvm_unmap_gfn(vcpu, &map, &vcpu->arch.st.cache, true, true);
}

void kvm_arch_vcpu_put(struct kvm_vcpu *vcpu)
{

void kvm_arch_vcpu_destroy(struct kvm_vcpu *vcpu)
{
	struct gfn_to_pfn_cache *cache = &vcpu->arch.st.cache;
	int idx;

	kvm_release_pfn(cache->pfn, cache->dirty, cache);

	kvmclock_reset(vcpu);

	kvm_x86_ops->vcpu_free(vcpu);


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

int kvm_arch_prepare_memory_region(struct kvm *kvm,
				struct kvm_memory_slot *memslot,
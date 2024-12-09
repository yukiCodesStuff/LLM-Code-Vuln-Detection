	unsigned long flags, this_tsc_khz;
	struct kvm_vcpu_arch *vcpu = &v->arch;
	struct kvm_arch *ka = &v->kvm->arch;
	s64 kernel_ns, max_kernel_ns;
	u64 tsc_timestamp, host_tsc;
	struct pvclock_vcpu_time_info guest_hv_clock;
	u8 pvclock_flags;
	bool use_master_clock;

	kernel_ns = 0;

	local_irq_restore(flags);

	if (!vcpu->pv_time_enabled)
		return 0;

	/*
	 * Time as measured by the TSC may go backwards when resetting the base
	 */
	vcpu->hv_clock.version += 2;

	if (unlikely(kvm_read_guest_cached(v->kvm, &vcpu->pv_time,
		&guest_hv_clock, sizeof(guest_hv_clock))))
		return 0;

	/* retain PVCLOCK_GUEST_STOPPED if set in guest copy */
	pvclock_flags = (guest_hv_clock.flags & PVCLOCK_GUEST_STOPPED);

	if (vcpu->pvclock_set_guest_stopped_request) {
		pvclock_flags |= PVCLOCK_GUEST_STOPPED;
		vcpu->pvclock_set_guest_stopped_request = false;

	vcpu->hv_clock.flags = pvclock_flags;

	kvm_write_guest_cached(v->kvm, &vcpu->pv_time,
				&vcpu->hv_clock,
				sizeof(vcpu->hv_clock));
	return 0;
}

static bool msr_mtrr_valid(unsigned msr)

static void kvmclock_reset(struct kvm_vcpu *vcpu)
{
	vcpu->arch.pv_time_enabled = false;
}

static void accumulate_steal_time(struct kvm_vcpu *vcpu)
{
		break;
	case MSR_KVM_SYSTEM_TIME_NEW:
	case MSR_KVM_SYSTEM_TIME: {
		u64 gpa_offset;
		kvmclock_reset(vcpu);

		vcpu->arch.time = data;
		kvm_make_request(KVM_REQ_CLOCK_UPDATE, vcpu);
		if (!(data & 1))
			break;

		gpa_offset = data & ~(PAGE_MASK | 1);

		/* Check that the address is 32-byte aligned. */
		if (gpa_offset & (sizeof(struct pvclock_vcpu_time_info) - 1))
			break;

		if (kvm_gfn_to_hva_cache_init(vcpu->kvm,
		     &vcpu->arch.pv_time, data & ~1ULL))
			vcpu->arch.pv_time_enabled = false;
		else
			vcpu->arch.pv_time_enabled = true;

		break;
	}
	case MSR_KVM_ASYNC_PF_EN:
 */
static int kvm_set_guest_paused(struct kvm_vcpu *vcpu)
{
	if (!vcpu->arch.pv_time_enabled)
		return -EINVAL;
	vcpu->arch.pvclock_set_guest_stopped_request = true;
	kvm_make_request(KVM_REQ_CLOCK_UPDATE, vcpu);
	return 0;
		goto fail_free_wbinvd_dirty_mask;

	vcpu->arch.ia32_tsc_adjust_msr = 0x0;
	vcpu->arch.pv_time_enabled = false;
	kvm_async_pf_hash_reset(vcpu);
	kvm_pmu_init(vcpu);

	return 0;
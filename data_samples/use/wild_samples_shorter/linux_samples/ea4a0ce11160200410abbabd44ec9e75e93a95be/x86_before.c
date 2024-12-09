	unsigned long flags, this_tsc_khz;
	struct kvm_vcpu_arch *vcpu = &v->arch;
	struct kvm_arch *ka = &v->kvm->arch;
	void *shared_kaddr;
	s64 kernel_ns, max_kernel_ns;
	u64 tsc_timestamp, host_tsc;
	struct pvclock_vcpu_time_info *guest_hv_clock;
	u8 pvclock_flags;
	bool use_master_clock;

	kernel_ns = 0;
	host_tsc = 0;

	/* Keep irq disabled to prevent changes to the clock */
	local_irq_save(flags);
	this_tsc_khz = __get_cpu_var(cpu_tsc_khz);
	if (unlikely(this_tsc_khz == 0)) {
		local_irq_restore(flags);
		kvm_make_request(KVM_REQ_CLOCK_UPDATE, v);
		return 1;
	}

	/*
	 * If the host uses TSC clock, then passthrough TSC as stable
	 * to the guest.
	 */
		kernel_ns = ka->master_kernel_ns;
	}
	spin_unlock(&ka->pvclock_gtod_sync_lock);
	if (!use_master_clock) {
		host_tsc = native_read_tsc();
		kernel_ns = get_kernel_ns();
	}

	local_irq_restore(flags);

	if (!vcpu->time_page)
		return 0;

	/*
	 * Time as measured by the TSC may go backwards when resetting the base
	 */
	vcpu->hv_clock.version += 2;

	shared_kaddr = kmap_atomic(vcpu->time_page);

	guest_hv_clock = shared_kaddr + vcpu->time_offset;

	/* retain PVCLOCK_GUEST_STOPPED if set in guest copy */
	pvclock_flags = (guest_hv_clock->flags & PVCLOCK_GUEST_STOPPED);

	if (vcpu->pvclock_set_guest_stopped_request) {
		pvclock_flags |= PVCLOCK_GUEST_STOPPED;
		vcpu->pvclock_set_guest_stopped_request = false;

	vcpu->hv_clock.flags = pvclock_flags;

	memcpy(shared_kaddr + vcpu->time_offset, &vcpu->hv_clock,
	       sizeof(vcpu->hv_clock));

	kunmap_atomic(shared_kaddr);

	mark_page_dirty(v->kvm, vcpu->time >> PAGE_SHIFT);
	return 0;
}

static bool msr_mtrr_valid(unsigned msr)

static void kvmclock_reset(struct kvm_vcpu *vcpu)
{
	if (vcpu->arch.time_page) {
		kvm_release_page_dirty(vcpu->arch.time_page);
		vcpu->arch.time_page = NULL;
	}
}

static void accumulate_steal_time(struct kvm_vcpu *vcpu)
{
		break;
	case MSR_KVM_SYSTEM_TIME_NEW:
	case MSR_KVM_SYSTEM_TIME: {
		kvmclock_reset(vcpu);

		vcpu->arch.time = data;
		kvm_make_request(KVM_REQ_CLOCK_UPDATE, vcpu);
		if (!(data & 1))
			break;

		/* ...but clean it before doing the actual write */
		vcpu->arch.time_offset = data & ~(PAGE_MASK | 1);

		vcpu->arch.time_page =
				gfn_to_page(vcpu->kvm, data >> PAGE_SHIFT);

		if (is_error_page(vcpu->arch.time_page))
			vcpu->arch.time_page = NULL;

		break;
	}
	case MSR_KVM_ASYNC_PF_EN:
 */
static int kvm_set_guest_paused(struct kvm_vcpu *vcpu)
{
	if (!vcpu->arch.time_page)
		return -EINVAL;
	vcpu->arch.pvclock_set_guest_stopped_request = true;
	kvm_make_request(KVM_REQ_CLOCK_UPDATE, vcpu);
	return 0;
		goto fail_free_wbinvd_dirty_mask;

	vcpu->arch.ia32_tsc_adjust_msr = 0x0;
	kvm_async_pf_hash_reset(vcpu);
	kvm_pmu_init(vcpu);

	return 0;
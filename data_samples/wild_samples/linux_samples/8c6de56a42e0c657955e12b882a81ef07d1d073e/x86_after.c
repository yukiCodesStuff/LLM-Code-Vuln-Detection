{
	if (!(vcpu->arch.st.msr_val & KVM_MSR_ENABLED))
		return;

	if (vcpu->arch.st.steal.preempted)
		return;

	vcpu->arch.st.steal.preempted = KVM_VCPU_PREEMPTED;

	kvm_write_guest_offset_cached(vcpu->kvm, &vcpu->arch.st.stime,
			&vcpu->arch.st.steal.preempted,
			offsetof(struct kvm_steal_time, preempted),
			sizeof(vcpu->arch.st.steal.preempted));
}

void kvm_arch_vcpu_put(struct kvm_vcpu *vcpu)
{
	int idx;

	if (vcpu->preempted)
		vcpu->arch.preempted_in_kernel = !kvm_x86_ops->get_cpl(vcpu);

	/*
	 * Disable page faults because we're in atomic context here.
	 * kvm_write_guest_offset_cached() would call might_fault()
	 * that relies on pagefault_disable() to tell if there's a
	 * bug. NOTE: the write to guest memory may not go through if
	 * during postcopy live migration or if there's heavy guest
	 * paging.
	 */
	pagefault_disable();
	/*
	 * kvm_memslots() will be called by
	 * kvm_write_guest_offset_cached() so take the srcu lock.
	 */
	idx = srcu_read_lock(&vcpu->kvm->srcu);
	kvm_steal_time_set_preempted(vcpu);
	srcu_read_unlock(&vcpu->kvm->srcu, idx);
	pagefault_enable();
	kvm_x86_ops->vcpu_put(vcpu);
	vcpu->arch.last_host_tsc = rdtsc();
	/*
	 * If userspace has set any breakpoints or watchpoints, dr6 is restored
	 * on every vmexit, but if not, we might have a stale dr6 from the
	 * guest. do_debug expects dr6 to be cleared after it runs, do the same.
	 */
	set_debugreg(0, 6);
}

static int kvm_vcpu_ioctl_get_lapic(struct kvm_vcpu *vcpu,
				    struct kvm_lapic_state *s)
{
	if (vcpu->arch.apicv_active)
		kvm_x86_ops->sync_pir_to_irr(vcpu);

	return kvm_apic_get_state(vcpu, s);
}

static int kvm_vcpu_ioctl_set_lapic(struct kvm_vcpu *vcpu,
				    struct kvm_lapic_state *s)
{
	int r;

	r = kvm_apic_set_state(vcpu, s);
	if (r)
		return r;
	update_cr8_intercept(vcpu);

	return 0;
}

static int kvm_cpu_accept_dm_intr(struct kvm_vcpu *vcpu)
{
	return (!lapic_in_kernel(vcpu) ||
		kvm_apic_accept_pic_intr(vcpu));
}

/*
 * if userspace requested an interrupt window, check that the
 * interrupt window is open.
 *
 * No need to exit to userspace if we already have an interrupt queued.
 */
static int kvm_vcpu_ready_for_interrupt_injection(struct kvm_vcpu *vcpu)
{
	return kvm_arch_interrupt_allowed(vcpu) &&
		!kvm_cpu_has_interrupt(vcpu) &&
		!kvm_event_needs_reinjection(vcpu) &&
		kvm_cpu_accept_dm_intr(vcpu);
}

static int kvm_vcpu_ioctl_interrupt(struct kvm_vcpu *vcpu,
				    struct kvm_interrupt *irq)
{
	if (irq->irq >= KVM_NR_INTERRUPTS)
		return -EINVAL;

	if (!irqchip_in_kernel(vcpu->kvm)) {
		kvm_queue_interrupt(vcpu, irq->irq, false);
		kvm_make_request(KVM_REQ_EVENT, vcpu);
		return 0;
	}
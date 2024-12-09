{
	struct mips_coproc *cop0 = vcpu->arch.cop0;
	int cpu = smp_processor_id();
	unsigned int gasid;

	/*
	 * Lazy host ASID regeneration for guest user mode.
	 * If the guest ASID has changed since the last guest usermode
	 * execution, regenerate the host ASID so as to invalidate stale TLB
	 * entries.
	 */
	if (!KVM_GUEST_KERNEL_MODE(vcpu)) {
		gasid = kvm_read_c0_guest_entryhi(cop0) & KVM_ENTRYHI_ASID;
		if (gasid != vcpu->arch.last_user_gasid) {
			kvm_get_new_mmu_context(&vcpu->arch.guest_user_mm, cpu,
						vcpu);
			vcpu->arch.guest_user_asid[cpu] =
				vcpu->arch.guest_user_mm.context.asid[cpu];
			vcpu->arch.last_user_gasid = gasid;
		}
	}
}
		if (gasid != vcpu->arch.last_user_gasid) {
			kvm_get_new_mmu_context(&vcpu->arch.guest_user_mm, cpu,
						vcpu);
			vcpu->arch.guest_user_asid[cpu] =
				vcpu->arch.guest_user_mm.context.asid[cpu];
			vcpu->arch.last_user_gasid = gasid;
		}
	}
}

int kvm_arch_vcpu_ioctl_run(struct kvm_vcpu *vcpu, struct kvm_run *run)
{
	int r = 0;
	sigset_t sigsaved;

	if (vcpu->sigset_active)
		sigprocmask(SIG_SETMASK, &vcpu->sigset, &sigsaved);

	if (vcpu->mmio_needed) {
		if (!vcpu->mmio_is_write)
			kvm_mips_complete_mmio_load(vcpu, run);
		vcpu->mmio_needed = 0;
	}

	lose_fpu(1);

	local_irq_disable();
	/* Check if we have any exceptions/interrupts pending */
	kvm_mips_deliver_interrupts(vcpu,
				    kvm_read_c0_guest_cause(vcpu->arch.cop0));

	guest_enter_irqoff();

	/* Disable hardware page table walking while in guest */
	htw_stop();

	trace_kvm_enter(vcpu);

	kvm_mips_check_asids(vcpu);

	r = vcpu->arch.vcpu_run(run, vcpu);
	trace_kvm_out(vcpu);

	/* Re-enable HTW before enabling interrupts */
	htw_start();

	guest_exit_irqoff();
	local_irq_enable();

	if (vcpu->sigset_active)
		sigprocmask(SIG_SETMASK, &sigsaved, NULL);

	return r;
}

int kvm_vcpu_ioctl_interrupt(struct kvm_vcpu *vcpu,
			     struct kvm_mips_interrupt *irq)
{
	int intr = (int)irq->irq;
	struct kvm_vcpu *dvcpu = NULL;

	if (intr == 3 || intr == -3 || intr == 4 || intr == -4)
		kvm_debug("%s: CPU: %d, INTR: %d\n", __func__, irq->cpu,
			  (int)intr);

	if (irq->cpu == -1)
		dvcpu = vcpu;
	else
		dvcpu = vcpu->kvm->vcpus[irq->cpu];

	if (intr == 2 || intr == 3 || intr == 4) {
		kvm_mips_callbacks->queue_io_int(dvcpu, irq);

	} else if (intr == -2 || intr == -3 || intr == -4) {
		kvm_mips_callbacks->dequeue_io_int(dvcpu, irq);
	} else {
		kvm_err("%s: invalid interrupt ioctl (%d:%d)\n", __func__,
			irq->cpu, irq->irq);
		return -EINVAL;
	}
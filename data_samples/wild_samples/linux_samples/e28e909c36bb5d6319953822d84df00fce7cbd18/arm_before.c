{
	struct kvm *kvm = vcpu->kvm;
	int ret;

	if (likely(vcpu->arch.has_run_once))
		return 0;

	vcpu->arch.has_run_once = true;

	/*
	 * Map the VGIC hardware resources before running a vcpu the first
	 * time on this VM.
	 */
	if (unlikely(irqchip_in_kernel(kvm) && !vgic_ready(kvm))) {
		ret = kvm_vgic_map_resources(kvm);
		if (ret)
			return ret;
	}

	/*
	 * Enable the arch timers only if we have an in-kernel VGIC
	 * and it has been properly initialized, since we cannot handle
	 * interrupts from the virtual timer with a userspace gic.
	 */
	if (irqchip_in_kernel(kvm) && vgic_initialized(kvm))
		kvm_timer_enable(kvm);

	return 0;
}
	if (unlikely(irqchip_in_kernel(kvm) && !vgic_ready(kvm))) {
		ret = kvm_vgic_map_resources(kvm);
		if (ret)
			return ret;
	}

	/*
	 * Enable the arch timers only if we have an in-kernel VGIC
	 * and it has been properly initialized, since we cannot handle
	 * interrupts from the virtual timer with a userspace gic.
	 */
	if (irqchip_in_kernel(kvm) && vgic_initialized(kvm))
		kvm_timer_enable(kvm);

	return 0;
}

bool kvm_arch_intc_initialized(struct kvm *kvm)
{
	return vgic_initialized(kvm);
}

static void kvm_arm_halt_guest(struct kvm *kvm) __maybe_unused;
static void kvm_arm_resume_guest(struct kvm *kvm) __maybe_unused;

static void kvm_arm_halt_guest(struct kvm *kvm)
{
	int i;
	struct kvm_vcpu *vcpu;

	kvm_for_each_vcpu(i, vcpu, kvm)
		vcpu->arch.pause = true;
	force_vm_exit(cpu_all_mask);
}

static void kvm_arm_resume_guest(struct kvm *kvm)
{
	int i;
	struct kvm_vcpu *vcpu;

	kvm_for_each_vcpu(i, vcpu, kvm) {
		struct swait_queue_head *wq = kvm_arch_vcpu_wq(vcpu);

		vcpu->arch.pause = false;
		swake_up(wq);
	}
{
	struct kvm *kvm = vcpu->kvm;
	int ret = 0;

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
		ret = kvm_timer_enable(vcpu);

	return ret;
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
		ret = kvm_timer_enable(vcpu);

	return ret;
}

bool kvm_arch_intc_initialized(struct kvm *kvm)
{
	return vgic_initialized(kvm);
}

void kvm_arm_halt_guest(struct kvm *kvm)
{
	int i;
	struct kvm_vcpu *vcpu;

	kvm_for_each_vcpu(i, vcpu, kvm)
		vcpu->arch.pause = true;
	kvm_make_all_cpus_request(kvm, KVM_REQ_VCPU_EXIT);
}

void kvm_arm_halt_vcpu(struct kvm_vcpu *vcpu)
{
	vcpu->arch.pause = true;
	kvm_vcpu_kick(vcpu);
}

void kvm_arm_resume_vcpu(struct kvm_vcpu *vcpu)
{
	struct swait_queue_head *wq = kvm_arch_vcpu_wq(vcpu);

	vcpu->arch.pause = false;
	swake_up(wq);
}

void kvm_arm_resume_guest(struct kvm *kvm)
{
	int i;
	struct kvm_vcpu *vcpu;

	kvm_for_each_vcpu(i, vcpu, kvm)
		kvm_arm_resume_vcpu(vcpu);
}

static void vcpu_sleep(struct kvm_vcpu *vcpu)
{
	struct swait_queue_head *wq = kvm_arch_vcpu_wq(vcpu);

	swait_event_interruptible(*wq, ((!vcpu->arch.power_off) &&
				       (!vcpu->arch.pause)));
}

static int kvm_vcpu_initialized(struct kvm_vcpu *vcpu)
{
	return vcpu->arch.target >= 0;
}

/**
 * kvm_arch_vcpu_ioctl_run - the main VCPU run function to execute guest code
 * @vcpu:	The VCPU pointer
 * @run:	The kvm_run structure pointer used for userspace state exchange
 *
 * This function is called through the VCPU_RUN ioctl called from user space. It
 * will execute VM code in a loop until the time slice for the process is used
 * or some emulation is needed from user space in which case the function will
 * return with return value 0 and with the kvm_run structure filled in with the
 * required data for the requested emulation.
 */
int kvm_arch_vcpu_ioctl_run(struct kvm_vcpu *vcpu, struct kvm_run *run)
{
	int ret;
	sigset_t sigsaved;

	if (unlikely(!kvm_vcpu_initialized(vcpu)))
		return -ENOEXEC;

	ret = kvm_vcpu_first_run_init(vcpu);
	if (ret)
		return ret;

	if (run->exit_reason == KVM_EXIT_MMIO) {
		ret = kvm_handle_mmio_return(vcpu, vcpu->run);
		if (ret)
			return ret;
	}
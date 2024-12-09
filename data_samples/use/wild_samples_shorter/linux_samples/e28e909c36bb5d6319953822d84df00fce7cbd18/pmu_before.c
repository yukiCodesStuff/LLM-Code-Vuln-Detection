	return 0;
}

static bool irq_is_valid(struct kvm *kvm, int irq, bool is_ppi)
{
	int i;
	struct kvm_vcpu *vcpu;

		if (!kvm_arm_pmu_irq_initialized(vcpu))
			continue;

		if (is_ppi) {
			if (vcpu->arch.pmu.irq_num != irq)
				return false;
		} else {
			if (vcpu->arch.pmu.irq_num == irq)
	return true;
}


int kvm_arm_pmu_v3_set_attr(struct kvm_vcpu *vcpu, struct kvm_device_attr *attr)
{
	switch (attr->attr) {
	case KVM_ARM_VCPU_PMU_V3_IRQ: {
		if (get_user(irq, uaddr))
			return -EFAULT;

		/*
		 * The PMU overflow interrupt could be a PPI or SPI, but for one
		 * VM the interrupt type must be same for each vcpu. As a PPI,
		 * the interrupt number is the same for all vcpus, while as an
		 * SPI it must be a separate number per vcpu.
		 */
		if (irq < VGIC_NR_SGIS || irq >= vcpu->kvm->arch.vgic.nr_irqs ||
		    !irq_is_valid(vcpu->kvm, irq, irq < VGIC_NR_PRIVATE_IRQS))
			return -EINVAL;

		if (kvm_arm_pmu_irq_initialized(vcpu))
			return -EBUSY;
{
	u8 val;
	if (pv_eoi_get_user(vcpu, &val) < 0) {
		printk(KERN_WARNING "Can't read EOI MSR value: 0x%llx\n",
			   (unsigned long long)vcpu->arch.pv_eoi.msr_val);
		return false;
	}
	return val & 0x1;
}
		if (apic_test_vector(vector, apic->regs + APIC_TMR) != !!trig_mode) {
			if (trig_mode)
				kvm_lapic_set_vector(vector,
						     apic->regs + APIC_TMR);
			else
				kvm_lapic_clear_vector(vector,
						       apic->regs + APIC_TMR);
		}

		if (kvm_x86_ops->deliver_posted_interrupt(vcpu, vector)) {
			kvm_lapic_set_irr(vector, apic);
			kvm_make_request(KVM_REQ_EVENT, vcpu);
			kvm_vcpu_kick(vcpu);
		}
		break;

	case APIC_DM_REMRD:
		result = 1;
		vcpu->arch.pv.pv_unhalted = 1;
		kvm_make_request(KVM_REQ_EVENT, vcpu);
		kvm_vcpu_kick(vcpu);
		break;

	case APIC_DM_SMI:
		result = 1;
		kvm_make_request(KVM_REQ_SMI, vcpu);
		kvm_vcpu_kick(vcpu);
		break;

	case APIC_DM_NMI:
		result = 1;
		kvm_inject_nmi(vcpu);
		kvm_vcpu_kick(vcpu);
		break;

	case APIC_DM_INIT:
		if (!trig_mode || level) {
			result = 1;
			/* assumes that there are only KVM_APIC_INIT/SIPI */
			apic->pending_events = (1UL << KVM_APIC_INIT);
			kvm_make_request(KVM_REQ_EVENT, vcpu);
			kvm_vcpu_kick(vcpu);
		}
		break;

	case APIC_DM_STARTUP:
		result = 1;
		apic->sipi_vector = vector;
		/* make sure sipi_vector is visible for the receiver */
		smp_wmb();
		set_bit(KVM_APIC_SIPI, &apic->pending_events);
		kvm_make_request(KVM_REQ_EVENT, vcpu);
		kvm_vcpu_kick(vcpu);
		break;

	case APIC_DM_EXTINT:
		/*
		 * Should only be called by kvm_apic_local_deliver() with LVT0,
		 * before NMI watchdog was enabled. Already handled by
		 * kvm_apic_accept_pic_intr().
		 */
		break;

	default:
		printk(KERN_ERR "TODO: unsupported delivery mode %x\n",
		       delivery_mode);
		break;
	}
	return result;
}

/*
 * This routine identifies the destination vcpus mask meant to receive the
 * IOAPIC interrupts. It either uses kvm_apic_map_get_dest_lapic() to find
 * out the destination vcpus array and set the bitmap or it traverses to
 * each available vcpu to identify the same.
 */
void kvm_bitmap_or_dest_vcpus(struct kvm *kvm, struct kvm_lapic_irq *irq,
			      unsigned long *vcpu_bitmap)
{
	struct kvm_lapic **dest_vcpu = NULL;
	struct kvm_lapic *src = NULL;
	struct kvm_apic_map *map;
	struct kvm_vcpu *vcpu;
	unsigned long bitmap;
	int i, vcpu_idx;
	bool ret;

	rcu_read_lock();
	map = rcu_dereference(kvm->arch.apic_map);

	ret = kvm_apic_map_get_dest_lapic(kvm, &src, irq, map, &dest_vcpu,
					  &bitmap);
	if (ret) {
		for_each_set_bit(i, &bitmap, 16) {
			if (!dest_vcpu[i])
				continue;
			vcpu_idx = dest_vcpu[i]->vcpu->vcpu_idx;
			__set_bit(vcpu_idx, vcpu_bitmap);
		}
	} else {
		kvm_for_each_vcpu(i, vcpu, kvm) {
			if (!kvm_apic_present(vcpu))
				continue;
			if (!kvm_apic_match_dest(vcpu, NULL,
						 irq->shorthand,
						 irq->dest_id,
						 irq->dest_mode))
				continue;
			__set_bit(i, vcpu_bitmap);
		}
static bool pv_eoi_get_pending(struct kvm_vcpu *vcpu)
{
	u8 val;
	if (pv_eoi_get_user(vcpu, &val) < 0)
		printk(KERN_WARNING "Can't read EOI MSR value: 0x%llx\n",
			   (unsigned long long)vcpu->arch.pv_eoi.msr_val);
	return val & 0x1;
}

static void pv_eoi_set_pending(struct kvm_vcpu *vcpu)
						       apic->regs + APIC_TMR);
		}

		if (vcpu->arch.apicv_active)
			kvm_x86_ops->deliver_posted_interrupt(vcpu, vector);
		else {
			kvm_lapic_set_irr(vector, apic);

			kvm_make_request(KVM_REQ_EVENT, vcpu);
			kvm_vcpu_kick(vcpu);
		}
		break;
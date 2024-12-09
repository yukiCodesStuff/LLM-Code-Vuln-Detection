	bitmap_zero(ioapic->rtc_status.dest_map, KVM_MAX_VCPUS);
}

static void __rtc_irq_eoi_tracking_restore_one(struct kvm_vcpu *vcpu)
{
	bool new_val, old_val;
	struct kvm_ioapic *ioapic = vcpu->kvm->arch.vioapic;
	} else {
		__clear_bit(vcpu->vcpu_id, ioapic->rtc_status.dest_map);
		ioapic->rtc_status.pending_eoi--;
	}

	WARN_ON(ioapic->rtc_status.pending_eoi < 0);
}

void kvm_rtc_eoi_tracking_restore_one(struct kvm_vcpu *vcpu)
{

static void rtc_irq_eoi(struct kvm_ioapic *ioapic, struct kvm_vcpu *vcpu)
{
	if (test_and_clear_bit(vcpu->vcpu_id, ioapic->rtc_status.dest_map))
		--ioapic->rtc_status.pending_eoi;

	WARN_ON(ioapic->rtc_status.pending_eoi < 0);
}

static bool rtc_irq_check_coalesced(struct kvm_ioapic *ioapic)
{
		ioapic->irr &= ~(1 << irq);

	if (irq == RTC_GSI && line_status) {
		BUG_ON(ioapic->rtc_status.pending_eoi != 0);
		ret = kvm_irq_delivery_to_apic(ioapic->kvm, NULL, &irqe,
				ioapic->rtc_status.dest_map);
		ioapic->rtc_status.pending_eoi = ret;
	} else
		ret = kvm_irq_delivery_to_apic(ioapic->kvm, NULL, &irqe, NULL);

	if (ret && irqe.trig_mode == IOAPIC_LEVEL_TRIG)
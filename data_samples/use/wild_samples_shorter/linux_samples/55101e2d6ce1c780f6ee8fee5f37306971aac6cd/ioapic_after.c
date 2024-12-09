	bitmap_zero(ioapic->rtc_status.dest_map, KVM_MAX_VCPUS);
}

static void kvm_rtc_eoi_tracking_restore_all(struct kvm_ioapic *ioapic);

static void rtc_status_pending_eoi_check_valid(struct kvm_ioapic *ioapic)
{
	if (WARN_ON(ioapic->rtc_status.pending_eoi < 0))
		kvm_rtc_eoi_tracking_restore_all(ioapic);
}

static void __rtc_irq_eoi_tracking_restore_one(struct kvm_vcpu *vcpu)
{
	bool new_val, old_val;
	struct kvm_ioapic *ioapic = vcpu->kvm->arch.vioapic;
	} else {
		__clear_bit(vcpu->vcpu_id, ioapic->rtc_status.dest_map);
		ioapic->rtc_status.pending_eoi--;
		rtc_status_pending_eoi_check_valid(ioapic);
	}
}

void kvm_rtc_eoi_tracking_restore_one(struct kvm_vcpu *vcpu)
{

static void rtc_irq_eoi(struct kvm_ioapic *ioapic, struct kvm_vcpu *vcpu)
{
	if (test_and_clear_bit(vcpu->vcpu_id, ioapic->rtc_status.dest_map)) {
		--ioapic->rtc_status.pending_eoi;
		rtc_status_pending_eoi_check_valid(ioapic);
	}
}

static bool rtc_irq_check_coalesced(struct kvm_ioapic *ioapic)
{
		ioapic->irr &= ~(1 << irq);

	if (irq == RTC_GSI && line_status) {
		/*
		 * pending_eoi cannot ever become negative (see
		 * rtc_status_pending_eoi_check_valid) and the caller
		 * ensures that it is only called if it is >= zero, namely
		 * if rtc_irq_check_coalesced returns false).
		 */
		BUG_ON(ioapic->rtc_status.pending_eoi != 0);
		ret = kvm_irq_delivery_to_apic(ioapic->kvm, NULL, &irqe,
				ioapic->rtc_status.dest_map);
		ioapic->rtc_status.pending_eoi = (ret < 0 ? 0 : ret);
	} else
		ret = kvm_irq_delivery_to_apic(ioapic->kvm, NULL, &irqe, NULL);

	if (ret && irqe.trig_mode == IOAPIC_LEVEL_TRIG)
	kvm_for_each_vcpu(i, vcpu, ioapic->kvm) {
		if (!kvm_apic_match_dest(vcpu, NULL, APIC_DEST_NOSHORT,
					 entry->fields.dest_id,
					 entry->fields.dest_mode) ||
		    kvm_apic_pending_eoi(vcpu, entry->fields.vector))
			continue;

		/*
		 * If no longer has pending EOI in LAPICs, update
		 * EOI for this vector.
		 */
		rtc_irq_eoi(ioapic, vcpu, entry->fields.vector);
		break;
	}
}

static int ioapic_set_irq(struct kvm_ioapic *ioapic, unsigned int irq,
		int irq_level, bool line_status)
{
	union kvm_ioapic_redirect_entry entry;
	u32 mask = 1 << irq;
	u32 old_irr;
	int edge, ret;

	entry = ioapic->redirtbl[irq];
	edge = (entry.fields.trig_mode == IOAPIC_EDGE_TRIG);

	if (!irq_level) {
		ioapic->irr &= ~mask;
		ret = 1;
		goto out;
	}

	/*
	 * AMD SVM AVIC accelerate EOI write iff the interrupt is edge
	 * triggered, in which case the in-kernel IOAPIC will not be able
	 * to receive the EOI.  In this case, we do a lazy update of the
	 * pending EOI when trying to set IOAPIC irq.
	 */
	if (edge && kvm_apicv_activated(ioapic->kvm))
		ioapic_lazy_update_eoi(ioapic, irq);

	/*
	 * Return 0 for coalesced interrupts; for edge-triggered interrupts,
	 * this only happens if a previous edge has not been delivered due
	 * to masking.  For level interrupts, the remote_irr field tells
	 * us if the interrupt is waiting for an EOI.
	 *
	 * RTC is special: it is edge-triggered, but userspace likes to know
	 * if it has been already ack-ed via EOI because coalesced RTC
	 * interrupts lead to time drift in Windows guests.  So we track
	 * EOI manually for the RTC interrupt.
	 */
	if (irq == RTC_GSI && line_status &&
		rtc_irq_check_coalesced(ioapic)) {
		ret = 0;
		goto out;
	}

	old_irr = ioapic->irr;
	ioapic->irr |= mask;
	if (edge) {
		ioapic->irr_delivered &= ~mask;
		if (old_irr == ioapic->irr) {
			ret = 0;
			goto out;
		}
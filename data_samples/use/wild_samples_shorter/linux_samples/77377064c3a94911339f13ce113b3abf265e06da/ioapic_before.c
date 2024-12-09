
		/*
		 * If no longer has pending EOI in LAPICs, update
		 * EOI for this vetor.
		 */
		rtc_irq_eoi(ioapic, vcpu, entry->fields.vector);
		kvm_ioapic_update_eoi_one(vcpu, ioapic,
					  entry->fields.trig_mode,
					  irq);
		break;
	}
}

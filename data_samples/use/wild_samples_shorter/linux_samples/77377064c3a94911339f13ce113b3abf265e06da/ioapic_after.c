
		/*
		 * If no longer has pending EOI in LAPICs, update
		 * EOI for this vector.
		 */
		rtc_irq_eoi(ioapic, vcpu, entry->fields.vector);
		break;
	}
}

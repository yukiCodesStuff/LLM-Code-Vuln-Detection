	if (!r) {
		dev->irq_requested_type |= guest_irq_type;
		if (dev->ack_notifier.gsi != -1)
			kvm_register_irq_ack_notifier(kvm, &dev->ack_notifier);
	} else {
		kvm_free_irq_source_id(kvm, dev->irq_source_id);
		dev->irq_source_id = -1;
	}
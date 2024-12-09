
			kvm_set_msi_irq(vcpu->kvm, entry, &irq);

			if (irq.trig_mode &&
			    kvm_apic_match_dest(vcpu, NULL, APIC_DEST_NOSHORT,
						irq.dest_id, irq.dest_mode))
				__set_bit(irq.vector, ioapic_handled_vectors);
		}
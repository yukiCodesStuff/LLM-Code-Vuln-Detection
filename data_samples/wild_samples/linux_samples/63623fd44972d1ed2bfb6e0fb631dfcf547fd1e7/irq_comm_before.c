		hlist_for_each_entry(entry, &table->map[i], link) {
			struct kvm_lapic_irq irq;

			if (entry->type != KVM_IRQ_ROUTING_MSI)
				continue;

			kvm_set_msi_irq(vcpu->kvm, entry, &irq);

			if (irq.level &&
			    kvm_apic_match_dest(vcpu, NULL, APIC_DEST_NOSHORT,
						irq.dest_id, irq.dest_mode))
				__set_bit(irq.vector, ioapic_handled_vectors);
		}
	}
	srcu_read_unlock(&kvm->irq_srcu, idx);
}

void kvm_arch_irq_routing_update(struct kvm *kvm)
{
	kvm_hv_irq_routing_update(kvm);
}
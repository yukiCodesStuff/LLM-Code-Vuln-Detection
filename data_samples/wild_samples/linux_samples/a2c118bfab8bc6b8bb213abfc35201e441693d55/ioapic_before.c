		{
			u32 redir_index = (ioapic->ioregsel - 0x10) >> 1;
			u64 redir_content;

			ASSERT(redir_index < IOAPIC_NUM_PINS);

			redir_content = ioapic->redirtbl[redir_index].bits;
			result = (ioapic->ioregsel & 0x1) ?
			    (redir_content >> 32) & 0xffffffff :
			    redir_content & 0xffffffff;
			break;
		}
	}

	return result;
}

static int ioapic_service(struct kvm_ioapic *ioapic, unsigned int idx)
{
	union kvm_ioapic_redirect_entry *pent;
	int injected = -1;

	pent = &ioapic->redirtbl[idx];

	if (!pent->fields.mask) {
		injected = ioapic_deliver(ioapic, idx);
		if (injected && pent->fields.trig_mode == IOAPIC_LEVEL_TRIG)
			pent->fields.remote_irr = 1;
	}

	return injected;
}

static void update_handled_vectors(struct kvm_ioapic *ioapic)
{
	DECLARE_BITMAP(handled_vectors, 256);
	int i;

	memset(handled_vectors, 0, sizeof(handled_vectors));
	for (i = 0; i < IOAPIC_NUM_PINS; ++i)
		__set_bit(ioapic->redirtbl[i].fields.vector, handled_vectors);
	memcpy(ioapic->handled_vectors, handled_vectors,
	       sizeof(handled_vectors));
	smp_wmb();
}

void kvm_ioapic_calculate_eoi_exitmap(struct kvm_vcpu *vcpu,
					u64 *eoi_exit_bitmap)
{
	struct kvm_ioapic *ioapic = vcpu->kvm->arch.vioapic;
	union kvm_ioapic_redirect_entry *e;
	struct kvm_lapic_irq irqe;
	int index;

	spin_lock(&ioapic->lock);
	/* traverse ioapic entry to set eoi exit bitmap*/
	for (index = 0; index < IOAPIC_NUM_PINS; index++) {
		e = &ioapic->redirtbl[index];
		if (!e->fields.mask &&
			(e->fields.trig_mode == IOAPIC_LEVEL_TRIG ||
			 kvm_irq_has_notifier(ioapic->kvm, KVM_IRQCHIP_IOAPIC,
				 index))) {
			irqe.dest_id = e->fields.dest_id;
			irqe.vector = e->fields.vector;
			irqe.dest_mode = e->fields.dest_mode;
			irqe.delivery_mode = e->fields.delivery_mode << 8;
			kvm_calculate_eoi_exitmap(vcpu, &irqe, eoi_exit_bitmap);
		}
	}
	spin_unlock(&ioapic->lock);
}
EXPORT_SYMBOL_GPL(kvm_ioapic_calculate_eoi_exitmap);

void kvm_ioapic_make_eoibitmap_request(struct kvm *kvm)
{
	struct kvm_ioapic *ioapic = kvm->arch.vioapic;

	if (!kvm_apic_vid_enabled(kvm) || !ioapic)
		return;
	kvm_make_update_eoibitmap_request(kvm);
}

static void ioapic_write_indirect(struct kvm_ioapic *ioapic, u32 val)
{
	unsigned index;
	bool mask_before, mask_after;
	union kvm_ioapic_redirect_entry *e;

	switch (ioapic->ioregsel) {
	case IOAPIC_REG_VERSION:
		/* Writes are ignored. */
		break;

	case IOAPIC_REG_APIC_ID:
		ioapic->id = (val >> 24) & 0xf;
		break;

	case IOAPIC_REG_ARB_ID:
		break;

	default:
		index = (ioapic->ioregsel - 0x10) >> 1;

		ioapic_debug("change redir index %x val %x\n", index, val);
		if (index >= IOAPIC_NUM_PINS)
			return;
		e = &ioapic->redirtbl[index];
		mask_before = e->fields.mask;
		if (ioapic->ioregsel & 1) {
			e->bits &= 0xffffffff;
			e->bits |= (u64) val << 32;
		} else {
			e->bits &= ~0xffffffffULL;
			e->bits |= (u32) val;
			e->fields.remote_irr = 0;
		}
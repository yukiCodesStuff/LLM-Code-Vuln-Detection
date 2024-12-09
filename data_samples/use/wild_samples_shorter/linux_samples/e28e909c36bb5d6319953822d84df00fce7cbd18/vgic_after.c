 */
void vgic_unqueue_irqs(struct kvm_vcpu *vcpu)
{
	u64 elrsr = vgic_get_elrsr(vcpu);
	unsigned long *elrsr_ptr = u64_to_bitmask(&elrsr);
	int i;

	for_each_clear_bit(i, elrsr_ptr, vgic->nr_lr) {
		struct vgic_lr lr = vgic_get_lr(vcpu, i);

		/*
		 * There are three options for the state bits:
	struct vgic_dist *dist = &vcpu->kvm->arch.vgic;
	struct vgic_io_device *iodev = container_of(this,
						    struct vgic_io_device, dev);
	const struct vgic_io_range *range;
	struct kvm_exit_mmio mmio;
	bool updated_state;
	gpa_t offset;
		updated_state = false;
	}
	spin_unlock(&dist->lock);

	if (updated_state)
		vgic_kick_vcpus(vcpu->kvm);

	return test_bit(vcpu->vcpu_id, dist->irq_active_on_cpu);
}

bool kvm_vgic_map_is_active(struct kvm_vcpu *vcpu, unsigned int virt_irq)
{
	int i;

	for (i = 0; i < vgic->nr_lr; i++) {
		struct vgic_lr vlr = vgic_get_lr(vcpu, i);

		if (vlr.irq == virt_irq && vlr.state & LR_STATE_ACTIVE)
			return true;
	}

	return vgic_irq_is_active(vcpu, virt_irq);
}

/*
 * An interrupt may have been disabled after being made pending on the
}

static int vgic_update_irq_pending(struct kvm *kvm, int cpuid,
				   unsigned int irq_num, bool level)
{
	struct vgic_dist *dist = &kvm->arch.vgic;
	struct kvm_vcpu *vcpu;
	if (map)
		return -EINVAL;

	return vgic_update_irq_pending(kvm, cpuid, irq_num, level);
}

/**
 * kvm_vgic_inject_mapped_irq - Inject a physically mapped IRQ to the vgic
 * @kvm:     The VM structure pointer
 * @cpuid:   The CPU for PPIs
 * @virt_irq: The virtual IRQ to be injected
 * @level:   Edge-triggered:  true:  to trigger the interrupt
 *			      false: to ignore the call
 *	     Level-sensitive  true:  raise the input signal
 *			      false: lower the input signal
 * being HIGH and 0 being LOW and all devices being active-HIGH.
 */
int kvm_vgic_inject_mapped_irq(struct kvm *kvm, int cpuid,
			       unsigned int virt_irq, bool level)
{
	int ret;

	ret = vgic_lazy_init(kvm);
	if (ret)
		return ret;

	return vgic_update_irq_pending(kvm, cpuid, virt_irq, level);
}

static irqreturn_t vgic_maintenance_handler(int irq, void *data)
{
/**
 * kvm_vgic_map_phys_irq - map a virtual IRQ to a physical IRQ
 * @vcpu: The VCPU pointer
 * @virt_irq: The virtual IRQ number for the guest
 * @phys_irq: The hardware IRQ number of the host
 *
 * Establish a mapping between a guest visible irq (@virt_irq) and a
 * hardware irq (@phys_irq). On injection, @virt_irq will be associated with
 * the physical interrupt represented by @phys_irq. This mapping can be
 * established multiple times as long as the parameters are the same.
 *
 * Returns 0 on success or an error value otherwise.
 */
int kvm_vgic_map_phys_irq(struct kvm_vcpu *vcpu, int virt_irq, int phys_irq)
{
	struct vgic_dist *dist = &vcpu->kvm->arch.vgic;
	struct list_head *root = vgic_get_irq_phys_map_list(vcpu, virt_irq);
	struct irq_phys_map *map;
	struct irq_phys_map_entry *entry;
	int ret = 0;

	/* Create a new mapping */
	entry = kzalloc(sizeof(*entry), GFP_KERNEL);
	if (!entry)
		return -ENOMEM;

	spin_lock(&dist->irq_phys_map_lock);

	/* Try to match an existing mapping */
	map = vgic_irq_map_search(vcpu, virt_irq);
	if (map) {
		/* Make sure this mapping matches */
		if (map->phys_irq != phys_irq)
			ret = -EINVAL;

		/* Found an existing, valid mapping */
		goto out;
	}
	map           = &entry->map;
	map->virt_irq = virt_irq;
	map->phys_irq = phys_irq;

	list_add_tail_rcu(&entry->entry, root);

out:
	spin_unlock(&dist->irq_phys_map_lock);
	/* If we've found a hit in the existing list, free the useless
	 * entry */
	if (ret || map != &entry->map)
		kfree(entry);
	return ret;
}

static struct irq_phys_map *vgic_irq_map_search(struct kvm_vcpu *vcpu,
						int virt_irq)
/**
 * kvm_vgic_unmap_phys_irq - Remove a virtual to physical IRQ mapping
 * @vcpu: The VCPU pointer
 * @virt_irq: The virtual IRQ number to be unmapped
 *
 * Remove an existing mapping between virtual and physical interrupts.
 */
int kvm_vgic_unmap_phys_irq(struct kvm_vcpu *vcpu, unsigned int virt_irq)
{
	struct vgic_dist *dist = &vcpu->kvm->arch.vgic;
	struct irq_phys_map_entry *entry;
	struct list_head *root;

	root = vgic_get_irq_phys_map_list(vcpu, virt_irq);

	spin_lock(&dist->irq_phys_map_lock);

	list_for_each_entry(entry, root, entry) {
		if (entry->map.virt_irq == virt_irq) {
			list_del_rcu(&entry->entry);
			call_rcu(&entry->rcu, vgic_free_phys_irq_map_rcu);
			break;
		}
		return -ENOMEM;
	}

	return 0;
}

/**
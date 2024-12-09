		} else {
			*reg &= 0xffff << 16;
			*reg |= val;
		}
	}

	return false;
}

/**
 * vgic_unqueue_irqs - move pending/active IRQs from LRs to the distributor
 * @vgic_cpu: Pointer to the vgic_cpu struct holding the LRs
 *
 * Move any IRQs that have already been assigned to LRs back to the
 * emulated distributor state so that the complete emulated state can be read
 * from the main emulation structures without investigating the LRs.
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
		 *
		 * 01: pending
		 * 10: active
		 * 11: pending and active
		 */
		BUG_ON(!(lr.state & LR_STATE_MASK));

		/* Reestablish SGI source for pending and active IRQs */
		if (lr.irq < VGIC_NR_SGIS)
			add_sgi_source(vcpu, lr.irq, lr.source);

		/*
		 * If the LR holds an active (10) or a pending and active (11)
		 * interrupt then move the active state to the
		 * distributor tracking bit.
		 */
		if (lr.state & LR_STATE_ACTIVE)
			vgic_irq_set_active(vcpu, lr.irq);

		/*
		 * Reestablish the pending state on the distributor and the
		 * CPU interface and mark the LR as free for other use.
		 */
		vgic_retire_lr(i, vcpu);

		/* Finally update the VGIC state. */
		vgic_update_state(vcpu->kvm);
	}
}
{
	struct vgic_dist *dist = &vcpu->kvm->arch.vgic;
	struct vgic_io_device *iodev = container_of(this,
						    struct vgic_io_device, dev);
	const struct vgic_io_range *range;
	struct kvm_exit_mmio mmio;
	bool updated_state;
	gpa_t offset;

	offset = addr - iodev->addr;
	range = vgic_find_range(iodev->reg_ranges, len, offset);
	if (unlikely(!range || !range->handle_mmio)) {
		pr_warn("Unhandled access %d %08llx %d\n", is_write, addr, len);
		return -ENXIO;
	}
	} else {
		if (!is_write)
			memset(val, 0, len);
		updated_state = false;
	}
	spin_unlock(&dist->lock);

	if (updated_state)
		vgic_kick_vcpus(vcpu->kvm);

	return 0;
}

static int vgic_handle_mmio_read(struct kvm_vcpu *vcpu,
				 struct kvm_io_device *this,
				 gpa_t addr, int len, void *val)
{
	return vgic_handle_mmio_access(vcpu, this, addr, len, val, false);
}

static int vgic_handle_mmio_write(struct kvm_vcpu *vcpu,
				  struct kvm_io_device *this,
				  gpa_t addr, int len, const void *val)
{
	return vgic_handle_mmio_access(vcpu, this, addr, len, (void *)val,
				       true);
}

static struct kvm_io_device_ops vgic_io_ops = {
	.read	= vgic_handle_mmio_read,
	.write	= vgic_handle_mmio_write,
};

/**
 * vgic_register_kvm_io_dev - register VGIC register frame on the KVM I/O bus
 * @kvm:            The VM structure pointer
 * @base:           The (guest) base address for the register frame
 * @len:            Length of the register frame window
 * @ranges:         Describing the handler functions for each register
 * @redist_vcpu_id: The VCPU ID to pass on to the handlers on call
 * @iodev:          Points to memory to be passed on to the handler
 *
 * @iodev stores the parameters of this function to be usable by the handler
 * respectively the dispatcher function (since the KVM I/O bus framework lacks
 * an opaque parameter). Initialization is done in this function, but the
 * reference should be valid and unique for the whole VGIC lifetime.
 * If the register frame is not mapped for a specific VCPU, pass -1 to
 * @redist_vcpu_id.
 */
int vgic_register_kvm_io_dev(struct kvm *kvm, gpa_t base, int len,
			     const struct vgic_io_range *ranges,
			     int redist_vcpu_id,
			     struct vgic_io_device *iodev)
{
	struct kvm_vcpu *vcpu = NULL;
	int ret;

	if (redist_vcpu_id >= 0)
		vcpu = kvm_get_vcpu(kvm, redist_vcpu_id);

	iodev->addr		= base;
	iodev->len		= len;
	iodev->reg_ranges	= ranges;
	iodev->redist_vcpu	= vcpu;

	kvm_iodevice_init(&iodev->dev, &vgic_io_ops);

	mutex_lock(&kvm->slots_lock);

	ret = kvm_io_bus_register_dev(kvm, KVM_MMIO_BUS, base, len,
				      &iodev->dev);
	mutex_unlock(&kvm->slots_lock);

	/* Mark the iodev as invalid if registration fails. */
	if (ret)
		iodev->dev.ops = NULL;

	return ret;
}

static int vgic_nr_shared_irqs(struct vgic_dist *dist)
{
	return dist->nr_irqs - VGIC_NR_PRIVATE_IRQS;
}

static int compute_active_for_cpu(struct kvm_vcpu *vcpu)
{
	struct vgic_dist *dist = &vcpu->kvm->arch.vgic;
	unsigned long *active, *enabled, *act_percpu, *act_shared;
	unsigned long active_private, active_shared;
	int nr_shared = vgic_nr_shared_irqs(dist);
	int vcpu_id;

	vcpu_id = vcpu->vcpu_id;
	act_percpu = vcpu->arch.vgic_cpu.active_percpu;
	act_shared = vcpu->arch.vgic_cpu.active_shared;

	active = vgic_bitmap_get_cpu_map(&dist->irq_active, vcpu_id);
	enabled = vgic_bitmap_get_cpu_map(&dist->irq_enabled, vcpu_id);
	bitmap_and(act_percpu, active, enabled, VGIC_NR_PRIVATE_IRQS);

	active = vgic_bitmap_get_shared_map(&dist->irq_active);
	enabled = vgic_bitmap_get_shared_map(&dist->irq_enabled);
	bitmap_and(act_shared, active, enabled, nr_shared);
	bitmap_and(act_shared, act_shared,
		   vgic_bitmap_get_shared_map(&dist->irq_spi_target[vcpu_id]),
		   nr_shared);

	active_private = find_first_bit(act_percpu, VGIC_NR_PRIVATE_IRQS);
	active_shared = find_first_bit(act_shared, nr_shared);

	return (active_private < VGIC_NR_PRIVATE_IRQS ||
		active_shared < nr_shared);
}

static int compute_pending_for_cpu(struct kvm_vcpu *vcpu)
{
	struct vgic_dist *dist = &vcpu->kvm->arch.vgic;
	unsigned long *pending, *enabled, *pend_percpu, *pend_shared;
	unsigned long pending_private, pending_shared;
	int nr_shared = vgic_nr_shared_irqs(dist);
	int vcpu_id;

	vcpu_id = vcpu->vcpu_id;
	pend_percpu = vcpu->arch.vgic_cpu.pending_percpu;
	pend_shared = vcpu->arch.vgic_cpu.pending_shared;

	if (!dist->enabled) {
		bitmap_zero(pend_percpu, VGIC_NR_PRIVATE_IRQS);
		bitmap_zero(pend_shared, nr_shared);
		return 0;
	}
{
	struct vgic_dist *dist = &vcpu->kvm->arch.vgic;

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
	} else {
		int state = vgic_dist_irq_get_level(vcpu, irq);
		return level != state;
	}
}

static int vgic_update_irq_pending(struct kvm *kvm, int cpuid,
				   unsigned int irq_num, bool level)
{
	struct vgic_dist *dist = &kvm->arch.vgic;
	struct kvm_vcpu *vcpu;
	int edge_triggered, level_triggered;
	int enabled;
	bool ret = true, can_inject = true;

	trace_vgic_update_irq_pending(cpuid, irq_num, level);

	if (irq_num >= min(kvm->arch.vgic.nr_irqs, 1020))
		return -EINVAL;

	spin_lock(&dist->lock);

	vcpu = kvm_get_vcpu(kvm, cpuid);
	edge_triggered = vgic_irq_is_edge(vcpu, irq_num);
	level_triggered = !edge_triggered;

	if (!vgic_validate_injection(vcpu, irq_num, level)) {
		ret = false;
		goto out;
	}

	if (irq_num >= VGIC_NR_PRIVATE_IRQS) {
		cpuid = dist->irq_spi_cpu[irq_num - VGIC_NR_PRIVATE_IRQS];
		if (cpuid == VCPU_NOT_ALLOCATED) {
			/* Pretend we use CPU0, and prevent injection */
			cpuid = 0;
			can_inject = false;
		}
		vcpu = kvm_get_vcpu(kvm, cpuid);
	}

	kvm_debug("Inject IRQ%d level %d CPU%d\n", irq_num, level, cpuid);

	if (level) {
		if (level_triggered)
			vgic_dist_irq_set_level(vcpu, irq_num);
		vgic_dist_irq_set_pending(vcpu, irq_num);
	} else {
		if (level_triggered) {
			vgic_dist_irq_clear_level(vcpu, irq_num);
			if (!vgic_dist_irq_soft_pend(vcpu, irq_num)) {
				vgic_dist_irq_clear_pending(vcpu, irq_num);
				vgic_cpu_irq_clear(vcpu, irq_num);
				if (!compute_pending_for_cpu(vcpu))
					clear_bit(cpuid, dist->irq_pending_on_cpu);
			}
		}

		ret = false;
		goto out;
	}

	enabled = vgic_irq_is_enabled(vcpu, irq_num);

	if (!enabled || !can_inject) {
		ret = false;
		goto out;
	}

	if (!vgic_can_sample_irq(vcpu, irq_num)) {
		/*
		 * Level interrupt in progress, will be picked up
		 * when EOId.
		 */
		ret = false;
		goto out;
	}

	if (level) {
		vgic_cpu_irq_set(vcpu, irq_num);
		set_bit(cpuid, dist->irq_pending_on_cpu);
	}

out:
	spin_unlock(&dist->lock);

	if (ret) {
		/* kick the specified vcpu */
		kvm_vcpu_kick(kvm_get_vcpu(kvm, cpuid));
	}

	return 0;
}

static int vgic_lazy_init(struct kvm *kvm)
{
	int ret = 0;

	if (unlikely(!vgic_initialized(kvm))) {
		/*
		 * We only provide the automatic initialization of the VGIC
		 * for the legacy case of a GICv2. Any other type must
		 * be explicitly initialized once setup with the respective
		 * KVM device call.
		 */
		if (kvm->arch.vgic.vgic_model != KVM_DEV_TYPE_ARM_VGIC_V2)
			return -EBUSY;

		mutex_lock(&kvm->lock);
		ret = vgic_init(kvm);
		mutex_unlock(&kvm->lock);
	}

	return ret;
}

/**
 * kvm_vgic_inject_irq - Inject an IRQ from a device to the vgic
 * @kvm:     The VM structure pointer
 * @cpuid:   The CPU for PPIs
 * @irq_num: The IRQ number that is assigned to the device. This IRQ
 *           must not be mapped to a HW interrupt.
 * @level:   Edge-triggered:  true:  to trigger the interrupt
 *			      false: to ignore the call
 *	     Level-sensitive  true:  raise the input signal
 *			      false: lower the input signal
 *
 * The GIC is not concerned with devices being active-LOW or active-HIGH for
 * level-sensitive interrupts.  You can think of the level parameter as 1
 * being HIGH and 0 being LOW and all devices being active-HIGH.
 */
int kvm_vgic_inject_irq(struct kvm *kvm, int cpuid, unsigned int irq_num,
			bool level)
{
	struct irq_phys_map *map;
	int ret;

	ret = vgic_lazy_init(kvm);
	if (ret)
		return ret;

	map = vgic_irq_map_search(kvm_get_vcpu(kvm, cpuid), irq_num);
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
 *
 * The GIC is not concerned with devices being active-LOW or active-HIGH for
 * level-sensitive interrupts.  You can think of the level parameter as 1
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
	/*
	 * We cannot rely on the vgic maintenance interrupt to be
	 * delivered synchronously. This means we can only use it to
	 * exit the VM, and we perform the handling of EOIed
	 * interrupts on the exit path (see vgic_process_maintenance).
	 */
	return IRQ_HANDLED;
}

static struct list_head *vgic_get_irq_phys_map_list(struct kvm_vcpu *vcpu,
						    int virt_irq)
{
	if (virt_irq < VGIC_NR_PRIVATE_IRQS)
		return &vcpu->arch.vgic_cpu.irq_phys_map_list;
	else
		return &vcpu->kvm->arch.vgic.irq_phys_map_list;
}

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
{
	struct list_head *root = vgic_get_irq_phys_map_list(vcpu, virt_irq);
	struct irq_phys_map_entry *entry;
	struct irq_phys_map *map;

	rcu_read_lock();

	list_for_each_entry_rcu(entry, root, entry) {
		map = &entry->map;
		if (map->virt_irq == virt_irq) {
			rcu_read_unlock();
			return map;
		}
	}

	rcu_read_unlock();

	return NULL;
}

static void vgic_free_phys_irq_map_rcu(struct rcu_head *rcu)
{
	struct irq_phys_map_entry *entry;

	entry = container_of(rcu, struct irq_phys_map_entry, rcu);
	kfree(entry);
}

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
	}

	spin_unlock(&dist->irq_phys_map_lock);

	return 0;
}

static void vgic_destroy_irq_phys_map(struct kvm *kvm, struct list_head *root)
{
	struct vgic_dist *dist = &kvm->arch.vgic;
	struct irq_phys_map_entry *entry;

	spin_lock(&dist->irq_phys_map_lock);

	list_for_each_entry(entry, root, entry) {
		list_del_rcu(&entry->entry);
		call_rcu(&entry->rcu, vgic_free_phys_irq_map_rcu);
	}

	spin_unlock(&dist->irq_phys_map_lock);
}

void kvm_vgic_vcpu_destroy(struct kvm_vcpu *vcpu)
{
	struct vgic_cpu *vgic_cpu = &vcpu->arch.vgic_cpu;

	kfree(vgic_cpu->pending_shared);
	kfree(vgic_cpu->active_shared);
	kfree(vgic_cpu->pend_act_shared);
	vgic_destroy_irq_phys_map(vcpu->kvm, &vgic_cpu->irq_phys_map_list);
	vgic_cpu->pending_shared = NULL;
	vgic_cpu->active_shared = NULL;
	vgic_cpu->pend_act_shared = NULL;
}

static int vgic_vcpu_init_maps(struct kvm_vcpu *vcpu, int nr_irqs)
{
	struct vgic_cpu *vgic_cpu = &vcpu->arch.vgic_cpu;
	int nr_longs = BITS_TO_LONGS(nr_irqs - VGIC_NR_PRIVATE_IRQS);
	int sz = nr_longs * sizeof(unsigned long);
	vgic_cpu->pending_shared = kzalloc(sz, GFP_KERNEL);
	vgic_cpu->active_shared = kzalloc(sz, GFP_KERNEL);
	vgic_cpu->pend_act_shared = kzalloc(sz, GFP_KERNEL);

	if (!vgic_cpu->pending_shared
		|| !vgic_cpu->active_shared
		|| !vgic_cpu->pend_act_shared) {
		kvm_vgic_vcpu_destroy(vcpu);
		return -ENOMEM;
	}

	return 0;
}

/**
 * kvm_vgic_vcpu_early_init - Earliest possible per-vcpu vgic init stage
 *
 * No memory allocation should be performed here, only static init.
 */
void kvm_vgic_vcpu_early_init(struct kvm_vcpu *vcpu)
{
	struct vgic_cpu *vgic_cpu = &vcpu->arch.vgic_cpu;
	INIT_LIST_HEAD(&vgic_cpu->irq_phys_map_list);
}

/**
 * kvm_vgic_get_max_vcpus - Get the maximum number of VCPUs allowed by HW
 *
 * The host's GIC naturally limits the maximum amount of VCPUs a guest
 * can use.
 */
int kvm_vgic_get_max_vcpus(void)
{
	return vgic->max_gic_vcpus;
}

void kvm_vgic_destroy(struct kvm *kvm)
{
	struct vgic_dist *dist = &kvm->arch.vgic;
	struct kvm_vcpu *vcpu;
	int i;

	kvm_for_each_vcpu(i, vcpu, kvm)
		kvm_vgic_vcpu_destroy(vcpu);

	vgic_free_bitmap(&dist->irq_enabled);
	vgic_free_bitmap(&dist->irq_level);
	vgic_free_bitmap(&dist->irq_pending);
	vgic_free_bitmap(&dist->irq_soft_pend);
	vgic_free_bitmap(&dist->irq_queued);
	vgic_free_bitmap(&dist->irq_cfg);
	vgic_free_bytemap(&dist->irq_priority);
	if (dist->irq_spi_target) {
		for (i = 0; i < dist->nr_cpus; i++)
			vgic_free_bitmap(&dist->irq_spi_target[i]);
	}
	kfree(dist->irq_sgi_sources);
	kfree(dist->irq_spi_cpu);
	kfree(dist->irq_spi_mpidr);
	kfree(dist->irq_spi_target);
	kfree(dist->irq_pending_on_cpu);
	kfree(dist->irq_active_on_cpu);
	vgic_destroy_irq_phys_map(kvm, &dist->irq_phys_map_list);
	dist->irq_sgi_sources = NULL;
	dist->irq_spi_cpu = NULL;
	dist->irq_spi_target = NULL;
	dist->irq_pending_on_cpu = NULL;
	dist->irq_active_on_cpu = NULL;
	dist->nr_cpus = 0;
}

/*
 * Allocate and initialize the various data structures. Must be called
 * with kvm->lock held!
 */
int vgic_init(struct kvm *kvm)
{
	struct vgic_dist *dist = &kvm->arch.vgic;
	struct kvm_vcpu *vcpu;
	int nr_cpus, nr_irqs;
	int ret, i, vcpu_id;

	if (vgic_initialized(kvm))
		return 0;

	nr_cpus = dist->nr_cpus = atomic_read(&kvm->online_vcpus);
	if (!nr_cpus)		/* No vcpus? Can't be good... */
		return -ENODEV;

	/*
	 * If nobody configured the number of interrupts, use the
	 * legacy one.
	 */
	if (!dist->nr_irqs)
		dist->nr_irqs = VGIC_NR_IRQS_LEGACY;

	nr_irqs = dist->nr_irqs;

	ret  = vgic_init_bitmap(&dist->irq_enabled, nr_cpus, nr_irqs);
	ret |= vgic_init_bitmap(&dist->irq_level, nr_cpus, nr_irqs);
	ret |= vgic_init_bitmap(&dist->irq_pending, nr_cpus, nr_irqs);
	ret |= vgic_init_bitmap(&dist->irq_soft_pend, nr_cpus, nr_irqs);
	ret |= vgic_init_bitmap(&dist->irq_queued, nr_cpus, nr_irqs);
	ret |= vgic_init_bitmap(&dist->irq_active, nr_cpus, nr_irqs);
	ret |= vgic_init_bitmap(&dist->irq_cfg, nr_cpus, nr_irqs);
	ret |= vgic_init_bytemap(&dist->irq_priority, nr_cpus, nr_irqs);

	if (ret)
		goto out;

	dist->irq_sgi_sources = kzalloc(nr_cpus * VGIC_NR_SGIS, GFP_KERNEL);
	dist->irq_spi_cpu = kzalloc(nr_irqs - VGIC_NR_PRIVATE_IRQS, GFP_KERNEL);
	dist->irq_spi_target = kzalloc(sizeof(*dist->irq_spi_target) * nr_cpus,
				       GFP_KERNEL);
	dist->irq_pending_on_cpu = kzalloc(BITS_TO_LONGS(nr_cpus) * sizeof(long),
					   GFP_KERNEL);
	dist->irq_active_on_cpu = kzalloc(BITS_TO_LONGS(nr_cpus) * sizeof(long),
					   GFP_KERNEL);
	if (!dist->irq_sgi_sources ||
	    !dist->irq_spi_cpu ||
	    !dist->irq_spi_target ||
	    !dist->irq_pending_on_cpu ||
	    !dist->irq_active_on_cpu) {
		ret = -ENOMEM;
		goto out;
	}

	for (i = 0; i < nr_cpus; i++)
		ret |= vgic_init_bitmap(&dist->irq_spi_target[i],
					nr_cpus, nr_irqs);

	if (ret)
		goto out;

	ret = kvm->arch.vgic.vm_ops.init_model(kvm);
	if (ret)
		goto out;

	kvm_for_each_vcpu(vcpu_id, vcpu, kvm) {
		ret = vgic_vcpu_init_maps(vcpu, nr_irqs);
		if (ret) {
			kvm_err("VGIC: Failed to allocate vcpu memory\n");
			break;
		}

		/*
		 * Enable and configure all SGIs to be edge-triggere and
		 * configure all PPIs as level-triggered.
		 */
		for (i = 0; i < VGIC_NR_PRIVATE_IRQS; i++) {
			if (i < VGIC_NR_SGIS) {
				/* SGIs */
				vgic_bitmap_set_irq_val(&dist->irq_enabled,
							vcpu->vcpu_id, i, 1);
				vgic_bitmap_set_irq_val(&dist->irq_cfg,
							vcpu->vcpu_id, i,
							VGIC_CFG_EDGE);
			} else if (i < VGIC_NR_PRIVATE_IRQS) {
				/* PPIs */
				vgic_bitmap_set_irq_val(&dist->irq_cfg,
							vcpu->vcpu_id, i,
							VGIC_CFG_LEVEL);
			}
		}

		vgic_enable(vcpu);
	}

out:
	if (ret)
		kvm_vgic_destroy(kvm);

	return ret;
}

static int init_vgic_model(struct kvm *kvm, int type)
{
	switch (type) {
	case KVM_DEV_TYPE_ARM_VGIC_V2:
		vgic_v2_init_emulation(kvm);
		break;
#ifdef CONFIG_KVM_ARM_VGIC_V3
	case KVM_DEV_TYPE_ARM_VGIC_V3:
		vgic_v3_init_emulation(kvm);
		break;
#endif
	default:
		return -ENODEV;
	}

	if (atomic_read(&kvm->online_vcpus) > kvm->arch.max_vcpus)
		return -E2BIG;

	return 0;
}

/**
 * kvm_vgic_early_init - Earliest possible vgic initialization stage
 *
 * No memory allocation should be performed here, only static init.
 */
void kvm_vgic_early_init(struct kvm *kvm)
{
	spin_lock_init(&kvm->arch.vgic.lock);
	spin_lock_init(&kvm->arch.vgic.irq_phys_map_lock);
	INIT_LIST_HEAD(&kvm->arch.vgic.irq_phys_map_list);
}

int kvm_vgic_create(struct kvm *kvm, u32 type)
{
	int i, vcpu_lock_idx = -1, ret;
	struct kvm_vcpu *vcpu;

	mutex_lock(&kvm->lock);

	if (irqchip_in_kernel(kvm)) {
		ret = -EEXIST;
		goto out;
	}

	/*
	 * This function is also called by the KVM_CREATE_IRQCHIP handler,
	 * which had no chance yet to check the availability of the GICv2
	 * emulation. So check this here again. KVM_CREATE_DEVICE does
	 * the proper checks already.
	 */
	if (type == KVM_DEV_TYPE_ARM_VGIC_V2 && !vgic->can_emulate_gicv2) {
		ret = -ENODEV;
		goto out;
	}

	/*
	 * Any time a vcpu is run, vcpu_load is called which tries to grab the
	 * vcpu->mutex.  By grabbing the vcpu->mutex of all VCPUs we ensure
	 * that no other VCPUs are run while we create the vgic.
	 */
	ret = -EBUSY;
	kvm_for_each_vcpu(i, vcpu, kvm) {
		if (!mutex_trylock(&vcpu->mutex))
			goto out_unlock;
		vcpu_lock_idx = i;
	}

	kvm_for_each_vcpu(i, vcpu, kvm) {
		if (vcpu->arch.has_run_once)
			goto out_unlock;
	}
	ret = 0;

	ret = init_vgic_model(kvm, type);
	if (ret)
		goto out_unlock;

	kvm->arch.vgic.in_kernel = true;
	kvm->arch.vgic.vgic_model = type;
	kvm->arch.vgic.vctrl_base = vgic->vctrl_base;
	kvm->arch.vgic.vgic_dist_base = VGIC_ADDR_UNDEF;
	kvm->arch.vgic.vgic_cpu_base = VGIC_ADDR_UNDEF;
	kvm->arch.vgic.vgic_redist_base = VGIC_ADDR_UNDEF;

out_unlock:
	for (; vcpu_lock_idx >= 0; vcpu_lock_idx--) {
		vcpu = kvm_get_vcpu(kvm, vcpu_lock_idx);
		mutex_unlock(&vcpu->mutex);
	}

out:
	mutex_unlock(&kvm->lock);
	return ret;
}

static int vgic_ioaddr_overlap(struct kvm *kvm)
{
	phys_addr_t dist = kvm->arch.vgic.vgic_dist_base;
	phys_addr_t cpu = kvm->arch.vgic.vgic_cpu_base;

	if (IS_VGIC_ADDR_UNDEF(dist) || IS_VGIC_ADDR_UNDEF(cpu))
		return 0;
	if ((dist <= cpu && dist + KVM_VGIC_V2_DIST_SIZE > cpu) ||
	    (cpu <= dist && cpu + KVM_VGIC_V2_CPU_SIZE > dist))
		return -EBUSY;
	return 0;
}

static int vgic_ioaddr_assign(struct kvm *kvm, phys_addr_t *ioaddr,
			      phys_addr_t addr, phys_addr_t size)
{
	int ret;

	if (addr & ~KVM_PHYS_MASK)
		return -E2BIG;

	if (addr & (SZ_4K - 1))
		return -EINVAL;

	if (!IS_VGIC_ADDR_UNDEF(*ioaddr))
		return -EEXIST;
	if (addr + size < addr)
		return -EINVAL;

	*ioaddr = addr;
	ret = vgic_ioaddr_overlap(kvm);
	if (ret)
		*ioaddr = VGIC_ADDR_UNDEF;

	return ret;
}

/**
 * kvm_vgic_addr - set or get vgic VM base addresses
 * @kvm:   pointer to the vm struct
 * @type:  the VGIC addr type, one of KVM_VGIC_V[23]_ADDR_TYPE_XXX
 * @addr:  pointer to address value
 * @write: if true set the address in the VM address space, if false read the
 *          address
 *
 * Set or get the vgic base addresses for the distributor and the virtual CPU
 * interface in the VM physical address space.  These addresses are properties
 * of the emulated core/SoC and therefore user space initially knows this
 * information.
 */
int kvm_vgic_addr(struct kvm *kvm, unsigned long type, u64 *addr, bool write)
{
	int r = 0;
	struct vgic_dist *vgic = &kvm->arch.vgic;
	int type_needed;
	phys_addr_t *addr_ptr, block_size;
	phys_addr_t alignment;

	mutex_lock(&kvm->lock);
	switch (type) {
	case KVM_VGIC_V2_ADDR_TYPE_DIST:
		type_needed = KVM_DEV_TYPE_ARM_VGIC_V2;
		addr_ptr = &vgic->vgic_dist_base;
		block_size = KVM_VGIC_V2_DIST_SIZE;
		alignment = SZ_4K;
		break;
	case KVM_VGIC_V2_ADDR_TYPE_CPU:
		type_needed = KVM_DEV_TYPE_ARM_VGIC_V2;
		addr_ptr = &vgic->vgic_cpu_base;
		block_size = KVM_VGIC_V2_CPU_SIZE;
		alignment = SZ_4K;
		break;
#ifdef CONFIG_KVM_ARM_VGIC_V3
	case KVM_VGIC_V3_ADDR_TYPE_DIST:
		type_needed = KVM_DEV_TYPE_ARM_VGIC_V3;
		addr_ptr = &vgic->vgic_dist_base;
		block_size = KVM_VGIC_V3_DIST_SIZE;
		alignment = SZ_64K;
		break;
	case KVM_VGIC_V3_ADDR_TYPE_REDIST:
		type_needed = KVM_DEV_TYPE_ARM_VGIC_V3;
		addr_ptr = &vgic->vgic_redist_base;
		block_size = KVM_VGIC_V3_REDIST_SIZE;
		alignment = SZ_64K;
		break;
#endif
	default:
		r = -ENODEV;
		goto out;
	}

	if (vgic->vgic_model != type_needed) {
		r = -ENODEV;
		goto out;
	}

	if (write) {
		if (!IS_ALIGNED(*addr, alignment))
			r = -EINVAL;
		else
			r = vgic_ioaddr_assign(kvm, addr_ptr, *addr,
					       block_size);
	} else {
		*addr = *addr_ptr;
	}

out:
	mutex_unlock(&kvm->lock);
	return r;
}

int vgic_set_common_attr(struct kvm_device *dev, struct kvm_device_attr *attr)
{
	int r;

	switch (attr->group) {
	case KVM_DEV_ARM_VGIC_GRP_ADDR: {
		u64 __user *uaddr = (u64 __user *)(long)attr->addr;
		u64 addr;
		unsigned long type = (unsigned long)attr->attr;

		if (copy_from_user(&addr, uaddr, sizeof(addr)))
			return -EFAULT;

		r = kvm_vgic_addr(dev->kvm, type, &addr, true);
		return (r == -ENODEV) ? -ENXIO : r;
	}
	case KVM_DEV_ARM_VGIC_GRP_NR_IRQS: {
		u32 __user *uaddr = (u32 __user *)(long)attr->addr;
		u32 val;
		int ret = 0;

		if (get_user(val, uaddr))
			return -EFAULT;

		/*
		 * We require:
		 * - at least 32 SPIs on top of the 16 SGIs and 16 PPIs
		 * - at most 1024 interrupts
		 * - a multiple of 32 interrupts
		 */
		if (val < (VGIC_NR_PRIVATE_IRQS + 32) ||
		    val > VGIC_MAX_IRQS ||
		    (val & 31))
			return -EINVAL;

		mutex_lock(&dev->kvm->lock);

		if (vgic_ready(dev->kvm) || dev->kvm->arch.vgic.nr_irqs)
			ret = -EBUSY;
		else
			dev->kvm->arch.vgic.nr_irqs = val;

		mutex_unlock(&dev->kvm->lock);

		return ret;
	}
	case KVM_DEV_ARM_VGIC_GRP_CTRL: {
		switch (attr->attr) {
		case KVM_DEV_ARM_VGIC_CTRL_INIT:
			r = vgic_init(dev->kvm);
			return r;
		}
		break;
	}
	}

	return -ENXIO;
}

int vgic_get_common_attr(struct kvm_device *dev, struct kvm_device_attr *attr)
{
	int r = -ENXIO;

	switch (attr->group) {
	case KVM_DEV_ARM_VGIC_GRP_ADDR: {
		u64 __user *uaddr = (u64 __user *)(long)attr->addr;
		u64 addr;
		unsigned long type = (unsigned long)attr->attr;

		r = kvm_vgic_addr(dev->kvm, type, &addr, false);
		if (r)
			return (r == -ENODEV) ? -ENXIO : r;

		if (copy_to_user(uaddr, &addr, sizeof(addr)))
			return -EFAULT;
		break;
	}
	case KVM_DEV_ARM_VGIC_GRP_NR_IRQS: {
		u32 __user *uaddr = (u32 __user *)(long)attr->addr;

		r = put_user(dev->kvm->arch.vgic.nr_irqs, uaddr);
		break;
	}

	}

	return r;
}

int vgic_has_attr_regs(const struct vgic_io_range *ranges, phys_addr_t offset)
{
	if (vgic_find_range(ranges, 4, offset))
		return 0;
	else
		return -ENXIO;
}

static void vgic_init_maintenance_interrupt(void *info)
{
	enable_percpu_irq(vgic->maint_irq, 0);
}

static int vgic_cpu_notify(struct notifier_block *self,
			   unsigned long action, void *cpu)
{
	switch (action) {
	case CPU_STARTING:
	case CPU_STARTING_FROZEN:
		vgic_init_maintenance_interrupt(NULL);
		break;
	case CPU_DYING:
	case CPU_DYING_FROZEN:
		disable_percpu_irq(vgic->maint_irq);
		break;
	}

	return NOTIFY_OK;
}

static struct notifier_block vgic_cpu_nb = {
	.notifier_call = vgic_cpu_notify,
};

static int kvm_vgic_probe(void)
{
	const struct gic_kvm_info *gic_kvm_info;
	int ret;

	gic_kvm_info = gic_get_kvm_info();
	if (!gic_kvm_info)
		return -ENODEV;

	switch (gic_kvm_info->type) {
	case GIC_V2:
		ret = vgic_v2_probe(gic_kvm_info, &vgic_ops, &vgic);
		break;
	case GIC_V3:
		ret = vgic_v3_probe(gic_kvm_info, &vgic_ops, &vgic);
		break;
	default:
		ret = -ENODEV;
	}

	return ret;
}

int kvm_vgic_hyp_init(void)
{
	int ret;

	ret = kvm_vgic_probe();
	if (ret) {
		kvm_err("error: KVM vGIC probing failed\n");
		return ret;
	}

	ret = request_percpu_irq(vgic->maint_irq, vgic_maintenance_handler,
				 "vgic", kvm_get_running_vcpus());
	if (ret) {
		kvm_err("Cannot register interrupt %d\n", vgic->maint_irq);
		return ret;
	}

	ret = __register_cpu_notifier(&vgic_cpu_nb);
	if (ret) {
		kvm_err("Cannot register vgic CPU notifier\n");
		goto out_free_irq;
	}

	on_each_cpu(vgic_init_maintenance_interrupt, NULL, 1);

	return 0;

out_free_irq:
	free_percpu_irq(vgic->maint_irq, kvm_get_running_vcpus());
	return ret;
}

int kvm_irq_map_gsi(struct kvm *kvm,
		    struct kvm_kernel_irq_routing_entry *entries,
		    int gsi)
{
	return 0;
}

int kvm_irq_map_chip_pin(struct kvm *kvm, unsigned irqchip, unsigned pin)
{
	return pin;
}

int kvm_set_irq(struct kvm *kvm, int irq_source_id,
		u32 irq, int level, bool line_status)
{
	unsigned int spi = irq + VGIC_NR_PRIVATE_IRQS;

	trace_kvm_set_irq(irq, level, irq_source_id);

	BUG_ON(!vgic_initialized(kvm));

	return kvm_vgic_inject_irq(kvm, 0, spi, level);
}

/* MSI not implemented yet */
int kvm_set_msi(struct kvm_kernel_irq_routing_entry *e,
		struct kvm *kvm, int irq_source_id,
		int level, bool line_status)
{
	return 0;
}
{
	struct irq_phys_map *map;
	int ret;

	ret = vgic_lazy_init(kvm);
	if (ret)
		return ret;

	map = vgic_irq_map_search(kvm_get_vcpu(kvm, cpuid), irq_num);
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
 *
 * The GIC is not concerned with devices being active-LOW or active-HIGH for
 * level-sensitive interrupts.  You can think of the level parameter as 1
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
	/*
	 * We cannot rely on the vgic maintenance interrupt to be
	 * delivered synchronously. This means we can only use it to
	 * exit the VM, and we perform the handling of EOIed
	 * interrupts on the exit path (see vgic_process_maintenance).
	 */
	return IRQ_HANDLED;
}

static struct list_head *vgic_get_irq_phys_map_list(struct kvm_vcpu *vcpu,
						    int virt_irq)
{
	if (virt_irq < VGIC_NR_PRIVATE_IRQS)
		return &vcpu->arch.vgic_cpu.irq_phys_map_list;
	else
		return &vcpu->kvm->arch.vgic.irq_phys_map_list;
}

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
{
	struct irq_phys_map *map;
	int ret;

	ret = vgic_lazy_init(kvm);
	if (ret)
		return ret;

	map = vgic_irq_map_search(kvm_get_vcpu(kvm, cpuid), irq_num);
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
 *
 * The GIC is not concerned with devices being active-LOW or active-HIGH for
 * level-sensitive interrupts.  You can think of the level parameter as 1
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
{
	if (virt_irq < VGIC_NR_PRIVATE_IRQS)
		return &vcpu->arch.vgic_cpu.irq_phys_map_list;
	else
		return &vcpu->kvm->arch.vgic.irq_phys_map_list;
}

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
{
	struct list_head *root = vgic_get_irq_phys_map_list(vcpu, virt_irq);
	struct irq_phys_map_entry *entry;
	struct irq_phys_map *map;

	rcu_read_lock();

	list_for_each_entry_rcu(entry, root, entry) {
		map = &entry->map;
		if (map->virt_irq == virt_irq) {
			rcu_read_unlock();
			return map;
		}
	}
{
	struct irq_phys_map_entry *entry;

	entry = container_of(rcu, struct irq_phys_map_entry, rcu);
	kfree(entry);
}

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
	}

	spin_unlock(&dist->irq_phys_map_lock);

	return 0;
}
		|| !vgic_cpu->pend_act_shared) {
		kvm_vgic_vcpu_destroy(vcpu);
		return -ENOMEM;
	}

	return 0;
}

/**
 * kvm_vgic_vcpu_early_init - Earliest possible per-vcpu vgic init stage
 *
 * No memory allocation should be performed here, only static init.
 */
void kvm_vgic_vcpu_early_init(struct kvm_vcpu *vcpu)
{
	struct vgic_cpu *vgic_cpu = &vcpu->arch.vgic_cpu;
	INIT_LIST_HEAD(&vgic_cpu->irq_phys_map_list);
}

/**
 * kvm_vgic_get_max_vcpus - Get the maximum number of VCPUs allowed by HW
 *
 * The host's GIC naturally limits the maximum amount of VCPUs a guest
 * can use.
 */
int kvm_vgic_get_max_vcpus(void)
{
	return vgic->max_gic_vcpus;
}

void kvm_vgic_destroy(struct kvm *kvm)
{
	struct vgic_dist *dist = &kvm->arch.vgic;
	struct kvm_vcpu *vcpu;
	int i;

	kvm_for_each_vcpu(i, vcpu, kvm)
		kvm_vgic_vcpu_destroy(vcpu);

	vgic_free_bitmap(&dist->irq_enabled);
	vgic_free_bitmap(&dist->irq_level);
	vgic_free_bitmap(&dist->irq_pending);
	vgic_free_bitmap(&dist->irq_soft_pend);
	vgic_free_bitmap(&dist->irq_queued);
	vgic_free_bitmap(&dist->irq_cfg);
	vgic_free_bytemap(&dist->irq_priority);
	if (dist->irq_spi_target) {
		for (i = 0; i < dist->nr_cpus; i++)
			vgic_free_bitmap(&dist->irq_spi_target[i]);
	}
	kfree(dist->irq_sgi_sources);
	kfree(dist->irq_spi_cpu);
	kfree(dist->irq_spi_mpidr);
	kfree(dist->irq_spi_target);
	kfree(dist->irq_pending_on_cpu);
	kfree(dist->irq_active_on_cpu);
	vgic_destroy_irq_phys_map(kvm, &dist->irq_phys_map_list);
	dist->irq_sgi_sources = NULL;
	dist->irq_spi_cpu = NULL;
	dist->irq_spi_target = NULL;
	dist->irq_pending_on_cpu = NULL;
	dist->irq_active_on_cpu = NULL;
	dist->nr_cpus = 0;
}

/*
 * Allocate and initialize the various data structures. Must be called
 * with kvm->lock held!
 */
int vgic_init(struct kvm *kvm)
{
	struct vgic_dist *dist = &kvm->arch.vgic;
	struct kvm_vcpu *vcpu;
	int nr_cpus, nr_irqs;
	int ret, i, vcpu_id;

	if (vgic_initialized(kvm))
		return 0;

	nr_cpus = dist->nr_cpus = atomic_read(&kvm->online_vcpus);
	if (!nr_cpus)		/* No vcpus? Can't be good... */
		return -ENODEV;

	/*
	 * If nobody configured the number of interrupts, use the
	 * legacy one.
	 */
	if (!dist->nr_irqs)
		dist->nr_irqs = VGIC_NR_IRQS_LEGACY;

	nr_irqs = dist->nr_irqs;

	ret  = vgic_init_bitmap(&dist->irq_enabled, nr_cpus, nr_irqs);
	ret |= vgic_init_bitmap(&dist->irq_level, nr_cpus, nr_irqs);
	ret |= vgic_init_bitmap(&dist->irq_pending, nr_cpus, nr_irqs);
	ret |= vgic_init_bitmap(&dist->irq_soft_pend, nr_cpus, nr_irqs);
	ret |= vgic_init_bitmap(&dist->irq_queued, nr_cpus, nr_irqs);
	ret |= vgic_init_bitmap(&dist->irq_active, nr_cpus, nr_irqs);
	ret |= vgic_init_bitmap(&dist->irq_cfg, nr_cpus, nr_irqs);
	ret |= vgic_init_bytemap(&dist->irq_priority, nr_cpus, nr_irqs);

	if (ret)
		goto out;

	dist->irq_sgi_sources = kzalloc(nr_cpus * VGIC_NR_SGIS, GFP_KERNEL);
	dist->irq_spi_cpu = kzalloc(nr_irqs - VGIC_NR_PRIVATE_IRQS, GFP_KERNEL);
	dist->irq_spi_target = kzalloc(sizeof(*dist->irq_spi_target) * nr_cpus,
				       GFP_KERNEL);
	dist->irq_pending_on_cpu = kzalloc(BITS_TO_LONGS(nr_cpus) * sizeof(long),
					   GFP_KERNEL);
	dist->irq_active_on_cpu = kzalloc(BITS_TO_LONGS(nr_cpus) * sizeof(long),
					   GFP_KERNEL);
	if (!dist->irq_sgi_sources ||
	    !dist->irq_spi_cpu ||
	    !dist->irq_spi_target ||
	    !dist->irq_pending_on_cpu ||
	    !dist->irq_active_on_cpu) {
		ret = -ENOMEM;
		goto out;
	}

	for (i = 0; i < nr_cpus; i++)
		ret |= vgic_init_bitmap(&dist->irq_spi_target[i],
					nr_cpus, nr_irqs);

	if (ret)
		goto out;

	ret = kvm->arch.vgic.vm_ops.init_model(kvm);
	if (ret)
		goto out;

	kvm_for_each_vcpu(vcpu_id, vcpu, kvm) {
		ret = vgic_vcpu_init_maps(vcpu, nr_irqs);
		if (ret) {
			kvm_err("VGIC: Failed to allocate vcpu memory\n");
			break;
		}
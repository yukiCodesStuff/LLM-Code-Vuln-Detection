 * The ITS structure - contains most of the infrastructure, with the
 * top-level MSI domain, the command queue, the collections, and the
 * list of devices writing to it.
 *
 * dev_alloc_lock has to be taken for device allocations, while the
 * spinlock must be taken to parse data structures such as the device
 * list.
 */
struct its_node {
	raw_spinlock_t		lock;
	struct mutex		dev_alloc_lock;
	struct list_head	entry;
	void __iomem		*base;
	phys_addr_t		phys_base;
	struct its_cmd_block	*cmd_base;
	void			*itt;
	u32			nr_ites;
	u32			device_id;
	bool			shared;
};

static struct {
	raw_spinlock_t		lock;
		nr_irqs /= 2;
	} while (nr_irqs > 0);

	if (!nr_irqs)
		err = -ENOSPC;

	if (err)
		goto out;

	bitmap = kcalloc(BITS_TO_LONGS(nr_irqs), sizeof (long), GFP_ATOMIC);
	return 0;
}

static u64 its_clear_vpend_valid(void __iomem *vlpi_base)
{
	u32 count = 1000000;	/* 1s! */
	bool clean;
	u64 val;

	val = gits_read_vpendbaser(vlpi_base + GICR_VPENDBASER);
	val &= ~GICR_VPENDBASER_Valid;
	gits_write_vpendbaser(val, vlpi_base + GICR_VPENDBASER);

	do {
		val = gits_read_vpendbaser(vlpi_base + GICR_VPENDBASER);
		clean = !(val & GICR_VPENDBASER_Dirty);
		if (!clean) {
			count--;
			cpu_relax();
			udelay(1);
		}
	} while (!clean && count);

	return val;
}

static void its_cpu_init_lpis(void)
{
	void __iomem *rbase = gic_data_rdist_rd_base();
	struct page *pend_page;
	val |= GICR_CTLR_ENABLE_LPIS;
	writel_relaxed(val, rbase + GICR_CTLR);

	if (gic_rdists->has_vlpis) {
		void __iomem *vlpi_base = gic_data_rdist_vlpi_base();

		/*
		 * It's possible for CPU to receive VLPIs before it is
		 * sheduled as a vPE, especially for the first CPU, and the
		 * VLPI with INTID larger than 2^(IDbits+1) will be considered
		 * as out of range and dropped by GIC.
		 * So we initialize IDbits to known value to avoid VLPI drop.
		 */
		val = (LPI_NRBITS - 1) & GICR_VPROPBASER_IDBITS_MASK;
		pr_debug("GICv4: CPU%d: Init IDbits to 0x%llx for GICR_VPROPBASER\n",
			smp_processor_id(), val);
		gits_write_vpropbaser(val, vlpi_base + GICR_VPROPBASER);

		/*
		 * Also clear Valid bit of GICR_VPENDBASER, in case some
		 * ancient programming gets left in and has possibility of
		 * corrupting memory.
		 */
		val = its_clear_vpend_valid(vlpi_base);
		WARN_ON(val & GICR_VPENDBASER_Dirty);
	}

	/* Make sure the GIC has seen the above */
	dsb(sy);
out:
	gic_data_rdist()->lpi_enabled = true;
	struct its_device *its_dev;
	struct msi_domain_info *msi_info;
	u32 dev_id;
	int err = 0;

	/*
	 * We ignore "dev" entierely, and rely on the dev_id that has
	 * been passed via the scratchpad. This limits this domain's
		return -EINVAL;
	}

	mutex_lock(&its->dev_alloc_lock);
	its_dev = its_find_device(its, dev_id);
	if (its_dev) {
		/*
		 * We already have seen this ID, probably through
		 * another alias (PCI bridge of some sort). No need to
		 * create the device.
		 */
		its_dev->shared = true;
		pr_debug("Reusing ITT for devID %x\n", dev_id);
		goto out;
	}

	its_dev = its_create_device(its, dev_id, nvec, true);
	if (!its_dev) {
		err = -ENOMEM;
		goto out;
	}

	pr_debug("ITT %d entries, %d bits\n", nvec, ilog2(nvec));
out:
	mutex_unlock(&its->dev_alloc_lock);
	info->scratchpad[0].ptr = its_dev;
	return err;
}

static struct msi_domain_ops its_msi_domain_ops = {
	.msi_prepare	= its_msi_prepare,
{
	struct irq_data *d = irq_domain_get_irq_data(domain, virq);
	struct its_device *its_dev = irq_data_get_irq_chip_data(d);
	struct its_node *its = its_dev->its;
	int i;

	for (i = 0; i < nr_irqs; i++) {
		struct irq_data *data = irq_domain_get_irq_data(domain,
		irq_domain_reset_irq_data(data);
	}

	mutex_lock(&its->dev_alloc_lock);

	/*
	 * If all interrupts have been freed, start mopping the
	 * floor. This is conditionned on the device not being shared.
	 */
	if (!its_dev->shared &&
	    bitmap_empty(its_dev->event_map.lpi_map,
			 its_dev->event_map.nr_lpis)) {
		its_lpi_free(its_dev->event_map.lpi_map,
			     its_dev->event_map.lpi_base,
			     its_dev->event_map.nr_lpis);
		its_free_device(its_dev);
	}

	mutex_unlock(&its->dev_alloc_lock);

	irq_domain_free_irqs_parent(domain, virq, nr_irqs);
}

static const struct irq_domain_ops its_domain_ops = {
static void its_vpe_deschedule(struct its_vpe *vpe)
{
	void __iomem *vlpi_base = gic_data_rdist_vlpi_base();
	u64 val;

	val = its_clear_vpend_valid(vlpi_base);

	if (unlikely(val & GICR_VPENDBASER_Dirty)) {
		pr_err_ratelimited("ITS virtual pending table not cleaning\n");
		vpe->idai = false;
		vpe->pending_last = true;
	} else {
	}

	raw_spin_lock_init(&its->lock);
	mutex_init(&its->dev_alloc_lock);
	INIT_LIST_HEAD(&its->entry);
	INIT_LIST_HEAD(&its->its_device_list);
	typer = gic_read_typer(its_base + GITS_TYPER);
	its->base = its_base;
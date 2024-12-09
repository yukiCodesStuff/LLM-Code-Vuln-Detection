 * The ITS structure - contains most of the infrastructure, with the
 * top-level MSI domain, the command queue, the collections, and the
 * list of devices writing to it.
 */
struct its_node {
	raw_spinlock_t		lock;
	struct list_head	entry;
	void __iomem		*base;
	phys_addr_t		phys_base;
	struct its_cmd_block	*cmd_base;
	void			*itt;
	u32			nr_ites;
	u32			device_id;
};

static struct {
	raw_spinlock_t		lock;
		nr_irqs /= 2;
	} while (nr_irqs > 0);

	if (err)
		goto out;

	bitmap = kcalloc(BITS_TO_LONGS(nr_irqs), sizeof (long), GFP_ATOMIC);
	return 0;
}

static void its_cpu_init_lpis(void)
{
	void __iomem *rbase = gic_data_rdist_rd_base();
	struct page *pend_page;
	val |= GICR_CTLR_ENABLE_LPIS;
	writel_relaxed(val, rbase + GICR_CTLR);

	/* Make sure the GIC has seen the above */
	dsb(sy);
out:
	gic_data_rdist()->lpi_enabled = true;
	struct its_device *its_dev;
	struct msi_domain_info *msi_info;
	u32 dev_id;

	/*
	 * We ignore "dev" entierely, and rely on the dev_id that has
	 * been passed via the scratchpad. This limits this domain's
		return -EINVAL;
	}

	its_dev = its_find_device(its, dev_id);
	if (its_dev) {
		/*
		 * We already have seen this ID, probably through
		 * another alias (PCI bridge of some sort). No need to
		 * create the device.
		 */
		pr_debug("Reusing ITT for devID %x\n", dev_id);
		goto out;
	}

	its_dev = its_create_device(its, dev_id, nvec, true);
	if (!its_dev)
		return -ENOMEM;

	pr_debug("ITT %d entries, %d bits\n", nvec, ilog2(nvec));
out:
	info->scratchpad[0].ptr = its_dev;
	return 0;
}

static struct msi_domain_ops its_msi_domain_ops = {
	.msi_prepare	= its_msi_prepare,
{
	struct irq_data *d = irq_domain_get_irq_data(domain, virq);
	struct its_device *its_dev = irq_data_get_irq_chip_data(d);
	int i;

	for (i = 0; i < nr_irqs; i++) {
		struct irq_data *data = irq_domain_get_irq_data(domain,
		irq_domain_reset_irq_data(data);
	}

	/* If all interrupts have been freed, start mopping the floor */
	if (bitmap_empty(its_dev->event_map.lpi_map,
			 its_dev->event_map.nr_lpis)) {
		its_lpi_free(its_dev->event_map.lpi_map,
			     its_dev->event_map.lpi_base,
			     its_dev->event_map.nr_lpis);
		its_free_device(its_dev);
	}

	irq_domain_free_irqs_parent(domain, virq, nr_irqs);
}

static const struct irq_domain_ops its_domain_ops = {
static void its_vpe_deschedule(struct its_vpe *vpe)
{
	void __iomem *vlpi_base = gic_data_rdist_vlpi_base();
	u32 count = 1000000;	/* 1s! */
	bool clean;
	u64 val;

	/* We're being scheduled out */
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

	if (unlikely(!clean && !count)) {
		pr_err_ratelimited("ITS virtual pending table not cleaning\n");
		vpe->idai = false;
		vpe->pending_last = true;
	} else {
	}

	raw_spin_lock_init(&its->lock);
	INIT_LIST_HEAD(&its->entry);
	INIT_LIST_HEAD(&its->its_device_list);
	typer = gic_read_typer(its_base + GITS_TYPER);
	its->base = its_base;
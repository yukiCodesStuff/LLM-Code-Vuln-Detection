	if (err) {
		dev_err(host_pvt.dwc_dev, "%s: dma_request_interrupts returns"
			" %d\n", __func__, err);
		return err;
	}

	/* Enabe DMA */
	out_le32(&(host_pvt.sata_dma_regs->dma_cfg.low), DMA_EN);

	dev_notice(host_pvt.dwc_dev, "DMA initialized\n");
	dev_dbg(host_pvt.dwc_dev, "SATA DMA registers=0x%p\n", host_pvt.\
		sata_dma_regs);

	return 0;
}

static int sata_dwc_scr_read(struct ata_link *link, unsigned int scr, u32 *val)
{
	if (scr > SCR_NOTIFICATION) {
		dev_err(link->ap->dev, "%s: Incorrect SCR offset 0x%02x\n",
			__func__, scr);
		return -EINVAL;
	}

	*val = in_le32((void *)link->ap->ioaddr.scr_addr + (scr * 4));
	dev_dbg(link->ap->dev, "%s: id=%d reg=%d val=val=0x%08x\n",
		__func__, link->ap->print_id, scr, *val);

	return 0;
}

static int sata_dwc_scr_write(struct ata_link *link, unsigned int scr, u32 val)
{
	dev_dbg(link->ap->dev, "%s: id=%d reg=%d val=val=0x%08x\n",
		__func__, link->ap->print_id, scr, val);
	if (scr > SCR_NOTIFICATION) {
		dev_err(link->ap->dev, "%s: Incorrect SCR offset 0x%02x\n",
			 __func__, scr);
		return -EINVAL;
	}
	out_le32((void *)link->ap->ioaddr.scr_addr + (scr * 4), val);

	return 0;
}

static u32 core_scr_read(unsigned int scr)
{
	return in_le32((void __iomem *)(host_pvt.scr_addr_sstatus) +\
			(scr * 4));
}

static void core_scr_write(unsigned int scr, u32 val)
{
	out_le32((void __iomem *)(host_pvt.scr_addr_sstatus) + (scr * 4),
		val);
}

static void clear_serror(void)
{
	u32 val;
	val = core_scr_read(SCR_ERROR);
	core_scr_write(SCR_ERROR, val);

}

static void clear_interrupt_bit(struct sata_dwc_device *hsdev, u32 bit)
{
	out_le32(&hsdev->sata_dwc_regs->intpr,
		 in_le32(&hsdev->sata_dwc_regs->intpr));
}

static u32 qcmd_tag_to_mask(u8 tag)
{
	return 0x00000001 << (tag & 0x1f);
}

/* See ahci.c */
static void sata_dwc_error_intr(struct ata_port *ap,
				struct sata_dwc_device *hsdev, uint intpr)
{
	struct sata_dwc_device_port *hsdevp = HSDEVP_FROM_AP(ap);
	struct ata_eh_info *ehi = &ap->link.eh_info;
	unsigned int err_mask = 0, action = 0;
	struct ata_queued_cmd *qc;
	u32 serror;
	u8 status, tag;
	u32 err_reg;

	ata_ehi_clear_desc(ehi);

	serror = core_scr_read(SCR_ERROR);
	status = ap->ops->sff_check_status(ap);

	err_reg = in_le32(&(host_pvt.sata_dma_regs->interrupt_status.error.\
			low));
	tag = ap->link.active_tag;

	dev_err(ap->dev, "%s SCR_ERROR=0x%08x intpr=0x%08x status=0x%08x "
		"dma_intp=%d pending=%d issued=%d dma_err_status=0x%08x\n",
		__func__, serror, intpr, status, host_pvt.dma_interrupt_count,
		hsdevp->dma_pending[tag], hsdevp->cmd_issued[tag], err_reg);

	/* Clear error register and interrupt bit */
	clear_serror();
	clear_interrupt_bit(hsdev, SATA_DWC_INTPR_ERR);

	/* This is the only error happening now.  TODO check for exact error */

	err_mask |= AC_ERR_HOST_BUS;
	action |= ATA_EH_RESET;

	/* Pass this on to EH */
	ehi->serror |= serror;
	ehi->action |= action;

	qc = ata_qc_from_tag(ap, tag);
	if (qc)
		qc->err_mask |= err_mask;
	else
		ehi->err_mask |= err_mask;

	ata_port_abort(ap);
}

/*
 * Function : sata_dwc_isr
 * arguments : irq, void *dev_instance, struct pt_regs *regs
 * Return value : irqreturn_t - status of IRQ
 * This Interrupt handler called via port ops registered function.
 * .irq_handler = sata_dwc_isr
 */
static irqreturn_t sata_dwc_isr(int irq, void *dev_instance)
{
	struct ata_host *host = (struct ata_host *)dev_instance;
	struct sata_dwc_device *hsdev = HSDEV_FROM_HOST(host);
	struct ata_port *ap;
	struct ata_queued_cmd *qc;
	unsigned long flags;
	u8 status, tag;
	int handled, num_processed, port = 0;
	uint intpr, sactive, sactive2, tag_mask;
	struct sata_dwc_device_port *hsdevp;
	host_pvt.sata_dwc_sactive_issued = 0;

	spin_lock_irqsave(&host->lock, flags);

	/* Read the interrupt register */
	intpr = in_le32(&hsdev->sata_dwc_regs->intpr);

	ap = host->ports[port];
	hsdevp = HSDEVP_FROM_AP(ap);

	dev_dbg(ap->dev, "%s intpr=0x%08x active_tag=%d\n", __func__, intpr,
		ap->link.active_tag);

	/* Check for error interrupt */
	if (intpr & SATA_DWC_INTPR_ERR) {
		sata_dwc_error_intr(ap, hsdev, intpr);
		handled = 1;
		goto DONE;
	}

	/* Check for DMA SETUP FIS (FP DMA) interrupt */
	if (intpr & SATA_DWC_INTPR_NEWFP) {
		clear_interrupt_bit(hsdev, SATA_DWC_INTPR_NEWFP);

		tag = (u8)(in_le32(&hsdev->sata_dwc_regs->fptagr));
		dev_dbg(ap->dev, "%s: NEWFP tag=%d\n", __func__, tag);
		if (hsdevp->cmd_issued[tag] != SATA_DWC_CMD_ISSUED_PEND)
			dev_warn(ap->dev, "CMD tag=%d not pending?\n", tag);

		host_pvt.sata_dwc_sactive_issued |= qcmd_tag_to_mask(tag);

		qc = ata_qc_from_tag(ap, tag);
		/*
		 * Start FP DMA for NCQ command.  At this point the tag is the
		 * active tag.  It is the tag that matches the command about to
		 * be completed.
		 */
		qc->ap->link.active_tag = tag;
		sata_dwc_bmdma_start_by_tag(qc, tag);

		handled = 1;
		goto DONE;
	}
	sactive = core_scr_read(SCR_ACTIVE);
	tag_mask = (host_pvt.sata_dwc_sactive_issued | sactive) ^ sactive;

	/* If no sactive issued and tag_mask is zero then this is not NCQ */
	if (host_pvt.sata_dwc_sactive_issued == 0 && tag_mask == 0) {
		if (ap->link.active_tag == ATA_TAG_POISON)
			tag = 0;
		else
			tag = ap->link.active_tag;
		qc = ata_qc_from_tag(ap, tag);

		/* DEV interrupt w/ no active qc? */
		if (unlikely(!qc || (qc->tf.flags & ATA_TFLAG_POLLING))) {
			dev_err(ap->dev, "%s interrupt with no active qc "
				"qc=%p\n", __func__, qc);
			ap->ops->sff_check_status(ap);
			handled = 1;
			goto DONE;
		}
	if (err) {
		dev_err(host_pvt.dwc_dev, "%s: dma_request_interrupts returns"
			" %d\n", __func__, err);
		return err;
	}

	/* Enabe DMA */
	out_le32(&(host_pvt.sata_dma_regs->dma_cfg.low), DMA_EN);

	dev_notice(host_pvt.dwc_dev, "DMA initialized\n");
	dev_dbg(host_pvt.dwc_dev, "SATA DMA registers=0x%p\n", host_pvt.\
		sata_dma_regs);

	return 0;
}

static int sata_dwc_scr_read(struct ata_link *link, unsigned int scr, u32 *val)
{
	if (scr > SCR_NOTIFICATION) {
		dev_err(link->ap->dev, "%s: Incorrect SCR offset 0x%02x\n",
			__func__, scr);
		return -EINVAL;
	}
{
	struct sata_dwc_device *hsdev;
	u32 idr, versionr;
	char *ver = (char *)&versionr;
	u8 *base = NULL;
	int err = 0;
	int irq;
	struct ata_host *host;
	struct ata_port_info pi = sata_dwc_port_info[0];
	const struct ata_port_info *ppi[] = { &pi, NULL };
	struct device_node *np = ofdev->dev.of_node;
	u32 dma_chan;

	/* Allocate DWC SATA device */
	hsdev = kzalloc(sizeof(*hsdev), GFP_KERNEL);
	if (hsdev == NULL) {
		dev_err(&ofdev->dev, "kmalloc failed for hsdev\n");
		err = -ENOMEM;
		goto error;
	}
	if (irq == NO_IRQ) {
		dev_err(&ofdev->dev, "no SATA DMA irq\n");
		err = -ENODEV;
		goto error_iomap;
	}

	/* Get physical SATA DMA register base address */
	host_pvt.sata_dma_regs = of_iomap(ofdev->dev.of_node, 1);
	if (!(host_pvt.sata_dma_regs)) {
		dev_err(&ofdev->dev, "ioremap failed for AHBDMA register"
			" address\n");
		err = -ENODEV;
		goto error_iomap;
	}

	/* Save dev for later use in dev_xxx() routines */
	host_pvt.dwc_dev = &ofdev->dev;

	/* Initialize AHB DMAC */
	err = dma_dwc_init(hsdev, irq);
	if (err)
		goto error_dma_iomap;

	/* Enable SATA Interrupts */
	sata_dwc_enable_interrupts(hsdev);

	/* Get SATA interrupt number */
	irq = irq_of_parse_and_map(ofdev->dev.of_node, 0);
	if (irq == NO_IRQ) {
		dev_err(&ofdev->dev, "no SATA DMA irq\n");
		err = -ENODEV;
		goto error_out;
	}

	/*
	 * Now, register with libATA core, this will also initiate the
	 * device discovery process, invoking our port_start() handler &
	 * error_handler() to execute a dummy Softreset EH session
	 */
	err = ata_host_activate(host, irq, sata_dwc_isr, 0, &sata_dwc_sht);
	if (err)
		dev_err(&ofdev->dev, "failed to activate host");

	dev_set_drvdata(&ofdev->dev, host);
	return 0;

error_out:
	/* Free SATA DMA resources */
	dma_dwc_exit(hsdev);
error_dma_iomap:
	iounmap((void __iomem *)host_pvt.sata_dma_regs);
error_iomap:
	iounmap(base);
error_kmalloc:
	kfree(hsdev);
error:
	return err;
}

static int sata_dwc_remove(struct platform_device *ofdev)
{
	struct device *dev = &ofdev->dev;
	struct ata_host *host = dev_get_drvdata(dev);
	struct sata_dwc_device *hsdev = host->private_data;

	ata_host_detach(host);
	dev_set_drvdata(dev, NULL);

	/* Free SATA DMA resources */
	dma_dwc_exit(hsdev);

	iounmap((void __iomem *)host_pvt.sata_dma_regs);
	iounmap(hsdev->reg_base);
	kfree(hsdev);
	kfree(host);
	dev_dbg(&ofdev->dev, "done\n");
	return 0;
}

static const struct of_device_id sata_dwc_match[] = {
	{ .compatible = "amcc,sata-460ex", },
	{}
};
MODULE_DEVICE_TABLE(of, sata_dwc_match);

static struct platform_driver sata_dwc_driver = {
	.driver = {
		.name = DRV_NAME,
		.of_match_table = sata_dwc_match,
	},
	.probe = sata_dwc_probe,
	.remove = sata_dwc_remove,
};

module_platform_driver(sata_dwc_driver);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Mark Miesfeld <mmiesfeld@amcc.com>");
MODULE_DESCRIPTION("DesignWare Cores SATA controller low lever driver");
MODULE_VERSION(DRV_VERSION);
	if (!(host_pvt.sata_dma_regs)) {
		dev_err(&ofdev->dev, "ioremap failed for AHBDMA register"
			" address\n");
		err = -ENODEV;
		goto error_iomap;
	}

	/* Save dev for later use in dev_xxx() routines */
	host_pvt.dwc_dev = &ofdev->dev;

	/* Initialize AHB DMAC */
	err = dma_dwc_init(hsdev, irq);
	if (err)
		goto error_dma_iomap;

	/* Enable SATA Interrupts */
	sata_dwc_enable_interrupts(hsdev);

	/* Get SATA interrupt number */
	irq = irq_of_parse_and_map(ofdev->dev.of_node, 0);
	if (irq == NO_IRQ) {
		dev_err(&ofdev->dev, "no SATA DMA irq\n");
		err = -ENODEV;
		goto error_out;
	}

	/*
	 * Now, register with libATA core, this will also initiate the
	 * device discovery process, invoking our port_start() handler &
	 * error_handler() to execute a dummy Softreset EH session
	 */
	err = ata_host_activate(host, irq, sata_dwc_isr, 0, &sata_dwc_sht);
	if (err)
		dev_err(&ofdev->dev, "failed to activate host");

	dev_set_drvdata(&ofdev->dev, host);
	return 0;

error_out:
	/* Free SATA DMA resources */
	dma_dwc_exit(hsdev);
error_dma_iomap:
	iounmap((void __iomem *)host_pvt.sata_dma_regs);
error_iomap:
	iounmap(base);
error_kmalloc:
	kfree(hsdev);
error:
	return err;
}

static int sata_dwc_remove(struct platform_device *ofdev)
{
	struct device *dev = &ofdev->dev;
	struct ata_host *host = dev_get_drvdata(dev);
	struct sata_dwc_device *hsdev = host->private_data;

	ata_host_detach(host);
	dev_set_drvdata(dev, NULL);

	/* Free SATA DMA resources */
	dma_dwc_exit(hsdev);

	iounmap((void __iomem *)host_pvt.sata_dma_regs);
	iounmap(hsdev->reg_base);
	kfree(hsdev);
	kfree(host);
	dev_dbg(&ofdev->dev, "done\n");
	return 0;
}

static const struct of_device_id sata_dwc_match[] = {
	{ .compatible = "amcc,sata-460ex", },
	{}
};
MODULE_DEVICE_TABLE(of, sata_dwc_match);

static struct platform_driver sata_dwc_driver = {
	.driver = {
		.name = DRV_NAME,
		.of_match_table = sata_dwc_match,
	},
	.probe = sata_dwc_probe,
	.remove = sata_dwc_remove,
};

module_platform_driver(sata_dwc_driver);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Mark Miesfeld <mmiesfeld@amcc.com>");
MODULE_DESCRIPTION("DesignWare Cores SATA controller low lever driver");
MODULE_VERSION(DRV_VERSION);
	if (irq == NO_IRQ) {
		dev_err(&ofdev->dev, "no SATA DMA irq\n");
		err = -ENODEV;
		goto error_out;
	}

	/*
	 * Now, register with libATA core, this will also initiate the
	 * device discovery process, invoking our port_start() handler &
	 * error_handler() to execute a dummy Softreset EH session
	 */
	err = ata_host_activate(host, irq, sata_dwc_isr, 0, &sata_dwc_sht);
	if (err)
		dev_err(&ofdev->dev, "failed to activate host");

	dev_set_drvdata(&ofdev->dev, host);
	return 0;

error_out:
	/* Free SATA DMA resources */
	dma_dwc_exit(hsdev);
error_dma_iomap:
	iounmap((void __iomem *)host_pvt.sata_dma_regs);
error_iomap:
	iounmap(base);
error_kmalloc:
	kfree(hsdev);
error:
	return err;
}

static int sata_dwc_remove(struct platform_device *ofdev)
{
	struct device *dev = &ofdev->dev;
	struct ata_host *host = dev_get_drvdata(dev);
	struct sata_dwc_device *hsdev = host->private_data;

	ata_host_detach(host);
	dev_set_drvdata(dev, NULL);

	/* Free SATA DMA resources */
	dma_dwc_exit(hsdev);

	iounmap((void __iomem *)host_pvt.sata_dma_regs);
	iounmap(hsdev->reg_base);
	kfree(hsdev);
	kfree(host);
	dev_dbg(&ofdev->dev, "done\n");
	return 0;
}

static const struct of_device_id sata_dwc_match[] = {
	{ .compatible = "amcc,sata-460ex", },
	{}
};
MODULE_DEVICE_TABLE(of, sata_dwc_match);

static struct platform_driver sata_dwc_driver = {
	.driver = {
		.name = DRV_NAME,
		.of_match_table = sata_dwc_match,
	},
	.probe = sata_dwc_probe,
	.remove = sata_dwc_remove,
};

module_platform_driver(sata_dwc_driver);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Mark Miesfeld <mmiesfeld@amcc.com>");
MODULE_DESCRIPTION("DesignWare Cores SATA controller low lever driver");
MODULE_VERSION(DRV_VERSION);
	if (irq == NO_IRQ) {
		dev_err(&ofdev->dev, "no SATA DMA irq\n");
		err = -ENODEV;
		goto error_out;
	}

	/*
	 * Now, register with libATA core, this will also initiate the
	 * device discovery process, invoking our port_start() handler &
	 * error_handler() to execute a dummy Softreset EH session
	 */
	err = ata_host_activate(host, irq, sata_dwc_isr, 0, &sata_dwc_sht);
	if (err)
		dev_err(&ofdev->dev, "failed to activate host");

	dev_set_drvdata(&ofdev->dev, host);
	return 0;

error_out:
	/* Free SATA DMA resources */
	dma_dwc_exit(hsdev);
error_dma_iomap:
	iounmap((void __iomem *)host_pvt.sata_dma_regs);
error_iomap:
	iounmap(base);
error_kmalloc:
	kfree(hsdev);
error:
	return err;
}

static int sata_dwc_remove(struct platform_device *ofdev)
{
	struct device *dev = &ofdev->dev;
	struct ata_host *host = dev_get_drvdata(dev);
	struct sata_dwc_device *hsdev = host->private_data;

	ata_host_detach(host);
	dev_set_drvdata(dev, NULL);

	/* Free SATA DMA resources */
	dma_dwc_exit(hsdev);

	iounmap((void __iomem *)host_pvt.sata_dma_regs);
	iounmap(hsdev->reg_base);
	kfree(hsdev);
	kfree(host);
	dev_dbg(&ofdev->dev, "done\n");
	return 0;
}

static const struct of_device_id sata_dwc_match[] = {
	{ .compatible = "amcc,sata-460ex", },
	{}
};
MODULE_DEVICE_TABLE(of, sata_dwc_match);

static struct platform_driver sata_dwc_driver = {
	.driver = {
		.name = DRV_NAME,
		.of_match_table = sata_dwc_match,
	},
	.probe = sata_dwc_probe,
	.remove = sata_dwc_remove,
};

module_platform_driver(sata_dwc_driver);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Mark Miesfeld <mmiesfeld@amcc.com>");
MODULE_DESCRIPTION("DesignWare Cores SATA controller low lever driver");
MODULE_VERSION(DRV_VERSION);
{
	struct device *dev = &ofdev->dev;
	struct ata_host *host = dev_get_drvdata(dev);
	struct sata_dwc_device *hsdev = host->private_data;

	ata_host_detach(host);
	dev_set_drvdata(dev, NULL);

	/* Free SATA DMA resources */
	dma_dwc_exit(hsdev);

	iounmap((void __iomem *)host_pvt.sata_dma_regs);
	iounmap(hsdev->reg_base);
	kfree(hsdev);
	kfree(host);
	dev_dbg(&ofdev->dev, "done\n");
	return 0;
}

static const struct of_device_id sata_dwc_match[] = {
	{ .compatible = "amcc,sata-460ex", },
	{}
};
MODULE_DEVICE_TABLE(of, sata_dwc_match);

static struct platform_driver sata_dwc_driver = {
	.driver = {
		.name = DRV_NAME,
		.of_match_table = sata_dwc_match,
	},
	.probe = sata_dwc_probe,
	.remove = sata_dwc_remove,
};

module_platform_driver(sata_dwc_driver);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Mark Miesfeld <mmiesfeld@amcc.com>");
MODULE_DESCRIPTION("DesignWare Cores SATA controller low lever driver");
MODULE_VERSION(DRV_VERSION);
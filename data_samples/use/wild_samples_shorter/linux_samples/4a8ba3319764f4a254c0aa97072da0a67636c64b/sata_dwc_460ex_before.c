	if (err) {
		dev_err(host_pvt.dwc_dev, "%s: dma_request_interrupts returns"
			" %d\n", __func__, err);
		goto error_out;
	}

	/* Enabe DMA */
	out_le32(&(host_pvt.sata_dma_regs->dma_cfg.low), DMA_EN);
		sata_dma_regs);

	return 0;

error_out:
	dma_dwc_exit(hsdev);

	return err;
}

static int sata_dwc_scr_read(struct ata_link *link, unsigned int scr, u32 *val)
{
	char *ver = (char *)&versionr;
	u8 *base = NULL;
	int err = 0;
	int irq, rc;
	struct ata_host *host;
	struct ata_port_info pi = sata_dwc_port_info[0];
	const struct ata_port_info *ppi[] = { &pi, NULL };
	struct device_node *np = ofdev->dev.of_node;
	if (irq == NO_IRQ) {
		dev_err(&ofdev->dev, "no SATA DMA irq\n");
		err = -ENODEV;
		goto error_out;
	}

	/* Get physical SATA DMA register base address */
	host_pvt.sata_dma_regs = of_iomap(ofdev->dev.of_node, 1);
		dev_err(&ofdev->dev, "ioremap failed for AHBDMA register"
			" address\n");
		err = -ENODEV;
		goto error_out;
	}

	/* Save dev for later use in dev_xxx() routines */
	host_pvt.dwc_dev = &ofdev->dev;

	/* Initialize AHB DMAC */
	dma_dwc_init(hsdev, irq);

	/* Enable SATA Interrupts */
	sata_dwc_enable_interrupts(hsdev);

	 * device discovery process, invoking our port_start() handler &
	 * error_handler() to execute a dummy Softreset EH session
	 */
	rc = ata_host_activate(host, irq, sata_dwc_isr, 0, &sata_dwc_sht);

	if (rc != 0)
		dev_err(&ofdev->dev, "failed to activate host");

	dev_set_drvdata(&ofdev->dev, host);
	return 0;
error_out:
	/* Free SATA DMA resources */
	dma_dwc_exit(hsdev);

error_iomap:
	iounmap(base);
error_kmalloc:
	kfree(hsdev);
	/* Free SATA DMA resources */
	dma_dwc_exit(hsdev);

	iounmap(hsdev->reg_base);
	kfree(hsdev);
	kfree(host);
	dev_dbg(&ofdev->dev, "done\n");
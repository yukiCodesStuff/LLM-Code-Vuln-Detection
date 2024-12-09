	}
	/* Global interrupt queue */
	writel((u32)(((IRQ_RING_SIZE >> 5) - 1) << 20), ioaddr + IQLENR1);
	priv->iqcfg = (__le32 *) pci_alloc_consistent(pdev,
		IRQ_RING_SIZE*sizeof(__le32), &priv->iqcfg_dma);
	if (!priv->iqcfg)
		goto err_free_irq_5;
	writel(priv->iqcfg_dma, ioaddr + IQCFG);

	rc = -ENOMEM;

	/*
	 * SCC 0-3 private rx/tx irq structures
	 * IQRX/TXi needs to be set soon. Learned it the hard way...
	 */
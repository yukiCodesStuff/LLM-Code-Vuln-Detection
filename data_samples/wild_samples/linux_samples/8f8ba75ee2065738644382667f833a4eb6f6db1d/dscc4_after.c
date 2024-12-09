	{
		u32 bits;

		bits = (IRQ_RING_SIZE >> 5) - 1;
		bits |= bits << 4;
		bits |= bits << 8;
		bits |= bits << 16;
		writel(bits, ioaddr + IQLENR0);
	}
	/* Global interrupt queue */
	writel((u32)(((IRQ_RING_SIZE >> 5) - 1) << 20), ioaddr + IQLENR1);

	rc = -ENOMEM;

	priv->iqcfg = (__le32 *) pci_alloc_consistent(pdev,
		IRQ_RING_SIZE*sizeof(__le32), &priv->iqcfg_dma);
	if (!priv->iqcfg)
		goto err_free_irq_5;
	writel(priv->iqcfg_dma, ioaddr + IQCFG);

	/*
	 * SCC 0-3 private rx/tx irq structures
	 * IQRX/TXi needs to be set soon. Learned it the hard way...
	 */
	for (i = 0; i < dev_per_card; i++) {
		dpriv = priv->root + i;
		dpriv->iqtx = (__le32 *) pci_alloc_consistent(pdev,
			IRQ_RING_SIZE*sizeof(u32), &dpriv->iqtx_dma);
		if (!dpriv->iqtx)
			goto err_free_iqtx_6;
		writel(dpriv->iqtx_dma, ioaddr + IQTX0 + i*4);
	}
{
	struct dw_dma		*dw;
	bool			autocfg;
	unsigned int		dw_params;
	unsigned int		nr_channels;
	unsigned int		max_blk_size = 0;
	int			err;
	int			i;

	dw = devm_kzalloc(chip->dev, sizeof(*dw), GFP_KERNEL);
	if (!dw)
		return -ENOMEM;

	dw->regs = chip->regs;
	chip->dw = dw;

	pm_runtime_get_sync(chip->dev);

	dw_params = dma_read_byaddr(chip->regs, DW_PARAMS);
	autocfg = dw_params >> DW_PARAMS_EN & 0x1;

	dev_dbg(chip->dev, "DW_PARAMS: 0x%08x\n", dw_params);

	if (!pdata && autocfg) {
		pdata = devm_kzalloc(chip->dev, sizeof(*pdata), GFP_KERNEL);
		if (!pdata) {
			err = -ENOMEM;
			goto err_pdata;
		}

		/* Fill platform data with the default values */
		pdata->is_private = true;
		pdata->chan_allocation_order = CHAN_ALLOCATION_ASCENDING;
		pdata->chan_priority = CHAN_PRIORITY_ASCENDING;
	} else if (!pdata || pdata->nr_channels > DW_DMA_MAX_NR_CHANNELS) {
		err = -EINVAL;
		goto err_pdata;
	}

	if (autocfg)
		nr_channels = (dw_params >> DW_PARAMS_NR_CHAN & 0x7) + 1;
	else
		nr_channels = pdata->nr_channels;

	dw->chan = devm_kcalloc(chip->dev, nr_channels, sizeof(*dw->chan),
				GFP_KERNEL);
	if (!dw->chan) {
		err = -ENOMEM;
		goto err_pdata;
	}

	/* Get hardware configuration parameters */
	if (autocfg) {
		max_blk_size = dma_readl(dw, MAX_BLK_SIZE);

		dw->nr_masters = (dw_params >> DW_PARAMS_NR_MASTER & 3) + 1;
		for (i = 0; i < dw->nr_masters; i++) {
			dw->data_width[i] =
				(dw_params >> DW_PARAMS_DATA_WIDTH(i) & 3) + 2;
		}
	} else {
		dw->nr_masters = pdata->nr_masters;
		memcpy(dw->data_width, pdata->data_width, 4);
	}

	/* Calculate all channel mask before DMA setup */
	dw->all_chan_mask = (1 << nr_channels) - 1;

	/* Force dma off, just in case */
	dw_dma_off(dw);

	/* Disable BLOCK interrupts as well */
	channel_clear_bit(dw, MASK.BLOCK, dw->all_chan_mask);

	/* Create a pool of consistent memory blocks for hardware descriptors */
	dw->desc_pool = dmam_pool_create("dw_dmac_desc_pool", chip->dev,
					 sizeof(struct dw_desc), 4, 0);
	if (!dw->desc_pool) {
		dev_err(chip->dev, "No memory for descriptors dma pool\n");
		err = -ENOMEM;
		goto err_pdata;
	}

	tasklet_init(&dw->tasklet, dw_dma_tasklet, (unsigned long)dw);

	err = request_irq(chip->irq, dw_dma_interrupt, IRQF_SHARED,
			  "dw_dmac", dw);
	if (err)
		goto err_pdata;

	INIT_LIST_HEAD(&dw->dma.channels);
	for (i = 0; i < nr_channels; i++) {
		struct dw_dma_chan	*dwc = &dw->chan[i];
		int			r = nr_channels - i - 1;

		dwc->chan.device = &dw->dma;
		dma_cookie_init(&dwc->chan);
		if (pdata->chan_allocation_order == CHAN_ALLOCATION_ASCENDING)
			list_add_tail(&dwc->chan.device_node,
					&dw->dma.channels);
		else
			list_add(&dwc->chan.device_node, &dw->dma.channels);

		/* 7 is highest priority & 0 is lowest. */
		if (pdata->chan_priority == CHAN_PRIORITY_ASCENDING)
			dwc->priority = r;
		else
			dwc->priority = i;

		dwc->ch_regs = &__dw_regs(dw)->CHAN[i];
		spin_lock_init(&dwc->lock);
		dwc->mask = 1 << i;

		INIT_LIST_HEAD(&dwc->active_list);
		INIT_LIST_HEAD(&dwc->queue);
		INIT_LIST_HEAD(&dwc->free_list);

		channel_clear_bit(dw, CH_EN, dwc->mask);

		dwc->direction = DMA_TRANS_NONE;

		/* Hardware configuration */
		if (autocfg) {
			unsigned int dwc_params;
			void __iomem *addr = chip->regs + r * sizeof(u32);

			dwc_params = dma_read_byaddr(addr, DWC_PARAMS);

			dev_dbg(chip->dev, "DWC_PARAMS[%d]: 0x%08x\n", i,
					   dwc_params);

			/*
			 * Decode maximum block size for given channel. The
			 * stored 4 bit value represents blocks from 0x00 for 3
			 * up to 0x0a for 4095.
			 */
			dwc->block_size =
				(4 << ((max_blk_size >> 4 * i) & 0xf)) - 1;
			dwc->nollp =
				(dwc_params >> DWC_PARAMS_MBLK_EN & 0x1) == 0;
		} else {
			dwc->block_size = pdata->block_size;

			/* Check if channel supports multi block transfer */
			channel_writel(dwc, LLP, 0xfffffffc);
			dwc->nollp =
				(channel_readl(dwc, LLP) & 0xfffffffc) == 0;
			channel_writel(dwc, LLP, 0);
		}
	}

	/* Clear all interrupts on all channels. */
	dma_writel(dw, CLEAR.XFER, dw->all_chan_mask);
	dma_writel(dw, CLEAR.BLOCK, dw->all_chan_mask);
	dma_writel(dw, CLEAR.SRC_TRAN, dw->all_chan_mask);
	dma_writel(dw, CLEAR.DST_TRAN, dw->all_chan_mask);
	dma_writel(dw, CLEAR.ERROR, dw->all_chan_mask);

	dma_cap_set(DMA_MEMCPY, dw->dma.cap_mask);
	dma_cap_set(DMA_SLAVE, dw->dma.cap_mask);
	if (pdata->is_private)
		dma_cap_set(DMA_PRIVATE, dw->dma.cap_mask);
	dw->dma.dev = chip->dev;
	dw->dma.device_alloc_chan_resources = dwc_alloc_chan_resources;
	dw->dma.device_free_chan_resources = dwc_free_chan_resources;

	dw->dma.device_prep_dma_memcpy = dwc_prep_dma_memcpy;

	dw->dma.device_prep_slave_sg = dwc_prep_slave_sg;
	dw->dma.device_control = dwc_control;

	dw->dma.device_tx_status = dwc_tx_status;
	dw->dma.device_issue_pending = dwc_issue_pending;

	err = dma_async_device_register(&dw->dma);
	if (err)
		goto err_dma_register;

	dev_info(chip->dev, "DesignWare DMA Controller, %d channels\n",
		 nr_channels);

	pm_runtime_put_sync_suspend(chip->dev);

	return 0;

err_dma_register:
	free_irq(chip->irq, dw);
err_pdata:
	pm_runtime_put_sync_suspend(chip->dev);
	return err;
}
EXPORT_SYMBOL_GPL(dw_dma_probe);

int dw_dma_remove(struct dw_dma_chip *chip)
{
	struct dw_dma		*dw = chip->dw;
	struct dw_dma_chan	*dwc, *_dwc;

	pm_runtime_get_sync(chip->dev);

	dw_dma_off(dw);
	dma_async_device_unregister(&dw->dma);

	free_irq(chip->irq, dw);
	tasklet_kill(&dw->tasklet);

	list_for_each_entry_safe(dwc, _dwc, &dw->dma.channels,
			chan.device_node) {
		list_del(&dwc->chan.device_node);
		channel_clear_bit(dw, CH_EN, dwc->mask);
	}

	pm_runtime_put_sync_suspend(chip->dev);
	return 0;
}
EXPORT_SYMBOL_GPL(dw_dma_remove);

int dw_dma_disable(struct dw_dma_chip *chip)
{
	struct dw_dma *dw = chip->dw;

	dw_dma_off(dw);
	return 0;
}
EXPORT_SYMBOL_GPL(dw_dma_disable);

int dw_dma_enable(struct dw_dma_chip *chip)
{
	struct dw_dma *dw = chip->dw;

	dw_dma_on(dw);
	return 0;
}
EXPORT_SYMBOL_GPL(dw_dma_enable);

MODULE_LICENSE("GPL v2");
MODULE_DESCRIPTION("Synopsys DesignWare DMA Controller core driver");
MODULE_AUTHOR("Haavard Skinnemoen (Atmel)");
MODULE_AUTHOR("Viresh Kumar <viresh.linux@gmail.com>");
			chan.device_node) {
		list_del(&dwc->chan.device_node);
		channel_clear_bit(dw, CH_EN, dwc->mask);
	}

	pm_runtime_put_sync_suspend(chip->dev);
	return 0;
}
EXPORT_SYMBOL_GPL(dw_dma_remove);

int dw_dma_disable(struct dw_dma_chip *chip)
{
	struct dw_dma *dw = chip->dw;

	dw_dma_off(dw);
	return 0;
}
EXPORT_SYMBOL_GPL(dw_dma_disable);

int dw_dma_enable(struct dw_dma_chip *chip)
{
	struct dw_dma *dw = chip->dw;

	dw_dma_on(dw);
	return 0;
}
EXPORT_SYMBOL_GPL(dw_dma_enable);

MODULE_LICENSE("GPL v2");
MODULE_DESCRIPTION("Synopsys DesignWare DMA Controller core driver");
MODULE_AUTHOR("Haavard Skinnemoen (Atmel)");
MODULE_AUTHOR("Viresh Kumar <viresh.linux@gmail.com>");
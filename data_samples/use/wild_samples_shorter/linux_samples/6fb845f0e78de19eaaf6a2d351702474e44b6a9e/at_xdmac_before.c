	u32				save_cim;
	u32				save_cnda;
	u32				save_cndc;
	unsigned long			status;
	struct tasklet_struct		tasklet;
	struct dma_slave_config		sconfig;

	struct at_xdmac_desc	*desc;
	u32			error_mask;

	dev_dbg(chan2dev(&atchan->chan), "%s: status=0x%08lx\n",
		 __func__, atchan->status);

	error_mask = AT_XDMAC_CIS_RBEIS
		     | AT_XDMAC_CIS_WBEIS
		     | AT_XDMAC_CIS_ROIS;

	if (at_xdmac_chan_is_cyclic(atchan)) {
		at_xdmac_handle_cyclic(atchan);
	} else if ((atchan->status & AT_XDMAC_CIS_LIS)
		   || (atchan->status & error_mask)) {
		struct dma_async_tx_descriptor  *txd;

		if (atchan->status & AT_XDMAC_CIS_RBEIS)
			dev_err(chan2dev(&atchan->chan), "read bus error!!!");
		if (atchan->status & AT_XDMAC_CIS_WBEIS)
			dev_err(chan2dev(&atchan->chan), "write bus error!!!");
		if (atchan->status & AT_XDMAC_CIS_ROIS)
			dev_err(chan2dev(&atchan->chan), "request overflow error!!!");

		spin_lock(&atchan->lock);
		desc = list_first_entry(&atchan->xfers_list,
			atchan = &atxdmac->chan[i];
			chan_imr = at_xdmac_chan_read(atchan, AT_XDMAC_CIM);
			chan_status = at_xdmac_chan_read(atchan, AT_XDMAC_CIS);
			atchan->status = chan_status & chan_imr;
			dev_vdbg(atxdmac->dma.dev,
				 "%s: chan%d: imr=0x%x, status=0x%x\n",
				 __func__, i, chan_imr, chan_status);
			dev_vdbg(chan2dev(&atchan->chan),
				 at_xdmac_chan_read(atchan, AT_XDMAC_CDA),
				 at_xdmac_chan_read(atchan, AT_XDMAC_CUBC));

			if (atchan->status & (AT_XDMAC_CIS_RBEIS | AT_XDMAC_CIS_WBEIS))
				at_xdmac_write(atxdmac, AT_XDMAC_GD, atchan->mask);

			tasklet_schedule(&atchan->tasklet);
			ret = IRQ_HANDLED;
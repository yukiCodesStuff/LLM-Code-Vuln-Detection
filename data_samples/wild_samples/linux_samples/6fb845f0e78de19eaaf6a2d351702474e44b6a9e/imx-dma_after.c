{
	struct imxdma_channel *imxdmac = (void *)data;
	struct imxdma_engine *imxdma = imxdmac->imxdma;
	struct imxdma_desc *desc, *next_desc;
	unsigned long flags;

	spin_lock_irqsave(&imxdma->lock, flags);

	if (list_empty(&imxdmac->ld_active)) {
		/* Someone might have called terminate all */
		spin_unlock_irqrestore(&imxdma->lock, flags);
		return;
	}
	if (imxdmac->enabled_2d) {
		imxdma->slots_2d[imxdmac->slot_2d].count--;
		imxdmac->enabled_2d = false;
	}

	list_move_tail(imxdmac->ld_active.next, &imxdmac->ld_free);

	if (!list_empty(&imxdmac->ld_queue)) {
		next_desc = list_first_entry(&imxdmac->ld_queue,
					     struct imxdma_desc, node);
		list_move_tail(imxdmac->ld_queue.next, &imxdmac->ld_active);
		if (imxdma_xfer_desc(next_desc) < 0)
			dev_warn(imxdma->dev, "%s: channel: %d couldn't xfer desc\n",
				 __func__, imxdmac->channel);
	}
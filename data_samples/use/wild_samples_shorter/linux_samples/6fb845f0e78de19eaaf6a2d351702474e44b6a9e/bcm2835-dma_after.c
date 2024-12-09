	}
}

static int bcm2835_dma_abort(struct bcm2835_chan *c)
{
	void __iomem *chan_base = c->chan_base;
	long int timeout = 10000;

	/*
	 * A zero control block address means the channel is idle.
	 * (The ACTIVE flag in the CS register is not a reliable indicator.)
	 */
	if (!readl(chan_base + BCM2835_DMA_ADDR))
		return 0;

	/* Write 0 to the active bit - Pause the DMA */
	writel(0, chan_base + BCM2835_DMA_CS);

	/* Wait for any current AXI transfer to complete */
	while ((readl(chan_base + BCM2835_DMA_CS) &
		BCM2835_DMA_WAITING_FOR_WRITES) && --timeout)
		cpu_relax();

	/* Peripheral might be stuck and fail to signal AXI write responses */
	if (!timeout)
		dev_err(c->vc.chan.device->dev,
			"failed to complete outstanding writes\n");

	writel(BCM2835_DMA_RESET, chan_base + BCM2835_DMA_CS);
	return 0;
}

static void bcm2835_dma_start_desc(struct bcm2835_chan *c)

	spin_lock_irqsave(&c->vc.lock, flags);

	/*
	 * Clear the INT flag to receive further interrupts. Keep the channel
	 * active in case the descriptor is cyclic or in case the client has
	 * already terminated the descriptor and issued a new one. (May happen
	 * if this IRQ handler is threaded.) If the channel is finished, it
	 * will remain idle despite the ACTIVE flag being set.
	 */
	writel(BCM2835_DMA_INT | BCM2835_DMA_ACTIVE,
	       c->chan_base + BCM2835_DMA_CS);

	d = c->desc;

	if (d) {
		if (d->cyclic) {
			/* call the cyclic callback */
			vchan_cyclic_callback(&d->vd);
		} else if (!readl(c->chan_base + BCM2835_DMA_ADDR)) {
			vchan_cookie_complete(&c->desc->vd);
			bcm2835_dma_start_desc(c);
		}
	}
	struct bcm2835_chan *c = to_bcm2835_dma_chan(chan);
	struct bcm2835_dmadev *d = to_bcm2835_dma_dev(c->vc.chan.device);
	unsigned long flags;
	LIST_HEAD(head);

	spin_lock_irqsave(&c->vc.lock, flags);

	list_del_init(&c->node);
	spin_unlock(&d->lock);

	/* stop DMA activity */
	if (c->desc) {
		vchan_terminate_vdesc(&c->desc->vd);
		c->desc = NULL;
		bcm2835_dma_abort(c);
	}

	vchan_get_all_descriptors(&c->vc, &head);
	spin_unlock_irqrestore(&c->vc.lock, flags);
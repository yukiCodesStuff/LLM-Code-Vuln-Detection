/* level of occupied TX descriptors under which we wake up TX process */
#define MACB_TX_WAKEUP_THRESH(bp)	(3 * (bp)->tx_ring_size / 4)

#define MACB_RX_INT_FLAGS	(MACB_BIT(RCOMP) | MACB_BIT(ISR_ROVR))
#define MACB_TX_ERR_FLAGS	(MACB_BIT(ISR_TUND)			\
					| MACB_BIT(ISR_RLE)		\
					| MACB_BIT(TXERR))
#define MACB_TX_INT_FLAGS	(MACB_TX_ERR_FLAGS | MACB_BIT(TCOMP)	\
				queue_writel(queue, ISR, MACB_BIT(RCOMP));
			napi_reschedule(napi);
		} else {
			queue_writel(queue, IER, bp->rx_intr_mask);
		}
	}

	/* TODO: Handle errors */
	u32 ctrl;

	for (q = 0, queue = bp->queues; q < bp->num_queues; ++q, ++queue) {
		queue_writel(queue, IDR, bp->rx_intr_mask |
					 MACB_TX_INT_FLAGS |
					 MACB_BIT(HRESP));
	}
	ctrl = macb_readl(bp, NCR);

		/* Enable interrupts */
		queue_writel(queue, IER,
			     bp->rx_intr_mask |
			     MACB_TX_INT_FLAGS |
			     MACB_BIT(HRESP));
	}

			    (unsigned int)(queue - bp->queues),
			    (unsigned long)status);

		if (status & bp->rx_intr_mask) {
			/* There's no point taking any more interrupts
			 * until we have processed the buffers. The
			 * scheduling call may fail if the poll routine
			 * is already scheduled, so disable interrupts
			 * now.
			 */
			queue_writel(queue, IDR, bp->rx_intr_mask);
			if (bp->caps & MACB_CAPS_ISR_CLEAR_ON_WRITE)
				queue_writel(queue, ISR, MACB_BIT(RCOMP));

			if (napi_schedule_prep(&queue->napi)) {
		/* There is a hardware issue under heavy load where DMA can
		 * stop, this causes endless "used buffer descriptor read"
		 * interrupts but it can be cleared by re-enabling RX. See
		 * the at91rm9200 manual, section 41.3.1 or the Zynq manual
		 * section 16.7.4 for details. RXUBR is only enabled for
		 * these two versions.
		 */
		if (status & MACB_BIT(RXUBR)) {
			ctrl = macb_readl(bp, NCR);
			macb_writel(bp, NCR, ctrl & ~MACB_BIT(RE));

		/* Enable interrupts */
		queue_writel(queue, IER,
			     bp->rx_intr_mask |
			     MACB_TX_INT_FLAGS |
			     MACB_BIT(HRESP));
	}

};

static const struct macb_config emac_config = {
	.caps = MACB_CAPS_NEEDS_RSTONUBR,
	.clk_init = at91ether_clk_init,
	.init = at91ether_init,
};

};

static const struct macb_config zynq_config = {
	.caps = MACB_CAPS_GIGABIT_MODE_AVAILABLE | MACB_CAPS_NO_GIGABIT_HALF |
		MACB_CAPS_NEEDS_RSTONUBR,
	.dma_burst_length = 16,
	.clk_init = macb_clk_init,
	.init = macb_init,
};
						macb_dma_desc_get_size(bp);
	}

	bp->rx_intr_mask = MACB_RX_INT_FLAGS;
	if (bp->caps & MACB_CAPS_NEEDS_RSTONUBR)
		bp->rx_intr_mask |= MACB_BIT(RXUBR);

	mac = of_get_mac_address(np);
	if (mac) {
		ether_addr_copy(bp->dev->dev_addr, mac);
	} else {
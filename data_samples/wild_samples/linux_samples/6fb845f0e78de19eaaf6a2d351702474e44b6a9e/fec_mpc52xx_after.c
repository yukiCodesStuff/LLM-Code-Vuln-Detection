	while (bcom_buffer_done(priv->tx_dmatsk)) {
		struct sk_buff *skb;
		struct bcom_fec_bd *bd;
		skb = bcom_retrieve_buffer(priv->tx_dmatsk, NULL,
				(struct bcom_bd **)&bd);
		dma_unmap_single(dev->dev.parent, bd->skb_pa, skb->len,
				 DMA_TO_DEVICE);

		dev_consume_skb_irq(skb);
	}
	spin_unlock(&priv->lock);

	netif_wake_queue(dev);

	return IRQ_HANDLED;
}

static irqreturn_t mpc52xx_fec_rx_interrupt(int irq, void *dev_id)
{
	struct net_device *dev = dev_id;
	struct mpc52xx_fec_priv *priv = netdev_priv(dev);
	struct sk_buff *rskb; /* received sk_buff */
	struct sk_buff *skb;  /* new sk_buff to enqueue in its place */
	struct bcom_fec_bd *bd;
	u32 status, physaddr;
	int length;

	spin_lock(&priv->lock);

	while (bcom_buffer_done(priv->rx_dmatsk)) {

		rskb = bcom_retrieve_buffer(priv->rx_dmatsk, &status,
					    (struct bcom_bd **)&bd);
		physaddr = bd->skb_pa;

		/* Test for errors in received frame */
		if (status & BCOM_FEC_RX_BD_ERRORS) {
			/* Drop packet and reuse the buffer */
			mpc52xx_fec_rx_submit(dev, rskb);
			dev->stats.rx_dropped++;
			continue;
		}
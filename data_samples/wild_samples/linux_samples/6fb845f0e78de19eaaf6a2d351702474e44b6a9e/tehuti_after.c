		do {
			BDX_ASSERT(db->rptr->addr.dma == 0);
			pci_unmap_page(priv->pdev, db->rptr->addr.dma,
				       db->rptr->len, PCI_DMA_TODEVICE);
			bdx_tx_db_inc_rptr(db);
		} while (db->rptr->len > 0);
		tx_level -= db->rptr->len;	/* '-' koz len is negative */

		/* now should come skb pointer - free it */
		dev_consume_skb_irq(db->rptr->addr.skb);
		bdx_tx_db_inc_rptr(db);
	}

	/* let h/w know which TXF descriptors were cleaned */
	BDX_ASSERT((f->m.wptr & TXF_WPTR_WR_PTR) >= f->m.memsz);
	WRITE_REG(priv, f->m.reg_RPTR, f->m.rptr & TXF_WPTR_WR_PTR);

	/* We reclaimed resources, so in case the Q is stopped by xmit callback,
	 * we resume the transmission and use tx_lock to synchronize with xmit.*/
	spin_lock(&priv->tx_lock);
	priv->tx_level += tx_level;
	BDX_ASSERT(priv->tx_level <= 0 || priv->tx_level > BDX_MAX_TX_LEVEL);
#ifdef BDX_DELAY_WPTR
	if (priv->tx_noupd) {
		priv->tx_noupd = 0;
		WRITE_REG(priv, priv->txd_fifo0.m.reg_WPTR,
			  priv->txd_fifo0.m.wptr & TXF_WPTR_WR_PTR);
	}
#endif

	if (unlikely(netif_queue_stopped(priv->ndev) &&
		     netif_carrier_ok(priv->ndev) &&
		     (priv->tx_level >= BDX_MIN_TX_LEVEL))) {
		DBG("%s: %s: TX Q WAKE level %d\n",
		    BDX_DRV_NAME, priv->ndev->name, priv->tx_level);
		netif_wake_queue(priv->ndev);
	}
	spin_unlock(&priv->tx_lock);
}

/**
 * bdx_tx_free_skbs - frees all skbs from TXD fifo.
 * It gets called when OS stops this dev, eg upon "ifconfig down" or rmmod
 */
static void bdx_tx_free_skbs(struct bdx_priv *priv)
{
	struct txdb *db = &priv->txdb;

	ENTER;
	while (db->rptr != db->wptr) {
		if (likely(db->rptr->len))
			pci_unmap_page(priv->pdev, db->rptr->addr.dma,
				       db->rptr->len, PCI_DMA_TODEVICE);
		else
			dev_kfree_skb(db->rptr->addr.skb);
		bdx_tx_db_inc_rptr(db);
	}
	RET();
}

/* bdx_tx_free - frees all Tx resources */
static void bdx_tx_free(struct bdx_priv *priv)
{
	ENTER;
	bdx_tx_free_skbs(priv);
	bdx_fifo_free(priv, &priv->txd_fifo0.m);
	bdx_fifo_free(priv, &priv->txf_fifo0.m);
	bdx_tx_db_close(&priv->txdb);
}

/**
 * bdx_tx_push_desc - push descriptor to TxD fifo
 * @priv: NIC private structure
 * @data: desc's data
 * @size: desc's size
 *
 * Pushes desc to TxD fifo and overlaps it if needed.
 * NOTE: this func does not check for available space. this is responsibility
 *    of the caller. Neither does it check that data size is smaller than
 *    fifo size.
 */
static void bdx_tx_push_desc(struct bdx_priv *priv, void *data, int size)
{
	struct txd_fifo *f = &priv->txd_fifo0;
	int i = f->m.memsz - f->m.wptr;

	if (size == 0)
		return;

	if (i > size) {
		memcpy(f->m.va + f->m.wptr, data, size);
		f->m.wptr += size;
	} else {
		memcpy(f->m.va + f->m.wptr, data, i);
		f->m.wptr = size - i;
		memcpy(f->m.va, data + i, f->m.wptr);
	}
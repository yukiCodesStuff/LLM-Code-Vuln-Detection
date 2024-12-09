			& 0xffff;

	if (inuse) { /* Tx FIFO is not empty */
		ready = priv->tx_prod - priv->tx_cons - inuse - 1;
	} else {
		/* Check for buffered last packet */
		status = csrrd32(priv->tx_dma_csr, msgdma_csroffs(status));
		if (status & MSGDMA_CSR_STAT_BUSY)
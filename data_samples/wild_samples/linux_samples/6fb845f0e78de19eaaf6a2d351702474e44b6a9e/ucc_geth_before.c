{
	struct ucc_geth_info *ug_info;
	struct ucc_fast_info *uf_info;
	u16 i, j;
	u8 __iomem *bd;

	ug_info = ugeth->ug_info;
	uf_info = &ug_info->uf_info;

	for (i = 0; i < ugeth->ug_info->numQueuesTx; i++) {
		bd = ugeth->p_tx_bd_ring[i];
		if (!bd)
			continue;
		for (j = 0; j < ugeth->ug_info->bdRingLenTx[i]; j++) {
			if (ugeth->tx_skbuff[i][j]) {
				dma_unmap_single(ugeth->dev,
						 in_be32(&((struct qe_bd __iomem *)bd)->buf),
						 (in_be32((u32 __iomem *)bd) &
						  BD_LENGTH_MASK),
						 DMA_TO_DEVICE);
				dev_kfree_skb_any(ugeth->tx_skbuff[i][j]);
				ugeth->tx_skbuff[i][j] = NULL;
			}
		}

		kfree(ugeth->tx_skbuff[i]);

		if (ugeth->p_tx_bd_ring[i]) {
			if (ugeth->ug_info->uf_info.bd_mem_part ==
			    MEM_PART_SYSTEM)
				kfree((void *)ugeth->tx_bd_ring_offset[i]);
			else if (ugeth->ug_info->uf_info.bd_mem_part ==
				 MEM_PART_MURAM)
				qe_muram_free(ugeth->tx_bd_ring_offset[i]);
			ugeth->p_tx_bd_ring[i] = NULL;
		}
	}
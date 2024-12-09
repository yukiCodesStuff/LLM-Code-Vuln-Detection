
	sqp_mad = (struct mlx4_mad_snd_buf *) (sqp->tx_ring[wire_tx_ix].buf.addr);
	if (sqp->tx_ring[wire_tx_ix].ah)
		mlx4_ib_destroy_ah(sqp->tx_ring[wire_tx_ix].ah, 0);
	sqp->tx_ring[wire_tx_ix].ah = ah;
	ib_dma_sync_single_for_cpu(&dev->ib_dev,
				   sqp->tx_ring[wire_tx_ix].buf.map,
				   sizeof (struct mlx4_mad_snd_buf),
		if (wc.status == IB_WC_SUCCESS) {
			switch (wc.opcode) {
			case IB_WC_SEND:
				mlx4_ib_destroy_ah(sqp->tx_ring[wc.wr_id &
					      (MLX4_NUM_TUNNEL_BUFS - 1)].ah, 0);
				sqp->tx_ring[wc.wr_id & (MLX4_NUM_TUNNEL_BUFS - 1)].ah
					= NULL;
				spin_lock(&sqp->tx_lock);
				 " status = %d, wrid = 0x%llx\n",
				 ctx->slave, wc.status, wc.wr_id);
			if (!MLX4_TUN_IS_RECV(wc.wr_id)) {
				mlx4_ib_destroy_ah(sqp->tx_ring[wc.wr_id &
					      (MLX4_NUM_TUNNEL_BUFS - 1)].ah, 0);
				sqp->tx_ring[wc.wr_id & (MLX4_NUM_TUNNEL_BUFS - 1)].ah
					= NULL;
				spin_lock(&sqp->tx_lock);
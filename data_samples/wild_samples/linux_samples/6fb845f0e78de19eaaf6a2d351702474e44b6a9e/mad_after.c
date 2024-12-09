	} else {
		src_qpnum = 1;
		sqp = &sqp_ctx->qp[1];
		wire_pkey_ix = dev->pkeys.virt2phys_pkey[slave][port - 1][pkey_index];
	}

	send_qp = sqp->qp;

	/* create ah */
	ah = mlx4_ib_create_ah_slave(sqp_ctx->pd, attr,
				     rdma_ah_retrieve_grh(attr)->sgid_index,
				     s_mac, vlan_id);
	if (IS_ERR(ah))
		return -ENOMEM;
	spin_lock(&sqp->tx_lock);
	if (sqp->tx_ix_head - sqp->tx_ix_tail >=
	    (MLX4_NUM_TUNNEL_BUFS - 1))
		ret = -EAGAIN;
	else
		wire_tx_ix = (++sqp->tx_ix_head) & (MLX4_NUM_TUNNEL_BUFS - 1);
	spin_unlock(&sqp->tx_lock);
	if (ret)
		goto out;

	sqp_mad = (struct mlx4_mad_snd_buf *) (sqp->tx_ring[wire_tx_ix].buf.addr);
	if (sqp->tx_ring[wire_tx_ix].ah)
		mlx4_ib_destroy_ah(sqp->tx_ring[wire_tx_ix].ah, 0);
	sqp->tx_ring[wire_tx_ix].ah = ah;
	ib_dma_sync_single_for_cpu(&dev->ib_dev,
				   sqp->tx_ring[wire_tx_ix].buf.map,
				   sizeof (struct mlx4_mad_snd_buf),
				   DMA_TO_DEVICE);

	memcpy(&sqp_mad->payload, mad, sizeof *mad);

	ib_dma_sync_single_for_device(&dev->ib_dev,
				      sqp->tx_ring[wire_tx_ix].buf.map,
				      sizeof (struct mlx4_mad_snd_buf),
				      DMA_TO_DEVICE);

	list.addr = sqp->tx_ring[wire_tx_ix].buf.map;
	list.length = sizeof (struct mlx4_mad_snd_buf);
	list.lkey = sqp_ctx->pd->local_dma_lkey;

	wr.ah = ah;
	wr.port_num = port;
	wr.pkey_index = wire_pkey_ix;
	wr.remote_qkey = qkey;
	wr.remote_qpn = remote_qpn;
	wr.wr.next = NULL;
	wr.wr.wr_id = ((u64) wire_tx_ix) | MLX4_TUN_SET_WRID_QPN(src_qpnum);
	wr.wr.sg_list = &list;
	wr.wr.num_sge = 1;
	wr.wr.opcode = IB_WR_SEND;
	wr.wr.send_flags = IB_SEND_SIGNALED;

	ret = ib_post_send(send_qp, &wr.wr, &bad_wr);
	if (!ret)
		return 0;

	spin_lock(&sqp->tx_lock);
	sqp->tx_ix_tail++;
	spin_unlock(&sqp->tx_lock);
	sqp->tx_ring[wire_tx_ix].ah = NULL;
out:
	mlx4_ib_destroy_ah(ah, 0);
	return ret;
}

static int get_slave_base_gid_ix(struct mlx4_ib_dev *dev, int slave, int port)
{
	if (rdma_port_get_link_layer(&dev->ib_dev, port) == IB_LINK_LAYER_INFINIBAND)
		return slave;
	return mlx4_get_base_gid_ix(dev->dev, slave, port);
}

static void fill_in_real_sgid_index(struct mlx4_ib_dev *dev, int slave, int port,
				    struct rdma_ah_attr *ah_attr)
{
	struct ib_global_route *grh = rdma_ah_retrieve_grh(ah_attr);
	if (rdma_port_get_link_layer(&dev->ib_dev, port) == IB_LINK_LAYER_INFINIBAND)
		grh->sgid_index = slave;
	else
		grh->sgid_index += get_slave_base_gid_ix(dev, slave, port);
}

static void mlx4_ib_multiplex_mad(struct mlx4_ib_demux_pv_ctx *ctx, struct ib_wc *wc)
{
	struct mlx4_ib_dev *dev = to_mdev(ctx->ib_dev);
	struct mlx4_ib_demux_pv_qp *tun_qp = &ctx->qp[MLX4_TUN_WRID_QPN(wc->wr_id)];
	int wr_ix = wc->wr_id & (MLX4_NUM_TUNNEL_BUFS - 1);
	struct mlx4_tunnel_mad *tunnel = tun_qp->ring[wr_ix].addr;
	struct mlx4_ib_ah ah;
	struct rdma_ah_attr ah_attr;
	u8 *slave_id;
	int slave;
	int port;
	u16 vlan_id;
	u8 qos;
	u8 *dmac;

	/* Get slave that sent this packet */
	if (wc->src_qp < dev->dev->phys_caps.base_proxy_sqpn ||
	    wc->src_qp >= dev->dev->phys_caps.base_proxy_sqpn + 8 * MLX4_MFUNC_MAX ||
	    (wc->src_qp & 0x1) != ctx->port - 1 ||
	    wc->src_qp & 0x4) {
		mlx4_ib_warn(ctx->ib_dev, "can't multiplex bad sqp:%d\n", wc->src_qp);
		return;
	}
			switch (wc.opcode) {
			case IB_WC_SEND:
				mlx4_ib_destroy_ah(sqp->tx_ring[wc.wr_id &
					      (MLX4_NUM_TUNNEL_BUFS - 1)].ah, 0);
				sqp->tx_ring[wc.wr_id & (MLX4_NUM_TUNNEL_BUFS - 1)].ah
					= NULL;
				spin_lock(&sqp->tx_lock);
				sqp->tx_ix_tail++;
				spin_unlock(&sqp->tx_lock);
				break;
			case IB_WC_RECV:
				mad = (struct ib_mad *) &(((struct mlx4_mad_rcv_buf *)
						(sqp->ring[wc.wr_id &
						(MLX4_NUM_TUNNEL_BUFS - 1)].addr))->payload);
				grh = &(((struct mlx4_mad_rcv_buf *)
						(sqp->ring[wc.wr_id &
						(MLX4_NUM_TUNNEL_BUFS - 1)].addr))->grh);
				mlx4_ib_demux_mad(ctx->ib_dev, ctx->port, &wc, grh, mad);
				if (mlx4_ib_post_pv_qp_buf(ctx, sqp, wc.wr_id &
							   (MLX4_NUM_TUNNEL_BUFS - 1)))
					pr_err("Failed reposting SQP "
					       "buf:%lld\n", wc.wr_id);
				break;
			default:
				break;
			}
		} else  {
			pr_debug("mlx4_ib: completion error in tunnel: %d."
				 " status = %d, wrid = 0x%llx\n",
				 ctx->slave, wc.status, wc.wr_id);
			if (!MLX4_TUN_IS_RECV(wc.wr_id)) {
				mlx4_ib_destroy_ah(sqp->tx_ring[wc.wr_id &
					      (MLX4_NUM_TUNNEL_BUFS - 1)].ah, 0);
				sqp->tx_ring[wc.wr_id & (MLX4_NUM_TUNNEL_BUFS - 1)].ah
					= NULL;
				spin_lock(&sqp->tx_lock);
				sqp->tx_ix_tail++;
				spin_unlock(&sqp->tx_lock);
			}
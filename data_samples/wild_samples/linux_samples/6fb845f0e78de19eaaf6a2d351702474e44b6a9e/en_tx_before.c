	if (ihs) {
		ihs += !!skb_vlan_tag_present(skb) * VLAN_HLEN;

		ds_cnt_inl = DIV_ROUND_UP(ihs - INL_HDR_START_SZ, MLX5_SEND_WQE_DS);
		ds_cnt += ds_cnt_inl;
	}

	num_wqebbs = DIV_ROUND_UP(ds_cnt, MLX5_SEND_WQEBB_NUM_DS);
	contig_wqebbs_room = mlx5_wq_cyc_get_contig_wqebbs(wq, pi);
	if (unlikely(contig_wqebbs_room < num_wqebbs)) {
		mlx5e_fill_sq_frag_edge(sq, wq, pi, contig_wqebbs_room);
		mlx5e_sq_fetch_wqe(sq, &wqe, &pi);
	}
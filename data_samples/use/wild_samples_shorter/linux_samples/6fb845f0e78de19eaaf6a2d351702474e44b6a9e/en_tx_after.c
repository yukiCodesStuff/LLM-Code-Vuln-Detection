	num_wqebbs = DIV_ROUND_UP(ds_cnt, MLX5_SEND_WQEBB_NUM_DS);
	contig_wqebbs_room = mlx5_wq_cyc_get_contig_wqebbs(wq, pi);
	if (unlikely(contig_wqebbs_room < num_wqebbs)) {
#ifdef CONFIG_MLX5_EN_IPSEC
		struct mlx5_wqe_eth_seg cur_eth = wqe->eth;
#endif
		mlx5e_fill_sq_frag_edge(sq, wq, pi, contig_wqebbs_room);
		mlx5e_sq_fetch_wqe(sq, &wqe, &pi);
#ifdef CONFIG_MLX5_EN_IPSEC
		wqe->eth = cur_eth;
#endif
	}

	/* fill wqe */
	wi   = &sq->db.wqe_info[pi];
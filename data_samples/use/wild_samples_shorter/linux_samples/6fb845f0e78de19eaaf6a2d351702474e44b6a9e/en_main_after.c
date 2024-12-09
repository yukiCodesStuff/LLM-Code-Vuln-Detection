	if (params->rx_dim_enabled)
		__set_bit(MLX5E_RQ_STATE_AM, &c->rq.state);

	if (MLX5E_GET_PFLAG(params, MLX5E_PFLAG_RX_NO_CSUM_COMPLETE))
		__set_bit(MLX5E_RQ_STATE_NO_CSUM_COMPLETE, &c->rq.state);

	return 0;

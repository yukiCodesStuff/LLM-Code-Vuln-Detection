{
	struct hal_wbm_release_ring *wbm_desc = desc;
	enum hal_wbm_rel_desc_type type;
	enum hal_wbm_rel_src_module rel_src;
	enum hal_rx_buf_return_buf_manager ret_buf_mgr;

	type = FIELD_GET(HAL_WBM_RELEASE_INFO0_DESC_TYPE,
			 wbm_desc->info0);
	/* We expect only WBM_REL buffer type */
	if (type != HAL_WBM_REL_DESC_TYPE_REL_MSDU) {
		WARN_ON(1);
		return -EINVAL;
	}
	if (type != HAL_WBM_REL_DESC_TYPE_REL_MSDU) {
		WARN_ON(1);
		return -EINVAL;
	}

	rel_src = FIELD_GET(HAL_WBM_RELEASE_INFO0_REL_SRC_MODULE,
			    wbm_desc->info0);
	if (rel_src != HAL_WBM_REL_SRC_MODULE_RXDMA &&
	    rel_src != HAL_WBM_REL_SRC_MODULE_REO)
		return -EINVAL;

	ret_buf_mgr = FIELD_GET(BUFFER_ADDR_INFO1_RET_BUF_MGR,
				wbm_desc->buf_addr_info.info1);
	if (ret_buf_mgr != ab->hw_params.hal_params->rx_buf_rbm) {
		ab->soc_stats.invalid_rbm++;
		return -EINVAL;
	}
		.hal_desc_sz = sizeof(struct hal_rx_desc_ipq8074),
		.fix_l1ss = true,
		.max_tx_ring = DP_TCL_NUM_RING_MAX,
	},
	{
		.hw_rev = ATH11K_HW_IPQ6018_HW10,
		.name = "ipq6018 hw1.0",
		.hal_desc_sz = sizeof(struct hal_rx_desc_ipq8074),
		.fix_l1ss = true,
		.max_tx_ring = DP_TCL_NUM_RING_MAX,
	},
	{
		.name = "qca6390 hw2.0",
		.hw_rev = ATH11K_HW_QCA6390_HW20,
		.hal_desc_sz = sizeof(struct hal_rx_desc_ipq8074),
		.fix_l1ss = true,
		.max_tx_ring = DP_TCL_NUM_RING_MAX_QCA6390,
	},
	{
		.name = "qcn9074 hw1.0",
		.hw_rev = ATH11K_HW_QCN9074_HW10,
		.hal_desc_sz = sizeof(struct hal_rx_desc_qcn9074),
		.fix_l1ss = true,
		.max_tx_ring = DP_TCL_NUM_RING_MAX,
	},
	{
		.name = "wcn6855 hw2.0",
		.hw_rev = ATH11K_HW_WCN6855_HW20,
		.hal_desc_sz = sizeof(struct hal_rx_desc_wcn6855),
		.fix_l1ss = false,
		.max_tx_ring = DP_TCL_NUM_RING_MAX_QCA6390,
	},
};

int ath11k_core_suspend(struct ath11k_base *ab)
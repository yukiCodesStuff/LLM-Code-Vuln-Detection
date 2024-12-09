			   int budget)
{
	struct napi_struct *napi = &irq_grp->napi;
	const struct ath11k_hw_hal_params *hal_params;
	int grp_id = irq_grp->grp_id;
	int work_done = 0;
	int i = 0, j;
	int tot_work_done = 0;
				struct ath11k_pdev_dp *dp = &ar->dp;
				struct dp_rxdma_ring *rx_ring = &dp->rx_refill_buf_ring;

				hal_params = ab->hw_params.hal_params;
				ath11k_dp_rxbufs_replenish(ab, id, rx_ring, 0,
							   hal_params->rx_buf_rbm);
			}
		}
	}
	/* TODO: Implement handler for other interrupts */
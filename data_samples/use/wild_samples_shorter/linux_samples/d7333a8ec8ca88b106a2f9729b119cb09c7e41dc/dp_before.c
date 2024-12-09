			   int budget)
{
	struct napi_struct *napi = &irq_grp->napi;
	int grp_id = irq_grp->grp_id;
	int work_done = 0;
	int i = 0, j;
	int tot_work_done = 0;
				struct ath11k_pdev_dp *dp = &ar->dp;
				struct dp_rxdma_ring *rx_ring = &dp->rx_refill_buf_ring;

				ath11k_dp_rxbufs_replenish(ab, id, rx_ring, 0,
							   HAL_RX_BUF_RBM_SW3_BM);
			}
		}
	}
	/* TODO: Implement handler for other interrupts */
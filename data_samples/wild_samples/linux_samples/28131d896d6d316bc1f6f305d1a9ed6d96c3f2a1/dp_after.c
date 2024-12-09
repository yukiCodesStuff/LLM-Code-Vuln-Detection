{
	struct napi_struct *napi = &irq_grp->napi;
	const struct ath11k_hw_hal_params *hal_params;
	int grp_id = irq_grp->grp_id;
	int work_done = 0;
	int i = 0, j;
	int tot_work_done = 0;

	while (ab->hw_params.ring_mask->tx[grp_id] >> i) {
		if (ab->hw_params.ring_mask->tx[grp_id] & BIT(i))
			ath11k_dp_tx_completion_handler(ab, i);
		i++;
	}

	if (ab->hw_params.ring_mask->rx_err[grp_id]) {
		work_done = ath11k_dp_process_rx_err(ab, napi, budget);
		budget -= work_done;
		tot_work_done += work_done;
		if (budget <= 0)
			goto done;
	}

	if (ab->hw_params.ring_mask->rx_wbm_rel[grp_id]) {
		work_done = ath11k_dp_rx_process_wbm_err(ab,
							 napi,
							 budget);
		budget -= work_done;
		tot_work_done += work_done;

		if (budget <= 0)
			goto done;
	}

	if (ab->hw_params.ring_mask->rx[grp_id]) {
		i =  fls(ab->hw_params.ring_mask->rx[grp_id]) - 1;
		work_done = ath11k_dp_process_rx(ab, i, napi,
						 budget);
		budget -= work_done;
		tot_work_done += work_done;
		if (budget <= 0)
			goto done;
	}

	if (ab->hw_params.ring_mask->rx_mon_status[grp_id]) {
		for (i = 0; i < ab->num_radios; i++) {
			for (j = 0; j < ab->hw_params.num_rxmda_per_pdev; j++) {
				int id = i * ab->hw_params.num_rxmda_per_pdev + j;

				if (ab->hw_params.ring_mask->rx_mon_status[grp_id] &
					BIT(id)) {
					work_done =
					ath11k_dp_rx_process_mon_rings(ab,
								       id,
								       napi, budget);
					budget -= work_done;
					tot_work_done += work_done;

					if (budget <= 0)
						goto done;
				}
			}
		}
	}

	if (ab->hw_params.ring_mask->reo_status[grp_id])
		ath11k_dp_process_reo_status(ab);

	for (i = 0; i < ab->num_radios; i++) {
		for (j = 0; j < ab->hw_params.num_rxmda_per_pdev; j++) {
			int id = i * ab->hw_params.num_rxmda_per_pdev + j;

			if (ab->hw_params.ring_mask->rxdma2host[grp_id] & BIT(id)) {
				work_done = ath11k_dp_process_rxdma_err(ab, id, budget);
				budget -= work_done;
				tot_work_done += work_done;
			}

			if (budget <= 0)
				goto done;

			if (ab->hw_params.ring_mask->host2rxdma[grp_id] & BIT(id)) {
				struct ath11k *ar = ath11k_ab_to_ar(ab, id);
				struct ath11k_pdev_dp *dp = &ar->dp;
				struct dp_rxdma_ring *rx_ring = &dp->rx_refill_buf_ring;

				hal_params = ab->hw_params.hal_params;
				ath11k_dp_rxbufs_replenish(ab, id, rx_ring, 0,
							   hal_params->rx_buf_rbm);
			}
		}
	}
	/* TODO: Implement handler for other interrupts */

done:
	return tot_work_done;
}
			if (ab->hw_params.ring_mask->host2rxdma[grp_id] & BIT(id)) {
				struct ath11k *ar = ath11k_ab_to_ar(ab, id);
				struct ath11k_pdev_dp *dp = &ar->dp;
				struct dp_rxdma_ring *rx_ring = &dp->rx_refill_buf_ring;

				hal_params = ab->hw_params.hal_params;
				ath11k_dp_rxbufs_replenish(ab, id, rx_ring, 0,
							   hal_params->rx_buf_rbm);
			}
		}
	}
	/* TODO: Implement handler for other interrupts */

done:
	return tot_work_done;
}
EXPORT_SYMBOL(ath11k_dp_service_srng);

void ath11k_dp_pdev_free(struct ath11k_base *ab)
{
	struct ath11k *ar;
	int i;

	del_timer_sync(&ab->mon_reap_timer);

	for (i = 0; i < ab->num_radios; i++) {
		ar = ab->pdevs[i].ar;
		ath11k_dp_rx_pdev_free(ab, i);
		ath11k_debugfs_unregister(ar);
		ath11k_dp_rx_pdev_mon_detach(ar);
	}
}

void ath11k_dp_pdev_pre_alloc(struct ath11k_base *ab)
{
	struct ath11k *ar;
	struct ath11k_pdev_dp *dp;
	int i;
	int j;

	for (i = 0; i <  ab->num_radios; i++) {
		ar = ab->pdevs[i].ar;
		dp = &ar->dp;
		dp->mac_id = i;
		idr_init(&dp->rx_refill_buf_ring.bufs_idr);
		spin_lock_init(&dp->rx_refill_buf_ring.idr_lock);
		atomic_set(&dp->num_tx_pending, 0);
		init_waitqueue_head(&dp->tx_empty_waitq);
		for (j = 0; j < ab->hw_params.num_rxmda_per_pdev; j++) {
			idr_init(&dp->rx_mon_status_refill_ring[j].bufs_idr);
			spin_lock_init(&dp->rx_mon_status_refill_ring[j].idr_lock);
		}
		idr_init(&dp->rxdma_mon_buf_ring.bufs_idr);
		spin_lock_init(&dp->rxdma_mon_buf_ring.idr_lock);
	}
}

int ath11k_dp_pdev_alloc(struct ath11k_base *ab)
{
	struct ath11k *ar;
	int ret;
	int i;

	/* TODO:Per-pdev rx ring unlike tx ring which is mapped to different AC's */
	for (i = 0; i < ab->num_radios; i++) {
		ar = ab->pdevs[i].ar;
		ret = ath11k_dp_rx_pdev_alloc(ab, i);
		if (ret) {
			ath11k_warn(ab, "failed to allocate pdev rx for pdev_id :%d\n",
				    i);
			goto err;
		}
		ret = ath11k_dp_rx_pdev_mon_attach(ar);
		if (ret) {
			ath11k_warn(ab, "failed to initialize mon pdev %d\n",
				    i);
			goto err;
		}
	}

	return 0;

err:
	ath11k_dp_pdev_free(ab);

	return ret;
}

int ath11k_dp_htt_connect(struct ath11k_dp *dp)
{
	struct ath11k_htc_svc_conn_req conn_req;
	struct ath11k_htc_svc_conn_resp conn_resp;
	int status;

	memset(&conn_req, 0, sizeof(conn_req));
	memset(&conn_resp, 0, sizeof(conn_resp));

	conn_req.ep_ops.ep_tx_complete = ath11k_dp_htt_htc_tx_complete;
	conn_req.ep_ops.ep_rx_complete = ath11k_dp_htt_htc_t2h_msg_handler;

	/* connect to control service */
	conn_req.service_id = ATH11K_HTC_SVC_ID_HTT_DATA_MSG;

	status = ath11k_htc_connect_service(&dp->ab->htc, &conn_req,
					    &conn_resp);

	if (status)
		return status;

	dp->eid = conn_resp.eid;

	return 0;
}

static void ath11k_dp_update_vdev_search(struct ath11k_vif *arvif)
{
	 /* When v2_map_support is true:for STA mode, enable address
	  * search index, tcl uses ast_hash value in the descriptor.
	  * When v2_map_support is false: for STA mode, dont' enable
	  * address search index.
	  */
	switch (arvif->vdev_type) {
	case WMI_VDEV_TYPE_STA:
		if (arvif->ar->ab->hw_params.htt_peer_map_v2) {
			arvif->hal_addr_search_flags = HAL_TX_ADDRX_EN;
			arvif->search_type = HAL_TX_ADDR_SEARCH_INDEX;
		} else {
			arvif->hal_addr_search_flags = HAL_TX_ADDRY_EN;
			arvif->search_type = HAL_TX_ADDR_SEARCH_DEFAULT;
		}
		break;
	case WMI_VDEV_TYPE_AP:
	case WMI_VDEV_TYPE_IBSS:
		arvif->hal_addr_search_flags = HAL_TX_ADDRX_EN;
		arvif->search_type = HAL_TX_ADDR_SEARCH_DEFAULT;
		break;
	case WMI_VDEV_TYPE_MONITOR:
	default:
		return;
	}
}

void ath11k_dp_vdev_tx_attach(struct ath11k *ar, struct ath11k_vif *arvif)
{
	arvif->tcl_metadata |= FIELD_PREP(HTT_TCL_META_DATA_TYPE, 1) |
			       FIELD_PREP(HTT_TCL_META_DATA_VDEV_ID,
					  arvif->vdev_id) |
			       FIELD_PREP(HTT_TCL_META_DATA_PDEV_ID,
					  ar->pdev->pdev_id);

	/* set HTT extension valid bit to 0 by default */
	arvif->tcl_metadata &= ~HTT_TCL_META_DATA_VALID_HTT;

	ath11k_dp_update_vdev_search(arvif);
}

static int ath11k_dp_tx_pending_cleanup(int buf_id, void *skb, void *ctx)
{
	struct ath11k_base *ab = (struct ath11k_base *)ctx;
	struct sk_buff *msdu = skb;

	dma_unmap_single(ab->dev, ATH11K_SKB_CB(msdu)->paddr, msdu->len,
			 DMA_TO_DEVICE);

	dev_kfree_skb_any(msdu);

	return 0;
}

void ath11k_dp_free(struct ath11k_base *ab)
{
	struct ath11k_dp *dp = &ab->dp;
	int i;

	ath11k_dp_link_desc_cleanup(ab, dp->link_desc_banks,
				    HAL_WBM_IDLE_LINK, &dp->wbm_idle_ring);

	ath11k_dp_srng_common_cleanup(ab);

	ath11k_dp_reo_cmd_list_cleanup(ab);

	for (i = 0; i < ab->hw_params.max_tx_ring; i++) {
		spin_lock_bh(&dp->tx_ring[i].tx_idr_lock);
		idr_for_each(&dp->tx_ring[i].txbuf_idr,
			     ath11k_dp_tx_pending_cleanup, ab);
		idr_destroy(&dp->tx_ring[i].txbuf_idr);
		spin_unlock_bh(&dp->tx_ring[i].tx_idr_lock);
		kfree(dp->tx_ring[i].tx_status);
	}

	/* Deinit any SOC level resource */
}

int ath11k_dp_alloc(struct ath11k_base *ab)
{
	struct ath11k_dp *dp = &ab->dp;
	struct hal_srng *srng = NULL;
	size_t size = 0;
	u32 n_link_desc = 0;
	int ret;
	int i;

	dp->ab = ab;

	INIT_LIST_HEAD(&dp->reo_cmd_list);
	INIT_LIST_HEAD(&dp->reo_cmd_cache_flush_list);
	spin_lock_init(&dp->reo_cmd_lock);

	dp->reo_cmd_cache_flush_count = 0;

	ret = ath11k_wbm_idle_ring_setup(ab, &n_link_desc);
	if (ret) {
		ath11k_warn(ab, "failed to setup wbm_idle_ring: %d\n", ret);
		return ret;
	}

	srng = &ab->hal.srng_list[dp->wbm_idle_ring.ring_id];

	ret = ath11k_dp_link_desc_setup(ab, dp->link_desc_banks,
					HAL_WBM_IDLE_LINK, srng, n_link_desc);
	if (ret) {
		ath11k_warn(ab, "failed to setup link desc: %d\n", ret);
		return ret;
	}

	ret = ath11k_dp_srng_common_setup(ab);
	if (ret)
		goto fail_link_desc_cleanup;

	size = sizeof(struct hal_wbm_release_ring) * DP_TX_COMP_RING_SIZE;

	for (i = 0; i < ab->hw_params.max_tx_ring; i++) {
		idr_init(&dp->tx_ring[i].txbuf_idr);
		spin_lock_init(&dp->tx_ring[i].tx_idr_lock);
		dp->tx_ring[i].tcl_data_ring_id = i;

		dp->tx_ring[i].tx_status_head = 0;
		dp->tx_ring[i].tx_status_tail = DP_TX_COMP_RING_SIZE - 1;
		dp->tx_ring[i].tx_status = kmalloc(size, GFP_KERNEL);
		if (!dp->tx_ring[i].tx_status) {
			ret = -ENOMEM;
			goto fail_cmn_srng_cleanup;
		}
	}

	for (i = 0; i < HAL_DSCP_TID_MAP_TBL_NUM_ENTRIES_MAX; i++)
		ath11k_hal_tx_set_dscp_tid_map(ab, i);

	/* Init any SOC level resource for DP */

	return 0;

fail_cmn_srng_cleanup:
	ath11k_dp_srng_common_cleanup(ab);

fail_link_desc_cleanup:
	ath11k_dp_link_desc_cleanup(ab, dp->link_desc_banks,
				    HAL_WBM_IDLE_LINK, &dp->wbm_idle_ring);

	return ret;
}

static void ath11k_dp_shadow_timer_handler(struct timer_list *t)
{
	struct ath11k_hp_update_timer *update_timer = from_timer(update_timer,
								 t, timer);
	struct ath11k_base *ab = update_timer->ab;
	struct hal_srng	*srng = &ab->hal.srng_list[update_timer->ring_id];

	spin_lock_bh(&srng->lock);

	/* when the timer is fired, the handler checks whether there
	 * are new TX happened. The handler updates HP only when there
	 * are no TX operations during the timeout interval, and stop
	 * the timer. Timer will be started again when TX happens again.
	 */
	if (update_timer->timer_tx_num != update_timer->tx_num) {
		update_timer->timer_tx_num = update_timer->tx_num;
		mod_timer(&update_timer->timer, jiffies +
		  msecs_to_jiffies(update_timer->interval));
	} else {
		update_timer->started = false;
		ath11k_hal_srng_shadow_update_hp_tp(ab, srng);
	}

	spin_unlock_bh(&srng->lock);
}

void ath11k_dp_shadow_start_timer(struct ath11k_base *ab,
				  struct hal_srng *srng,
				  struct ath11k_hp_update_timer *update_timer)
{
	lockdep_assert_held(&srng->lock);

	if (!ab->hw_params.supports_shadow_regs)
		return;

	update_timer->tx_num++;

	if (update_timer->started)
		return;

	update_timer->started = true;
	update_timer->timer_tx_num = update_timer->tx_num;
	mod_timer(&update_timer->timer, jiffies +
		  msecs_to_jiffies(update_timer->interval));
}

void ath11k_dp_shadow_stop_timer(struct ath11k_base *ab,
				 struct ath11k_hp_update_timer *update_timer)
{
	if (!ab->hw_params.supports_shadow_regs)
		return;

	if (!update_timer->init)
		return;

	del_timer_sync(&update_timer->timer);
}

void ath11k_dp_shadow_init_timer(struct ath11k_base *ab,
				 struct ath11k_hp_update_timer *update_timer,
				 u32 interval, u32 ring_id)
{
	if (!ab->hw_params.supports_shadow_regs)
		return;

	update_timer->tx_num = 0;
	update_timer->timer_tx_num = 0;
	update_timer->ab = ab;
	update_timer->ring_id = ring_id;
	update_timer->interval = interval;
	update_timer->init = true;
	timer_setup(&update_timer->timer,
		    ath11k_dp_shadow_timer_handler, 0);
}
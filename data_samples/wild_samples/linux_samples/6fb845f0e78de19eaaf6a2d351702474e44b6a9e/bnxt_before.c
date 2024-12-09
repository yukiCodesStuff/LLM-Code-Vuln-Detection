	for (i = 0; i < bp->cp_nr_rings; i++) {
		struct bnxt_napi *bnapi = bp->bnapi[i];
		struct bnxt_cp_ring_info *cpr = &bnapi->cp_ring;
		struct bnxt_ring_struct *ring = &cpr->cp_ring_struct;
		u32 map_idx = ring->map_idx;

		rc = hwrm_ring_alloc_send_msg(bp, ring, type, map_idx);
		if (rc)
			goto err_out;
		bnxt_set_db(bp, &cpr->cp_db, type, map_idx, ring->fw_ring_id);
		bnxt_db_nq(bp, &cpr->cp_db, cpr->cp_raw_cons);
		bp->grp_info[i].cp_fw_ring_id = ring->fw_ring_id;

		if (!i) {
			rc = bnxt_hwrm_set_async_event_cr(bp, ring->fw_ring_id);
			if (rc)
				netdev_warn(bp->dev, "Failed to set async event completion ring.\n");
		}
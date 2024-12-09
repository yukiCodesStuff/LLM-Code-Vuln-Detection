		struct bnxt_cp_ring_info *cpr = &bnapi->cp_ring;
		struct bnxt_ring_struct *ring = &cpr->cp_ring_struct;
		u32 map_idx = ring->map_idx;
		unsigned int vector;

		vector = bp->irq_tbl[map_idx].vector;
		disable_irq_nosync(vector);
		rc = hwrm_ring_alloc_send_msg(bp, ring, type, map_idx);
		if (rc) {
			enable_irq(vector);
			goto err_out;
		}
		bnxt_set_db(bp, &cpr->cp_db, type, map_idx, ring->fw_ring_id);
		bnxt_db_nq(bp, &cpr->cp_db, cpr->cp_raw_cons);
		enable_irq(vector);
		bp->grp_info[i].cp_fw_ring_id = ring->fw_ring_id;

		if (!i) {
			rc = bnxt_hwrm_set_async_event_cr(bp, ring->fw_ring_id);
{
	__le64 *wqe;
	struct irdma_sc_cqp *cqp;
	u64 hdr;
	enum irdma_page_size page_size;

	if (info->page_size == 0x40000000)
		page_size = IRDMA_PAGE_SIZE_1G;
	else if (info->page_size == 0x200000)
		page_size = IRDMA_PAGE_SIZE_2M;
	else
		page_size = IRDMA_PAGE_SIZE_4K;

	cqp = dev->cqp;
	wqe = irdma_sc_cqp_get_next_send_wqe(cqp, scratch);
	if (!wqe)
		return -ENOMEM;

	set_64bit_val(wqe, 8,
		      FLD_LS_64(dev, info->pd_id, IRDMA_CQPSQ_STAG_PDID) |
		      FIELD_PREP(IRDMA_CQPSQ_STAG_STAGLEN, info->total_len));
	set_64bit_val(wqe, 16,
		      FIELD_PREP(IRDMA_CQPSQ_STAG_IDX, info->stag_idx));
	set_64bit_val(wqe, 40,
		      FIELD_PREP(IRDMA_CQPSQ_STAG_HMCFNIDX, info->hmc_fcn_index));

	if (info->chunk_size)
		set_64bit_val(wqe, 48,
			      FIELD_PREP(IRDMA_CQPSQ_STAG_FIRSTPMPBLIDX, info->first_pm_pbl_idx));

	hdr = FIELD_PREP(IRDMA_CQPSQ_OPCODE, IRDMA_CQP_OP_ALLOC_STAG) |
	      FIELD_PREP(IRDMA_CQPSQ_STAG_MR, 1) |
	      FIELD_PREP(IRDMA_CQPSQ_STAG_ARIGHTS, info->access_rights) |
	      FIELD_PREP(IRDMA_CQPSQ_STAG_LPBLSIZE, info->chunk_size) |
	      FIELD_PREP(IRDMA_CQPSQ_STAG_HPAGESIZE, page_size) |
	      FIELD_PREP(IRDMA_CQPSQ_STAG_REMACCENABLED, info->remote_access) |
	      FIELD_PREP(IRDMA_CQPSQ_STAG_USEHMCFNIDX, info->use_hmc_fcn_index) |
	      FIELD_PREP(IRDMA_CQPSQ_STAG_USEPFRID, info->use_pf_rid) |
	      FIELD_PREP(IRDMA_CQPSQ_WQEVALID, cqp->polarity);
	dma_wmb(); /* make sure WQE is written before valid bit is set */

	set_64bit_val(wqe, 24, hdr);

	print_hex_dump_debug("WQE: ALLOC_STAG WQE", DUMP_PREFIX_OFFSET, 16, 8,
			     wqe, IRDMA_CQP_WQE_SIZE * 8, false);
	if (post_sq)
		irdma_sc_cqp_post_sq(cqp);

	return 0;
}

/**
 * irdma_sc_mr_reg_non_shared - non-shared mr registration
 * @dev: sc device struct
 * @info: mr info
 * @scratch: u64 saved to be used during cqp completion
 * @post_sq: flag for cqp db to ring
 */
static int irdma_sc_mr_reg_non_shared(struct irdma_sc_dev *dev,
				      struct irdma_reg_ns_stag_info *info,
				      u64 scratch, bool post_sq)
{
	__le64 *wqe;
	u64 fbo;
	struct irdma_sc_cqp *cqp;
	u64 hdr;
	u32 pble_obj_cnt;
	bool remote_access;
	u8 addr_type;
	enum irdma_page_size page_size;

	if (info->page_size == 0x40000000)
		page_size = IRDMA_PAGE_SIZE_1G;
	else if (info->page_size == 0x200000)
		page_size = IRDMA_PAGE_SIZE_2M;
	else if (info->page_size == 0x1000)
		page_size = IRDMA_PAGE_SIZE_4K;
	else
		return -EINVAL;

	if (info->access_rights & (IRDMA_ACCESS_FLAGS_REMOTEREAD_ONLY |
				   IRDMA_ACCESS_FLAGS_REMOTEWRITE_ONLY))
		remote_access = true;
	else
		remote_access = false;

	pble_obj_cnt = dev->hmc_info->hmc_obj[IRDMA_HMC_IW_PBLE].cnt;
	if (info->chunk_size && info->first_pm_pbl_index >= pble_obj_cnt)
		return -EINVAL;

	cqp = dev->cqp;
	wqe = irdma_sc_cqp_get_next_send_wqe(cqp, scratch);
	if (!wqe)
		return -ENOMEM;
	fbo = info->va & (info->page_size - 1);

	set_64bit_val(wqe, 0,
		      (info->addr_type == IRDMA_ADDR_TYPE_VA_BASED ?
		      info->va : fbo));
	set_64bit_val(wqe, 8,
		      FIELD_PREP(IRDMA_CQPSQ_STAG_STAGLEN, info->total_len) |
		      FLD_LS_64(dev, info->pd_id, IRDMA_CQPSQ_STAG_PDID));
	set_64bit_val(wqe, 16,
		      FIELD_PREP(IRDMA_CQPSQ_STAG_KEY, info->stag_key) |
		      FIELD_PREP(IRDMA_CQPSQ_STAG_IDX, info->stag_idx));
	if (!info->chunk_size) {
		set_64bit_val(wqe, 32, info->reg_addr_pa);
		set_64bit_val(wqe, 48, 0);
	} else {
		set_64bit_val(wqe, 32, 0);
		set_64bit_val(wqe, 48,
			      FIELD_PREP(IRDMA_CQPSQ_STAG_FIRSTPMPBLIDX, info->first_pm_pbl_index));
	}
	set_64bit_val(wqe, 40, info->hmc_fcn_index);
	set_64bit_val(wqe, 56, 0);

	addr_type = (info->addr_type == IRDMA_ADDR_TYPE_VA_BASED) ? 1 : 0;
	hdr = FIELD_PREP(IRDMA_CQPSQ_OPCODE, IRDMA_CQP_OP_REG_MR) |
	      FIELD_PREP(IRDMA_CQPSQ_STAG_MR, 1) |
	      FIELD_PREP(IRDMA_CQPSQ_STAG_LPBLSIZE, info->chunk_size) |
	      FIELD_PREP(IRDMA_CQPSQ_STAG_HPAGESIZE, page_size) |
	      FIELD_PREP(IRDMA_CQPSQ_STAG_ARIGHTS, info->access_rights) |
	      FIELD_PREP(IRDMA_CQPSQ_STAG_REMACCENABLED, remote_access) |
	      FIELD_PREP(IRDMA_CQPSQ_STAG_VABASEDTO, addr_type) |
	      FIELD_PREP(IRDMA_CQPSQ_STAG_USEHMCFNIDX, info->use_hmc_fcn_index) |
	      FIELD_PREP(IRDMA_CQPSQ_STAG_USEPFRID, info->use_pf_rid) |
	      FIELD_PREP(IRDMA_CQPSQ_WQEVALID, cqp->polarity);
	dma_wmb(); /* make sure WQE is written before valid bit is set */

	set_64bit_val(wqe, 24, hdr);

	print_hex_dump_debug("WQE: MR_REG_NS WQE", DUMP_PREFIX_OFFSET, 16, 8,
			     wqe, IRDMA_CQP_WQE_SIZE * 8, false);
	if (post_sq)
		irdma_sc_cqp_post_sq(cqp);

	return 0;
}
{
	__le64 *wqe;
	u64 fbo;
	struct irdma_sc_cqp *cqp;
	u64 hdr;
	u32 pble_obj_cnt;
	bool remote_access;
	u8 addr_type;
	enum irdma_page_size page_size;

	if (info->page_size == 0x40000000)
		page_size = IRDMA_PAGE_SIZE_1G;
	else if (info->page_size == 0x200000)
		page_size = IRDMA_PAGE_SIZE_2M;
	else if (info->page_size == 0x1000)
		page_size = IRDMA_PAGE_SIZE_4K;
	else
		return -EINVAL;

	if (info->access_rights & (IRDMA_ACCESS_FLAGS_REMOTEREAD_ONLY |
				   IRDMA_ACCESS_FLAGS_REMOTEWRITE_ONLY))
		remote_access = true;
	else
		remote_access = false;

	pble_obj_cnt = dev->hmc_info->hmc_obj[IRDMA_HMC_IW_PBLE].cnt;
	if (info->chunk_size && info->first_pm_pbl_index >= pble_obj_cnt)
		return -EINVAL;

	cqp = dev->cqp;
	wqe = irdma_sc_cqp_get_next_send_wqe(cqp, scratch);
	if (!wqe)
		return -ENOMEM;
	fbo = info->va & (info->page_size - 1);

	set_64bit_val(wqe, 0,
		      (info->addr_type == IRDMA_ADDR_TYPE_VA_BASED ?
		      info->va : fbo));
	set_64bit_val(wqe, 8,
		      FIELD_PREP(IRDMA_CQPSQ_STAG_STAGLEN, info->total_len) |
		      FLD_LS_64(dev, info->pd_id, IRDMA_CQPSQ_STAG_PDID));
	set_64bit_val(wqe, 16,
		      FIELD_PREP(IRDMA_CQPSQ_STAG_KEY, info->stag_key) |
		      FIELD_PREP(IRDMA_CQPSQ_STAG_IDX, info->stag_idx));
	if (!info->chunk_size) {
		set_64bit_val(wqe, 32, info->reg_addr_pa);
		set_64bit_val(wqe, 48, 0);
	} else {
		set_64bit_val(wqe, 32, 0);
		set_64bit_val(wqe, 48,
			      FIELD_PREP(IRDMA_CQPSQ_STAG_FIRSTPMPBLIDX, info->first_pm_pbl_index));
	}
	set_64bit_val(wqe, 40, info->hmc_fcn_index);
	set_64bit_val(wqe, 56, 0);

	addr_type = (info->addr_type == IRDMA_ADDR_TYPE_VA_BASED) ? 1 : 0;
	hdr = FIELD_PREP(IRDMA_CQPSQ_OPCODE, IRDMA_CQP_OP_REG_MR) |
	      FIELD_PREP(IRDMA_CQPSQ_STAG_MR, 1) |
	      FIELD_PREP(IRDMA_CQPSQ_STAG_LPBLSIZE, info->chunk_size) |
	      FIELD_PREP(IRDMA_CQPSQ_STAG_HPAGESIZE, page_size) |
	      FIELD_PREP(IRDMA_CQPSQ_STAG_ARIGHTS, info->access_rights) |
	      FIELD_PREP(IRDMA_CQPSQ_STAG_REMACCENABLED, remote_access) |
	      FIELD_PREP(IRDMA_CQPSQ_STAG_VABASEDTO, addr_type) |
	      FIELD_PREP(IRDMA_CQPSQ_STAG_USEHMCFNIDX, info->use_hmc_fcn_index) |
	      FIELD_PREP(IRDMA_CQPSQ_STAG_USEPFRID, info->use_pf_rid) |
	      FIELD_PREP(IRDMA_CQPSQ_WQEVALID, cqp->polarity);
	dma_wmb(); /* make sure WQE is written before valid bit is set */

	set_64bit_val(wqe, 24, hdr);

	print_hex_dump_debug("WQE: MR_REG_NS WQE", DUMP_PREFIX_OFFSET, 16, 8,
			     wqe, IRDMA_CQP_WQE_SIZE * 8, false);
	if (post_sq)
		irdma_sc_cqp_post_sq(cqp);

	return 0;
}
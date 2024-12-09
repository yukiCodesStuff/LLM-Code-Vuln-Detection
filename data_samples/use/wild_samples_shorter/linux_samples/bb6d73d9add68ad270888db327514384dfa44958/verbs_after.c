			       struct irdma_mr *iwmr)
{
	struct irdma_allocate_stag_info *info;
	struct ib_pd *pd = iwmr->ibmr.pd;
	struct irdma_pd *iwpd = to_iwpd(pd);
	int status;
	struct irdma_cqp_request *cqp_request;
	struct cqp_cmds_info *cqp_info;

	info->stag_idx = iwmr->stag >> IRDMA_CQPSQ_STAG_IDX_S;
	info->pd_id = iwpd->sc_pd.pd_id;
	info->total_len = iwmr->len;
	info->all_memory = pd->flags & IB_PD_UNSAFE_GLOBAL_RKEY;
	info->remote_access = true;
	cqp_info->cqp_cmd = IRDMA_OP_ALLOC_STAG;
	cqp_info->post_sq = 1;
	cqp_info->in.u.alloc_stag.dev = &iwdev->rf->sc_dev;
	iwmr->type = IRDMA_MEMREG_TYPE_MEM;
	palloc = &iwpbl->pble_alloc;
	iwmr->page_cnt = max_num_sg;
	/* Use system PAGE_SIZE as the sg page sizes are unknown at this point */
	iwmr->len = max_num_sg * PAGE_SIZE;
	err_code = irdma_get_pble(iwdev->rf->pble_rsrc, palloc, iwmr->page_cnt,
				  false);
	if (err_code)
		goto err_get_pble;
{
	struct irdma_pbl *iwpbl = &iwmr->iwpbl;
	struct irdma_reg_ns_stag_info *stag_info;
	struct ib_pd *pd = iwmr->ibmr.pd;
	struct irdma_pd *iwpd = to_iwpd(pd);
	struct irdma_pble_alloc *palloc = &iwpbl->pble_alloc;
	struct irdma_cqp_request *cqp_request;
	struct cqp_cmds_info *cqp_info;
	int ret;
	stag_info->total_len = iwmr->len;
	stag_info->access_rights = irdma_get_mr_access(access);
	stag_info->pd_id = iwpd->sc_pd.pd_id;
	stag_info->all_memory = pd->flags & IB_PD_UNSAFE_GLOBAL_RKEY;
	if (stag_info->access_rights & IRDMA_ACCESS_FLAGS_ZERO_BASED)
		stag_info->addr_type = IRDMA_ADDR_TYPE_ZERO_BASED;
	else
		stag_info->addr_type = IRDMA_ADDR_TYPE_VA_BASED;
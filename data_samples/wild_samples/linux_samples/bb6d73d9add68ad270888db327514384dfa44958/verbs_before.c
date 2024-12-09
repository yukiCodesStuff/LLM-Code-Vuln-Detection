{
	struct irdma_allocate_stag_info *info;
	struct irdma_pd *iwpd = to_iwpd(iwmr->ibmr.pd);
	int status;
	struct irdma_cqp_request *cqp_request;
	struct cqp_cmds_info *cqp_info;

	cqp_request = irdma_alloc_and_get_cqp_request(&iwdev->rf->cqp, true);
	if (!cqp_request)
		return -ENOMEM;

	cqp_info = &cqp_request->info;
	info = &cqp_info->in.u.alloc_stag.info;
	memset(info, 0, sizeof(*info));
	info->page_size = PAGE_SIZE;
	info->stag_idx = iwmr->stag >> IRDMA_CQPSQ_STAG_IDX_S;
	info->pd_id = iwpd->sc_pd.pd_id;
	info->total_len = iwmr->len;
	info->remote_access = true;
	cqp_info->cqp_cmd = IRDMA_OP_ALLOC_STAG;
	cqp_info->post_sq = 1;
	cqp_info->in.u.alloc_stag.dev = &iwdev->rf->sc_dev;
	cqp_info->in.u.alloc_stag.scratch = (uintptr_t)cqp_request;
	status = irdma_handle_cqp_op(iwdev->rf, cqp_request);
	irdma_put_cqp_request(&iwdev->rf->cqp, cqp_request);

	return status;
}
{
	struct irdma_allocate_stag_info *info;
	struct irdma_pd *iwpd = to_iwpd(iwmr->ibmr.pd);
	int status;
	struct irdma_cqp_request *cqp_request;
	struct cqp_cmds_info *cqp_info;

	cqp_request = irdma_alloc_and_get_cqp_request(&iwdev->rf->cqp, true);
	if (!cqp_request)
		return -ENOMEM;

	cqp_info = &cqp_request->info;
	info = &cqp_info->in.u.alloc_stag.info;
	memset(info, 0, sizeof(*info));
	info->page_size = PAGE_SIZE;
	info->stag_idx = iwmr->stag >> IRDMA_CQPSQ_STAG_IDX_S;
	info->pd_id = iwpd->sc_pd.pd_id;
	info->total_len = iwmr->len;
	info->remote_access = true;
	cqp_info->cqp_cmd = IRDMA_OP_ALLOC_STAG;
	cqp_info->post_sq = 1;
	cqp_info->in.u.alloc_stag.dev = &iwdev->rf->sc_dev;
	cqp_info->in.u.alloc_stag.scratch = (uintptr_t)cqp_request;
	status = irdma_handle_cqp_op(iwdev->rf, cqp_request);
	irdma_put_cqp_request(&iwdev->rf->cqp, cqp_request);

	return status;
}

/**
 * irdma_alloc_mr - register stag for fast memory registration
 * @pd: ibpd pointer
 * @mr_type: memory for stag registrion
 * @max_num_sg: man number of pages
 */
static struct ib_mr *irdma_alloc_mr(struct ib_pd *pd, enum ib_mr_type mr_type,
				    u32 max_num_sg)
{
	struct irdma_device *iwdev = to_iwdev(pd->device);
	struct irdma_pble_alloc *palloc;
	struct irdma_pbl *iwpbl;
	struct irdma_mr *iwmr;
	u32 stag;
	int err_code;

	iwmr = kzalloc(sizeof(*iwmr), GFP_KERNEL);
	if (!iwmr)
		return ERR_PTR(-ENOMEM);

	stag = irdma_create_stag(iwdev);
	if (!stag) {
		err_code = -ENOMEM;
		goto err;
	}
	if (!stag) {
		err_code = -ENOMEM;
		goto err;
	}

	iwmr->stag = stag;
	iwmr->ibmr.rkey = stag;
	iwmr->ibmr.lkey = stag;
	iwmr->ibmr.pd = pd;
	iwmr->ibmr.device = pd->device;
	iwpbl = &iwmr->iwpbl;
	iwpbl->iwmr = iwmr;
	iwmr->type = IRDMA_MEMREG_TYPE_MEM;
	palloc = &iwpbl->pble_alloc;
	iwmr->page_cnt = max_num_sg;
	err_code = irdma_get_pble(iwdev->rf->pble_rsrc, palloc, iwmr->page_cnt,
				  false);
	if (err_code)
		goto err_get_pble;

	err_code = irdma_hw_alloc_stag(iwdev, iwmr);
	if (err_code)
		goto err_alloc_stag;

	iwpbl->pbl_allocated = true;

	return &iwmr->ibmr;
err_alloc_stag:
	irdma_free_pble(iwdev->rf->pble_rsrc, palloc);
err_get_pble:
	irdma_free_stag(iwdev, stag);
err:
	kfree(iwmr);

	return ERR_PTR(err_code);
}

/**
 * irdma_set_page - populate pbl list for fmr
 * @ibmr: ib mem to access iwarp mr pointer
 * @addr: page dma address fro pbl list
 */
static int irdma_set_page(struct ib_mr *ibmr, u64 addr)
{
	struct irdma_mr *iwmr = to_iwmr(ibmr);
	struct irdma_pbl *iwpbl = &iwmr->iwpbl;
	struct irdma_pble_alloc *palloc = &iwpbl->pble_alloc;
	u64 *pbl;

	if (unlikely(iwmr->npages == iwmr->page_cnt))
		return -ENOMEM;

	if (palloc->level == PBLE_LEVEL_2) {
		struct irdma_pble_info *palloc_info =
			palloc->level2.leaf + (iwmr->npages >> PBLE_512_SHIFT);

		palloc_info->addr[iwmr->npages & (PBLE_PER_PAGE - 1)] = addr;
	} else {
		pbl = palloc->level1.addr;
		pbl[iwmr->npages] = addr;
	}
	iwmr->npages++;

	return 0;
}
{
	struct irdma_pbl *iwpbl = &iwmr->iwpbl;
	struct irdma_reg_ns_stag_info *stag_info;
	struct irdma_pd *iwpd = to_iwpd(iwmr->ibmr.pd);
	struct irdma_pble_alloc *palloc = &iwpbl->pble_alloc;
	struct irdma_cqp_request *cqp_request;
	struct cqp_cmds_info *cqp_info;
	int ret;

	cqp_request = irdma_alloc_and_get_cqp_request(&iwdev->rf->cqp, true);
	if (!cqp_request)
		return -ENOMEM;

	cqp_info = &cqp_request->info;
	stag_info = &cqp_info->in.u.mr_reg_non_shared.info;
	memset(stag_info, 0, sizeof(*stag_info));
	stag_info->va = iwpbl->user_base;
	stag_info->stag_idx = iwmr->stag >> IRDMA_CQPSQ_STAG_IDX_S;
	stag_info->stag_key = (u8)iwmr->stag;
	stag_info->total_len = iwmr->len;
	stag_info->access_rights = irdma_get_mr_access(access);
	stag_info->pd_id = iwpd->sc_pd.pd_id;
	if (stag_info->access_rights & IRDMA_ACCESS_FLAGS_ZERO_BASED)
		stag_info->addr_type = IRDMA_ADDR_TYPE_ZERO_BASED;
	else
		stag_info->addr_type = IRDMA_ADDR_TYPE_VA_BASED;
	stag_info->page_size = iwmr->page_size;

	if (iwpbl->pbl_allocated) {
		if (palloc->level == PBLE_LEVEL_1) {
			stag_info->first_pm_pbl_index = palloc->level1.idx;
			stag_info->chunk_size = 1;
		} else {
			stag_info->first_pm_pbl_index = palloc->level2.root.idx;
			stag_info->chunk_size = 3;
		}
	} else {
		stag_info->reg_addr_pa = iwmr->pgaddrmem[0];
	}

	cqp_info->cqp_cmd = IRDMA_OP_MR_REG_NON_SHARED;
	cqp_info->post_sq = 1;
	cqp_info->in.u.mr_reg_non_shared.dev = &iwdev->rf->sc_dev;
	cqp_info->in.u.mr_reg_non_shared.scratch = (uintptr_t)cqp_request;
	ret = irdma_handle_cqp_op(iwdev->rf, cqp_request);
	irdma_put_cqp_request(&iwdev->rf->cqp, cqp_request);

	return ret;
}

static int irdma_reg_user_mr_type_mem(struct irdma_mr *iwmr, int access)
{
	struct irdma_device *iwdev = to_iwdev(iwmr->ibmr.device);
	struct irdma_pbl *iwpbl = &iwmr->iwpbl;
	u32 stag;
	u8 lvl;
	int err;

	lvl = iwmr->page_cnt != 1 ? PBLE_LEVEL_1 | PBLE_LEVEL_2 : PBLE_LEVEL_0;

	err = irdma_setup_pbles(iwdev->rf, iwmr, lvl);
	if (err)
		return err;

	if (lvl) {
		err = irdma_check_mr_contiguous(&iwpbl->pble_alloc,
						iwmr->page_size);
		if (err) {
			irdma_free_pble(iwdev->rf->pble_rsrc, &iwpbl->pble_alloc);
			iwpbl->pbl_allocated = false;
		}
	}

	stag = irdma_create_stag(iwdev);
	if (!stag) {
		err = -ENOMEM;
		goto free_pble;
	}

	iwmr->stag = stag;
	iwmr->ibmr.rkey = stag;
	iwmr->ibmr.lkey = stag;
	err = irdma_hwreg_mr(iwdev, iwmr, access);
	if (err)
		goto err_hwreg;

	return 0;

err_hwreg:
	irdma_free_stag(iwdev, stag);

free_pble:
	if (iwpbl->pble_alloc.level != PBLE_LEVEL_0 && iwpbl->pbl_allocated)
		irdma_free_pble(iwdev->rf->pble_rsrc, &iwpbl->pble_alloc);

	return err;
}

static struct irdma_mr *irdma_alloc_iwmr(struct ib_umem *region,
					 struct ib_pd *pd, u64 virt,
					 enum irdma_memreg_type reg_type)
{
	struct irdma_device *iwdev = to_iwdev(pd->device);
	struct irdma_pbl *iwpbl;
	struct irdma_mr *iwmr;
	unsigned long pgsz_bitmap;

	iwmr = kzalloc(sizeof(*iwmr), GFP_KERNEL);
	if (!iwmr)
		return ERR_PTR(-ENOMEM);

	iwpbl = &iwmr->iwpbl;
	iwpbl->iwmr = iwmr;
	iwmr->region = region;
	iwmr->ibmr.pd = pd;
	iwmr->ibmr.device = pd->device;
	iwmr->ibmr.iova = virt;
	iwmr->type = reg_type;

	pgsz_bitmap = (reg_type == IRDMA_MEMREG_TYPE_MEM) ?
		iwdev->rf->sc_dev.hw_attrs.page_size_cap : PAGE_SIZE;

	iwmr->page_size = ib_umem_find_best_pgsz(region, pgsz_bitmap, virt);
	if (unlikely(!iwmr->page_size)) {
		kfree(iwmr);
		return ERR_PTR(-EOPNOTSUPP);
	}

	iwmr->len = region->length;
	iwpbl->user_base = virt;
	iwmr->page_cnt = ib_umem_num_dma_blocks(region, iwmr->page_size);

	return iwmr;
}

static void irdma_free_iwmr(struct irdma_mr *iwmr)
{
	kfree(iwmr);
}

static int irdma_reg_user_mr_type_qp(struct irdma_mem_reg_req req,
				     struct ib_udata *udata,
				     struct irdma_mr *iwmr)
{
	struct irdma_device *iwdev = to_iwdev(iwmr->ibmr.device);
	struct irdma_pbl *iwpbl = &iwmr->iwpbl;
	struct irdma_ucontext *ucontext = NULL;
	unsigned long flags;
	u32 total;
	int err;
	u8 lvl;

	total = req.sq_pages + req.rq_pages + 1;
	if (total > iwmr->page_cnt)
		return -EINVAL;

	total = req.sq_pages + req.rq_pages;
	lvl = total > 2 ? PBLE_LEVEL_1 : PBLE_LEVEL_0;
	err = irdma_handle_q_mem(iwdev, &req, iwpbl, lvl);
	if (err)
		return err;

	ucontext = rdma_udata_to_drv_context(udata, struct irdma_ucontext,
					     ibucontext);
	spin_lock_irqsave(&ucontext->qp_reg_mem_list_lock, flags);
	list_add_tail(&iwpbl->list, &ucontext->qp_reg_mem_list);
	iwpbl->on_list = true;
	spin_unlock_irqrestore(&ucontext->qp_reg_mem_list_lock, flags);

	return 0;
}

static int irdma_reg_user_mr_type_cq(struct irdma_mem_reg_req req,
				     struct ib_udata *udata,
				     struct irdma_mr *iwmr)
{
	struct irdma_device *iwdev = to_iwdev(iwmr->ibmr.device);
	struct irdma_pbl *iwpbl = &iwmr->iwpbl;
	struct irdma_ucontext *ucontext = NULL;
	u8 shadow_pgcnt = 1;
	unsigned long flags;
	u32 total;
	int err;
	u8 lvl;

	if (iwdev->rf->sc_dev.hw_attrs.uk_attrs.feature_flags & IRDMA_FEATURE_CQ_RESIZE)
		shadow_pgcnt = 0;
	total = req.cq_pages + shadow_pgcnt;
	if (total > iwmr->page_cnt)
		return -EINVAL;

	lvl = req.cq_pages > 1 ? PBLE_LEVEL_1 : PBLE_LEVEL_0;
	err = irdma_handle_q_mem(iwdev, &req, iwpbl, lvl);
	if (err)
		return err;

	ucontext = rdma_udata_to_drv_context(udata, struct irdma_ucontext,
					     ibucontext);
	spin_lock_irqsave(&ucontext->cq_reg_mem_list_lock, flags);
	list_add_tail(&iwpbl->list, &ucontext->cq_reg_mem_list);
	iwpbl->on_list = true;
	spin_unlock_irqrestore(&ucontext->cq_reg_mem_list_lock, flags);

	return 0;
}

/**
 * irdma_reg_user_mr - Register a user memory region
 * @pd: ptr of pd
 * @start: virtual start address
 * @len: length of mr
 * @virt: virtual address
 * @access: access of mr
 * @udata: user data
 */
static struct ib_mr *irdma_reg_user_mr(struct ib_pd *pd, u64 start, u64 len,
				       u64 virt, int access,
				       struct ib_udata *udata)
{
#define IRDMA_MEM_REG_MIN_REQ_LEN offsetofend(struct irdma_mem_reg_req, sq_pages)
	struct irdma_device *iwdev = to_iwdev(pd->device);
	struct irdma_mem_reg_req req = {};
	struct ib_umem *region = NULL;
	struct irdma_mr *iwmr = NULL;
	int err;

	if (len > iwdev->rf->sc_dev.hw_attrs.max_mr_size)
		return ERR_PTR(-EINVAL);

	if (udata->inlen < IRDMA_MEM_REG_MIN_REQ_LEN)
		return ERR_PTR(-EINVAL);

	region = ib_umem_get(pd->device, start, len, access);

	if (IS_ERR(region)) {
		ibdev_dbg(&iwdev->ibdev,
			  "VERBS: Failed to create ib_umem region\n");
		return (struct ib_mr *)region;
	}

	if (ib_copy_from_udata(&req, udata, min(sizeof(req), udata->inlen))) {
		ib_umem_release(region);
		return ERR_PTR(-EFAULT);
	}

	iwmr = irdma_alloc_iwmr(region, pd, virt, req.reg_type);
	if (IS_ERR(iwmr)) {
		ib_umem_release(region);
		return (struct ib_mr *)iwmr;
	}

	switch (req.reg_type) {
	case IRDMA_MEMREG_TYPE_QP:
		err = irdma_reg_user_mr_type_qp(req, udata, iwmr);
		if (err)
			goto error;

		break;
	case IRDMA_MEMREG_TYPE_CQ:
		err = irdma_reg_user_mr_type_cq(req, udata, iwmr);
		if (err)
			goto error;
		break;
	case IRDMA_MEMREG_TYPE_MEM:
		err = irdma_reg_user_mr_type_mem(iwmr, access);
		if (err)
			goto error;

		break;
	default:
		err = -EINVAL;
		goto error;
	}

	return &iwmr->ibmr;
error:
	ib_umem_release(region);
	irdma_free_iwmr(iwmr);

	return ERR_PTR(err);
}

static struct ib_mr *irdma_reg_user_mr_dmabuf(struct ib_pd *pd, u64 start,
					      u64 len, u64 virt,
					      int fd, int access,
					      struct ib_udata *udata)
{
	struct irdma_device *iwdev = to_iwdev(pd->device);
	struct ib_umem_dmabuf *umem_dmabuf;
	struct irdma_mr *iwmr;
	int err;

	if (len > iwdev->rf->sc_dev.hw_attrs.max_mr_size)
		return ERR_PTR(-EINVAL);

	umem_dmabuf = ib_umem_dmabuf_get_pinned(pd->device, start, len, fd, access);
	if (IS_ERR(umem_dmabuf)) {
		err = PTR_ERR(umem_dmabuf);
		ibdev_dbg(&iwdev->ibdev, "Failed to get dmabuf umem[%d]\n", err);
		return ERR_PTR(err);
	}

	iwmr = irdma_alloc_iwmr(&umem_dmabuf->umem, pd, virt, IRDMA_MEMREG_TYPE_MEM);
	if (IS_ERR(iwmr)) {
		err = PTR_ERR(iwmr);
		goto err_release;
	}

	err = irdma_reg_user_mr_type_mem(iwmr, access);
	if (err)
		goto err_iwmr;

	return &iwmr->ibmr;

err_iwmr:
	irdma_free_iwmr(iwmr);

err_release:
	ib_umem_release(&umem_dmabuf->umem);

	return ERR_PTR(err);
}

/**
 * irdma_reg_phys_mr - register kernel physical memory
 * @pd: ibpd pointer
 * @addr: physical address of memory to register
 * @size: size of memory to register
 * @access: Access rights
 * @iova_start: start of virtual address for physical buffers
 */
struct ib_mr *irdma_reg_phys_mr(struct ib_pd *pd, u64 addr, u64 size, int access,
				u64 *iova_start)
{
	struct irdma_device *iwdev = to_iwdev(pd->device);
	struct irdma_pbl *iwpbl;
	struct irdma_mr *iwmr;
	u32 stag;
	int ret;

	iwmr = kzalloc(sizeof(*iwmr), GFP_KERNEL);
	if (!iwmr)
		return ERR_PTR(-ENOMEM);

	iwmr->ibmr.pd = pd;
	iwmr->ibmr.device = pd->device;
	iwpbl = &iwmr->iwpbl;
	iwpbl->iwmr = iwmr;
	iwmr->type = IRDMA_MEMREG_TYPE_MEM;
	iwpbl->user_base = *iova_start;
	stag = irdma_create_stag(iwdev);
	if (!stag) {
		ret = -ENOMEM;
		goto err;
	}

	iwmr->stag = stag;
	iwmr->ibmr.iova = *iova_start;
	iwmr->ibmr.rkey = stag;
	iwmr->ibmr.lkey = stag;
	iwmr->page_cnt = 1;
	iwmr->pgaddrmem[0] = addr;
	iwmr->len = size;
	iwmr->page_size = SZ_4K;
	ret = irdma_hwreg_mr(iwdev, iwmr, access);
	if (ret) {
		irdma_free_stag(iwdev, stag);
		goto err;
	}

	return &iwmr->ibmr;

err:
	kfree(iwmr);

	return ERR_PTR(ret);
}

/**
 * irdma_get_dma_mr - register physical mem
 * @pd: ptr of pd
 * @acc: access for memory
 */
static struct ib_mr *irdma_get_dma_mr(struct ib_pd *pd, int acc)
{
	u64 kva = 0;

	return irdma_reg_phys_mr(pd, 0, 0, acc, &kva);
}

/**
 * irdma_del_memlist - Deleting pbl list entries for CQ/QP
 * @iwmr: iwmr for IB's user page addresses
 * @ucontext: ptr to user context
 */
static void irdma_del_memlist(struct irdma_mr *iwmr,
			      struct irdma_ucontext *ucontext)
{
	struct irdma_pbl *iwpbl = &iwmr->iwpbl;
	unsigned long flags;

	switch (iwmr->type) {
	case IRDMA_MEMREG_TYPE_CQ:
		spin_lock_irqsave(&ucontext->cq_reg_mem_list_lock, flags);
		if (iwpbl->on_list) {
			iwpbl->on_list = false;
			list_del(&iwpbl->list);
		}
		spin_unlock_irqrestore(&ucontext->cq_reg_mem_list_lock, flags);
		break;
	case IRDMA_MEMREG_TYPE_QP:
		spin_lock_irqsave(&ucontext->qp_reg_mem_list_lock, flags);
		if (iwpbl->on_list) {
			iwpbl->on_list = false;
			list_del(&iwpbl->list);
		}
		spin_unlock_irqrestore(&ucontext->qp_reg_mem_list_lock, flags);
		break;
	default:
		break;
	}
}

/**
 * irdma_dereg_mr - deregister mr
 * @ib_mr: mr ptr for dereg
 * @udata: user data
 */
static int irdma_dereg_mr(struct ib_mr *ib_mr, struct ib_udata *udata)
{
	struct ib_pd *ibpd = ib_mr->pd;
	struct irdma_pd *iwpd = to_iwpd(ibpd);
	struct irdma_mr *iwmr = to_iwmr(ib_mr);
	struct irdma_device *iwdev = to_iwdev(ib_mr->device);
	struct irdma_dealloc_stag_info *info;
	struct irdma_pbl *iwpbl = &iwmr->iwpbl;
	struct irdma_pble_alloc *palloc = &iwpbl->pble_alloc;
	struct irdma_cqp_request *cqp_request;
	struct cqp_cmds_info *cqp_info;
	int status;

	if (iwmr->type != IRDMA_MEMREG_TYPE_MEM) {
		if (iwmr->region) {
			struct irdma_ucontext *ucontext;

			ucontext = rdma_udata_to_drv_context(udata,
						struct irdma_ucontext,
						ibucontext);
			irdma_del_memlist(iwmr, ucontext);
		}
		goto done;
	}

	cqp_request = irdma_alloc_and_get_cqp_request(&iwdev->rf->cqp, true);
	if (!cqp_request)
		return -ENOMEM;

	cqp_info = &cqp_request->info;
	info = &cqp_info->in.u.dealloc_stag.info;
	memset(info, 0, sizeof(*info));
	info->pd_id = iwpd->sc_pd.pd_id;
	info->stag_idx = ib_mr->rkey >> IRDMA_CQPSQ_STAG_IDX_S;
	info->mr = true;
	if (iwpbl->pbl_allocated)
		info->dealloc_pbl = true;

	cqp_info->cqp_cmd = IRDMA_OP_DEALLOC_STAG;
	cqp_info->post_sq = 1;
	cqp_info->in.u.dealloc_stag.dev = &iwdev->rf->sc_dev;
	cqp_info->in.u.dealloc_stag.scratch = (uintptr_t)cqp_request;
	status = irdma_handle_cqp_op(iwdev->rf, cqp_request);
	irdma_put_cqp_request(&iwdev->rf->cqp, cqp_request);
	if (status)
		return status;

	irdma_free_stag(iwdev, iwmr->stag);
done:
	if (iwpbl->pbl_allocated)
		irdma_free_pble(iwdev->rf->pble_rsrc, palloc);
	ib_umem_release(iwmr->region);
	kfree(iwmr);

	return 0;
}

/**
 * irdma_post_send -  kernel application wr
 * @ibqp: qp ptr for wr
 * @ib_wr: work request ptr
 * @bad_wr: return of bad wr if err
 */
static int irdma_post_send(struct ib_qp *ibqp,
			   const struct ib_send_wr *ib_wr,
			   const struct ib_send_wr **bad_wr)
{
	struct irdma_qp *iwqp;
	struct irdma_qp_uk *ukqp;
	struct irdma_sc_dev *dev;
	struct irdma_post_sq_info info;
	int err = 0;
	unsigned long flags;
	bool inv_stag;
	struct irdma_ah *ah;

	iwqp = to_iwqp(ibqp);
	ukqp = &iwqp->sc_qp.qp_uk;
	dev = &iwqp->iwdev->rf->sc_dev;

	spin_lock_irqsave(&iwqp->lock, flags);
	while (ib_wr) {
		memset(&info, 0, sizeof(info));
		inv_stag = false;
		info.wr_id = (ib_wr->wr_id);
		if ((ib_wr->send_flags & IB_SEND_SIGNALED) || iwqp->sig_all)
			info.signaled = true;
		if (ib_wr->send_flags & IB_SEND_FENCE)
			info.read_fence = true;
		switch (ib_wr->opcode) {
		case IB_WR_SEND_WITH_IMM:
			if (ukqp->qp_caps & IRDMA_SEND_WITH_IMM) {
				info.imm_data_valid = true;
				info.imm_data = ntohl(ib_wr->ex.imm_data);
			} else {
				err = -EINVAL;
				break;
			}
			fallthrough;
		case IB_WR_SEND:
		case IB_WR_SEND_WITH_INV:
			if (ib_wr->opcode == IB_WR_SEND ||
			    ib_wr->opcode == IB_WR_SEND_WITH_IMM) {
				if (ib_wr->send_flags & IB_SEND_SOLICITED)
					info.op_type = IRDMA_OP_TYPE_SEND_SOL;
				else
					info.op_type = IRDMA_OP_TYPE_SEND;
			} else {
				if (ib_wr->send_flags & IB_SEND_SOLICITED)
					info.op_type = IRDMA_OP_TYPE_SEND_SOL_INV;
				else
					info.op_type = IRDMA_OP_TYPE_SEND_INV;
				info.stag_to_inv = ib_wr->ex.invalidate_rkey;
			}

			info.op.send.num_sges = ib_wr->num_sge;
			info.op.send.sg_list = ib_wr->sg_list;
			if (iwqp->ibqp.qp_type == IB_QPT_UD ||
			    iwqp->ibqp.qp_type == IB_QPT_GSI) {
				ah = to_iwah(ud_wr(ib_wr)->ah);
				info.op.send.ah_id = ah->sc_ah.ah_info.ah_idx;
				info.op.send.qkey = ud_wr(ib_wr)->remote_qkey;
				info.op.send.dest_qp = ud_wr(ib_wr)->remote_qpn;
			}

			if (ib_wr->send_flags & IB_SEND_INLINE)
				err = irdma_uk_inline_send(ukqp, &info, false);
			else
				err = irdma_uk_send(ukqp, &info, false);
			break;
		case IB_WR_RDMA_WRITE_WITH_IMM:
			if (ukqp->qp_caps & IRDMA_WRITE_WITH_IMM) {
				info.imm_data_valid = true;
				info.imm_data = ntohl(ib_wr->ex.imm_data);
			} else {
				err = -EINVAL;
				break;
			}
			fallthrough;
		case IB_WR_RDMA_WRITE:
			if (ib_wr->send_flags & IB_SEND_SOLICITED)
				info.op_type = IRDMA_OP_TYPE_RDMA_WRITE_SOL;
			else
				info.op_type = IRDMA_OP_TYPE_RDMA_WRITE;

			info.op.rdma_write.num_lo_sges = ib_wr->num_sge;
			info.op.rdma_write.lo_sg_list = ib_wr->sg_list;
			info.op.rdma_write.rem_addr.addr =
				rdma_wr(ib_wr)->remote_addr;
			info.op.rdma_write.rem_addr.lkey = rdma_wr(ib_wr)->rkey;
			if (ib_wr->send_flags & IB_SEND_INLINE)
				err = irdma_uk_inline_rdma_write(ukqp, &info, false);
			else
				err = irdma_uk_rdma_write(ukqp, &info, false);
			break;
		case IB_WR_RDMA_READ_WITH_INV:
			inv_stag = true;
			fallthrough;
		case IB_WR_RDMA_READ:
			if (ib_wr->num_sge >
			    dev->hw_attrs.uk_attrs.max_hw_read_sges) {
				err = -EINVAL;
				break;
			}
			info.op_type = IRDMA_OP_TYPE_RDMA_READ;
			info.op.rdma_read.rem_addr.addr = rdma_wr(ib_wr)->remote_addr;
			info.op.rdma_read.rem_addr.lkey = rdma_wr(ib_wr)->rkey;
			info.op.rdma_read.lo_sg_list = (void *)ib_wr->sg_list;
			info.op.rdma_read.num_lo_sges = ib_wr->num_sge;
			err = irdma_uk_rdma_read(ukqp, &info, inv_stag, false);
			break;
		case IB_WR_LOCAL_INV:
			info.op_type = IRDMA_OP_TYPE_INV_STAG;
			info.local_fence = info.read_fence;
			info.op.inv_local_stag.target_stag = ib_wr->ex.invalidate_rkey;
			err = irdma_uk_stag_local_invalidate(ukqp, &info, true);
			break;
		case IB_WR_REG_MR: {
			struct irdma_mr *iwmr = to_iwmr(reg_wr(ib_wr)->mr);
			struct irdma_pble_alloc *palloc = &iwmr->iwpbl.pble_alloc;
			struct irdma_fast_reg_stag_info stag_info = {};

			stag_info.signaled = info.signaled;
			stag_info.read_fence = info.read_fence;
			stag_info.access_rights = irdma_get_mr_access(reg_wr(ib_wr)->access);
			stag_info.stag_key = reg_wr(ib_wr)->key & 0xff;
			stag_info.stag_idx = reg_wr(ib_wr)->key >> 8;
			stag_info.page_size = reg_wr(ib_wr)->mr->page_size;
			stag_info.wr_id = ib_wr->wr_id;
			stag_info.addr_type = IRDMA_ADDR_TYPE_VA_BASED;
			stag_info.va = (void *)(uintptr_t)iwmr->ibmr.iova;
			stag_info.total_len = iwmr->ibmr.length;
			stag_info.reg_addr_pa = *palloc->level1.addr;
			stag_info.first_pm_pbl_index = palloc->level1.idx;
			stag_info.local_fence = ib_wr->send_flags & IB_SEND_FENCE;
			if (iwmr->npages > IRDMA_MIN_PAGES_PER_FMR)
				stag_info.chunk_size = 1;
			err = irdma_sc_mr_fast_register(&iwqp->sc_qp, &stag_info,
							true);
			break;
		}
		default:
			err = -EINVAL;
			ibdev_dbg(&iwqp->iwdev->ibdev,
				  "VERBS: upost_send bad opcode = 0x%x\n",
				  ib_wr->opcode);
			break;
		}

		if (err)
			break;
		ib_wr = ib_wr->next;
	}

	if (!iwqp->flush_issued) {
		if (iwqp->hw_iwarp_state <= IRDMA_QP_STATE_RTS)
			irdma_uk_qp_post_wr(ukqp);
		spin_unlock_irqrestore(&iwqp->lock, flags);
	} else {
		spin_unlock_irqrestore(&iwqp->lock, flags);
		mod_delayed_work(iwqp->iwdev->cleanup_wq, &iwqp->dwork_flush,
				 msecs_to_jiffies(IRDMA_FLUSH_DELAY_MS));
	}
	if (err)
		*bad_wr = ib_wr;

	return err;
}

/**
 * irdma_post_recv - post receive wr for kernel application
 * @ibqp: ib qp pointer
 * @ib_wr: work request for receive
 * @bad_wr: bad wr caused an error
 */
static int irdma_post_recv(struct ib_qp *ibqp,
			   const struct ib_recv_wr *ib_wr,
			   const struct ib_recv_wr **bad_wr)
{
	struct irdma_qp *iwqp;
	struct irdma_qp_uk *ukqp;
	struct irdma_post_rq_info post_recv = {};
	unsigned long flags;
	int err = 0;

	iwqp = to_iwqp(ibqp);
	ukqp = &iwqp->sc_qp.qp_uk;

	spin_lock_irqsave(&iwqp->lock, flags);
	while (ib_wr) {
		post_recv.num_sges = ib_wr->num_sge;
		post_recv.wr_id = ib_wr->wr_id;
		post_recv.sg_list = ib_wr->sg_list;
		err = irdma_uk_post_receive(ukqp, &post_recv);
		if (err) {
			ibdev_dbg(&iwqp->iwdev->ibdev,
				  "VERBS: post_recv err %d\n", err);
			goto out;
		}

		ib_wr = ib_wr->next;
	}

out:
	spin_unlock_irqrestore(&iwqp->lock, flags);
	if (iwqp->flush_issued)
		mod_delayed_work(iwqp->iwdev->cleanup_wq, &iwqp->dwork_flush,
				 msecs_to_jiffies(IRDMA_FLUSH_DELAY_MS));

	if (err)
		*bad_wr = ib_wr;

	return err;
}

/**
 * irdma_flush_err_to_ib_wc_status - return change flush error code to IB status
 * @opcode: iwarp flush code
 */
static enum ib_wc_status irdma_flush_err_to_ib_wc_status(enum irdma_flush_opcode opcode)
{
	switch (opcode) {
	case FLUSH_PROT_ERR:
		return IB_WC_LOC_PROT_ERR;
	case FLUSH_REM_ACCESS_ERR:
		return IB_WC_REM_ACCESS_ERR;
	case FLUSH_LOC_QP_OP_ERR:
		return IB_WC_LOC_QP_OP_ERR;
	case FLUSH_REM_OP_ERR:
		return IB_WC_REM_OP_ERR;
	case FLUSH_LOC_LEN_ERR:
		return IB_WC_LOC_LEN_ERR;
	case FLUSH_GENERAL_ERR:
		return IB_WC_WR_FLUSH_ERR;
	case FLUSH_RETRY_EXC_ERR:
		return IB_WC_RETRY_EXC_ERR;
	case FLUSH_MW_BIND_ERR:
		return IB_WC_MW_BIND_ERR;
	case FLUSH_REM_INV_REQ_ERR:
		return IB_WC_REM_INV_REQ_ERR;
	case FLUSH_FATAL_ERR:
	default:
		return IB_WC_FATAL_ERR;
	}
}

/**
 * irdma_process_cqe - process cqe info
 * @entry: processed cqe
 * @cq_poll_info: cqe info
 */
static void irdma_process_cqe(struct ib_wc *entry,
			      struct irdma_cq_poll_info *cq_poll_info)
{
	struct irdma_sc_qp *qp;

	entry->wc_flags = 0;
	entry->pkey_index = 0;
	entry->wr_id = cq_poll_info->wr_id;

	qp = cq_poll_info->qp_handle;
	entry->qp = qp->qp_uk.back_qp;

	if (cq_poll_info->error) {
		entry->status = (cq_poll_info->comp_status == IRDMA_COMPL_STATUS_FLUSHED) ?
				irdma_flush_err_to_ib_wc_status(cq_poll_info->minor_err) : IB_WC_GENERAL_ERR;

		entry->vendor_err = cq_poll_info->major_err << 16 |
				    cq_poll_info->minor_err;
	} else {
		entry->status = IB_WC_SUCCESS;
		if (cq_poll_info->imm_valid) {
			entry->ex.imm_data = htonl(cq_poll_info->imm_data);
			entry->wc_flags |= IB_WC_WITH_IMM;
		}
		if (cq_poll_info->ud_smac_valid) {
			ether_addr_copy(entry->smac, cq_poll_info->ud_smac);
			entry->wc_flags |= IB_WC_WITH_SMAC;
		}

		if (cq_poll_info->ud_vlan_valid) {
			u16 vlan = cq_poll_info->ud_vlan & VLAN_VID_MASK;

			entry->sl = cq_poll_info->ud_vlan >> VLAN_PRIO_SHIFT;
			if (vlan) {
				entry->vlan_id = vlan;
				entry->wc_flags |= IB_WC_WITH_VLAN;
			}
		} else {
			entry->sl = 0;
		}
	}

	if (cq_poll_info->q_type == IRDMA_CQE_QTYPE_SQ) {
		set_ib_wc_op_sq(cq_poll_info, entry);
	} else {
		set_ib_wc_op_rq(cq_poll_info, entry,
				qp->qp_uk.qp_caps & IRDMA_SEND_WITH_IMM);
		if (qp->qp_uk.qp_type != IRDMA_QP_TYPE_ROCE_UD &&
		    cq_poll_info->stag_invalid_set) {
			entry->ex.invalidate_rkey = cq_poll_info->inv_stag;
			entry->wc_flags |= IB_WC_WITH_INVALIDATE;
		}
	}

	if (qp->qp_uk.qp_type == IRDMA_QP_TYPE_ROCE_UD) {
		entry->src_qp = cq_poll_info->ud_src_qpn;
		entry->slid = 0;
		entry->wc_flags |=
			(IB_WC_GRH | IB_WC_WITH_NETWORK_HDR_TYPE);
		entry->network_hdr_type = cq_poll_info->ipv4 ?
						  RDMA_NETWORK_IPV4 :
						  RDMA_NETWORK_IPV6;
	} else {
		entry->src_qp = cq_poll_info->qp_id;
	}

	entry->byte_len = cq_poll_info->bytes_xfered;
}

/**
 * irdma_poll_one - poll one entry of the CQ
 * @ukcq: ukcq to poll
 * @cur_cqe: current CQE info to be filled in
 * @entry: ibv_wc object to be filled for non-extended CQ or NULL for extended CQ
 *
 * Returns the internal irdma device error code or 0 on success
 */
static inline int irdma_poll_one(struct irdma_cq_uk *ukcq,
				 struct irdma_cq_poll_info *cur_cqe,
				 struct ib_wc *entry)
{
	int ret = irdma_uk_cq_poll_cmpl(ukcq, cur_cqe);

	if (ret)
		return ret;

	irdma_process_cqe(entry, cur_cqe);

	return 0;
}

/**
 * __irdma_poll_cq - poll cq for completion (kernel apps)
 * @iwcq: cq to poll
 * @num_entries: number of entries to poll
 * @entry: wr of a completed entry
 */
static int __irdma_poll_cq(struct irdma_cq *iwcq, int num_entries, struct ib_wc *entry)
{
	struct list_head *tmp_node, *list_node;
	struct irdma_cq_buf *last_buf = NULL;
	struct irdma_cq_poll_info *cur_cqe = &iwcq->cur_cqe;
	struct irdma_cq_buf *cq_buf;
	int ret;
	struct irdma_device *iwdev;
	struct irdma_cq_uk *ukcq;
	bool cq_new_cqe = false;
	int resized_bufs = 0;
	int npolled = 0;

	iwdev = to_iwdev(iwcq->ibcq.device);
	ukcq = &iwcq->sc_cq.cq_uk;

	/* go through the list of previously resized CQ buffers */
	list_for_each_safe(list_node, tmp_node, &iwcq->resize_list) {
		cq_buf = container_of(list_node, struct irdma_cq_buf, list);
		while (npolled < num_entries) {
			ret = irdma_poll_one(&cq_buf->cq_uk, cur_cqe, entry + npolled);
			if (!ret) {
				++npolled;
				cq_new_cqe = true;
				continue;
			}
			if (ret == -ENOENT)
				break;
			 /* QP using the CQ is destroyed. Skip reporting this CQE */
			if (ret == -EFAULT) {
				cq_new_cqe = true;
				continue;
			}
			goto error;
		}

		/* save the resized CQ buffer which received the last cqe */
		if (cq_new_cqe)
			last_buf = cq_buf;
		cq_new_cqe = false;
	}

	/* check the current CQ for new cqes */
	while (npolled < num_entries) {
		ret = irdma_poll_one(ukcq, cur_cqe, entry + npolled);
		if (ret == -ENOENT) {
			ret = irdma_generated_cmpls(iwcq, cur_cqe);
			if (!ret)
				irdma_process_cqe(entry + npolled, cur_cqe);
		}
		if (!ret) {
			++npolled;
			cq_new_cqe = true;
			continue;
		}

		if (ret == -ENOENT)
			break;
		/* QP using the CQ is destroyed. Skip reporting this CQE */
		if (ret == -EFAULT) {
			cq_new_cqe = true;
			continue;
		}
		goto error;
	}

	if (cq_new_cqe)
		/* all previous CQ resizes are complete */
		resized_bufs = irdma_process_resize_list(iwcq, iwdev, NULL);
	else if (last_buf)
		/* only CQ resizes up to the last_buf are complete */
		resized_bufs = irdma_process_resize_list(iwcq, iwdev, last_buf);
	if (resized_bufs)
		/* report to the HW the number of complete CQ resizes */
		irdma_uk_cq_set_resized_cnt(ukcq, resized_bufs);

	return npolled;
error:
	ibdev_dbg(&iwdev->ibdev, "%s: Error polling CQ, irdma_err: %d\n",
		  __func__, ret);

	return ret;
}

/**
 * irdma_poll_cq - poll cq for completion (kernel apps)
 * @ibcq: cq to poll
 * @num_entries: number of entries to poll
 * @entry: wr of a completed entry
 */
static int irdma_poll_cq(struct ib_cq *ibcq, int num_entries,
			 struct ib_wc *entry)
{
	struct irdma_cq *iwcq;
	unsigned long flags;
	int ret;

	iwcq = to_iwcq(ibcq);

	spin_lock_irqsave(&iwcq->lock, flags);
	ret = __irdma_poll_cq(iwcq, num_entries, entry);
	spin_unlock_irqrestore(&iwcq->lock, flags);

	return ret;
}

/**
 * irdma_req_notify_cq - arm cq kernel application
 * @ibcq: cq to arm
 * @notify_flags: notofication flags
 */
static int irdma_req_notify_cq(struct ib_cq *ibcq,
			       enum ib_cq_notify_flags notify_flags)
{
	struct irdma_cq *iwcq;
	struct irdma_cq_uk *ukcq;
	unsigned long flags;
	enum irdma_cmpl_notify cq_notify;
	bool promo_event = false;
	int ret = 0;

	cq_notify = notify_flags == IB_CQ_SOLICITED ?
		    IRDMA_CQ_COMPL_SOLICITED : IRDMA_CQ_COMPL_EVENT;
	iwcq = to_iwcq(ibcq);
	ukcq = &iwcq->sc_cq.cq_uk;

	spin_lock_irqsave(&iwcq->lock, flags);
	/* Only promote to arm the CQ for any event if the last arm event was solicited. */
	if (iwcq->last_notify == IRDMA_CQ_COMPL_SOLICITED && notify_flags != IB_CQ_SOLICITED)
		promo_event = true;

	if (!atomic_cmpxchg(&iwcq->armed, 0, 1) || promo_event) {
		iwcq->last_notify = cq_notify;
		irdma_uk_cq_request_notification(ukcq, cq_notify);
	}

	if ((notify_flags & IB_CQ_REPORT_MISSED_EVENTS) &&
	    (!irdma_cq_empty(iwcq) || !list_empty(&iwcq->cmpl_generated)))
		ret = 1;
	spin_unlock_irqrestore(&iwcq->lock, flags);

	return ret;
}

static int irdma_roce_port_immutable(struct ib_device *ibdev, u32 port_num,
				     struct ib_port_immutable *immutable)
{
	struct ib_port_attr attr;
	int err;

	immutable->core_cap_flags = RDMA_CORE_PORT_IBA_ROCE_UDP_ENCAP;
	err = ib_query_port(ibdev, port_num, &attr);
	if (err)
		return err;

	immutable->max_mad_size = IB_MGMT_MAD_SIZE;
	immutable->pkey_tbl_len = attr.pkey_tbl_len;
	immutable->gid_tbl_len = attr.gid_tbl_len;

	return 0;
}

static int irdma_iw_port_immutable(struct ib_device *ibdev, u32 port_num,
				   struct ib_port_immutable *immutable)
{
	struct ib_port_attr attr;
	int err;

	immutable->core_cap_flags = RDMA_CORE_PORT_IWARP;
	err = ib_query_port(ibdev, port_num, &attr);
	if (err)
		return err;
	immutable->gid_tbl_len = attr.gid_tbl_len;

	return 0;
}

static const struct rdma_stat_desc irdma_hw_stat_names[] = {
	/* gen1 - 32-bit */
	[IRDMA_HW_STAT_INDEX_IP4RXDISCARD].name		= "ip4InDiscards",
	[IRDMA_HW_STAT_INDEX_IP4RXTRUNC].name		= "ip4InTruncatedPkts",
	[IRDMA_HW_STAT_INDEX_IP4TXNOROUTE].name		= "ip4OutNoRoutes",
	[IRDMA_HW_STAT_INDEX_IP6RXDISCARD].name		= "ip6InDiscards",
	[IRDMA_HW_STAT_INDEX_IP6RXTRUNC].name		= "ip6InTruncatedPkts",
	[IRDMA_HW_STAT_INDEX_IP6TXNOROUTE].name		= "ip6OutNoRoutes",
	[IRDMA_HW_STAT_INDEX_TCPRTXSEG].name		= "tcpRetransSegs",
	[IRDMA_HW_STAT_INDEX_TCPRXOPTERR].name		= "tcpInOptErrors",
	[IRDMA_HW_STAT_INDEX_TCPRXPROTOERR].name	= "tcpInProtoErrors",
	[IRDMA_HW_STAT_INDEX_RXVLANERR].name		= "rxVlanErrors",
	/* gen1 - 64-bit */
	[IRDMA_HW_STAT_INDEX_IP4RXOCTS].name		= "ip4InOctets",
	[IRDMA_HW_STAT_INDEX_IP4RXPKTS].name		= "ip4InPkts",
	[IRDMA_HW_STAT_INDEX_IP4RXFRAGS].name		= "ip4InReasmRqd",
	[IRDMA_HW_STAT_INDEX_IP4RXMCPKTS].name		= "ip4InMcastPkts",
	[IRDMA_HW_STAT_INDEX_IP4TXOCTS].name		= "ip4OutOctets",
	[IRDMA_HW_STAT_INDEX_IP4TXPKTS].name		= "ip4OutPkts",
	[IRDMA_HW_STAT_INDEX_IP4TXFRAGS].name		= "ip4OutSegRqd",
	[IRDMA_HW_STAT_INDEX_IP4TXMCPKTS].name		= "ip4OutMcastPkts",
	[IRDMA_HW_STAT_INDEX_IP6RXOCTS].name		= "ip6InOctets",
	[IRDMA_HW_STAT_INDEX_IP6RXPKTS].name		= "ip6InPkts",
	[IRDMA_HW_STAT_INDEX_IP6RXFRAGS].name		= "ip6InReasmRqd",
	[IRDMA_HW_STAT_INDEX_IP6RXMCPKTS].name		= "ip6InMcastPkts",
	[IRDMA_HW_STAT_INDEX_IP6TXOCTS].name		= "ip6OutOctets",
	[IRDMA_HW_STAT_INDEX_IP6TXPKTS].name		= "ip6OutPkts",
	[IRDMA_HW_STAT_INDEX_IP6TXFRAGS].name		= "ip6OutSegRqd",
	[IRDMA_HW_STAT_INDEX_IP6TXMCPKTS].name		= "ip6OutMcastPkts",
	[IRDMA_HW_STAT_INDEX_TCPRXSEGS].name		= "tcpInSegs",
	[IRDMA_HW_STAT_INDEX_TCPTXSEG].name		= "tcpOutSegs",
	[IRDMA_HW_STAT_INDEX_RDMARXRDS].name		= "iwInRdmaReads",
	[IRDMA_HW_STAT_INDEX_RDMARXSNDS].name		= "iwInRdmaSends",
	[IRDMA_HW_STAT_INDEX_RDMARXWRS].name		= "iwInRdmaWrites",
	[IRDMA_HW_STAT_INDEX_RDMATXRDS].name		= "iwOutRdmaReads",
	[IRDMA_HW_STAT_INDEX_RDMATXSNDS].name		= "iwOutRdmaSends",
	[IRDMA_HW_STAT_INDEX_RDMATXWRS].name		= "iwOutRdmaWrites",
	[IRDMA_HW_STAT_INDEX_RDMAVBND].name		= "iwRdmaBnd",
	[IRDMA_HW_STAT_INDEX_RDMAVINV].name		= "iwRdmaInv",

	/* gen2 - 32-bit */
	[IRDMA_HW_STAT_INDEX_RXRPCNPHANDLED].name	= "cnpHandled",
	[IRDMA_HW_STAT_INDEX_RXRPCNPIGNORED].name	= "cnpIgnored",
	[IRDMA_HW_STAT_INDEX_TXNPCNPSENT].name		= "cnpSent",
	/* gen2 - 64-bit */
	[IRDMA_HW_STAT_INDEX_IP4RXMCOCTS].name		= "ip4InMcastOctets",
	[IRDMA_HW_STAT_INDEX_IP4TXMCOCTS].name		= "ip4OutMcastOctets",
	[IRDMA_HW_STAT_INDEX_IP6RXMCOCTS].name		= "ip6InMcastOctets",
	[IRDMA_HW_STAT_INDEX_IP6TXMCOCTS].name		= "ip6OutMcastOctets",
	[IRDMA_HW_STAT_INDEX_UDPRXPKTS].name		= "RxUDP",
	[IRDMA_HW_STAT_INDEX_UDPTXPKTS].name		= "TxUDP",
	[IRDMA_HW_STAT_INDEX_RXNPECNMARKEDPKTS].name	= "RxECNMrkd",

};

static void irdma_get_dev_fw_str(struct ib_device *dev, char *str)
{
	struct irdma_device *iwdev = to_iwdev(dev);

	snprintf(str, IB_FW_VERSION_NAME_MAX, "%u.%u",
		 irdma_fw_major_ver(&iwdev->rf->sc_dev),
		 irdma_fw_minor_ver(&iwdev->rf->sc_dev));
}

/**
 * irdma_alloc_hw_port_stats - Allocate a hw stats structure
 * @ibdev: device pointer from stack
 * @port_num: port number
 */
static struct rdma_hw_stats *irdma_alloc_hw_port_stats(struct ib_device *ibdev,
						       u32 port_num)
{
	struct irdma_device *iwdev = to_iwdev(ibdev);
	struct irdma_sc_dev *dev = &iwdev->rf->sc_dev;

	int num_counters = dev->hw_attrs.max_stat_idx;
	unsigned long lifespan = RDMA_HW_STATS_DEFAULT_LIFESPAN;

	return rdma_alloc_hw_stats_struct(irdma_hw_stat_names, num_counters,
					  lifespan);
}

/**
 * irdma_get_hw_stats - Populates the rdma_hw_stats structure
 * @ibdev: device pointer from stack
 * @stats: stats pointer from stack
 * @port_num: port number
 * @index: which hw counter the stack is requesting we update
 */
static int irdma_get_hw_stats(struct ib_device *ibdev,
			      struct rdma_hw_stats *stats, u32 port_num,
			      int index)
{
	struct irdma_device *iwdev = to_iwdev(ibdev);
	struct irdma_dev_hw_stats *hw_stats = &iwdev->vsi.pestat->hw_stats;

	if (iwdev->rf->rdma_ver >= IRDMA_GEN_2)
		irdma_cqp_gather_stats_cmd(&iwdev->rf->sc_dev, iwdev->vsi.pestat, true);
	else
		irdma_cqp_gather_stats_gen1(&iwdev->rf->sc_dev, iwdev->vsi.pestat);

	memcpy(&stats->value[0], hw_stats, sizeof(u64) * stats->num_counters);

	return stats->num_counters;
}

/**
 * irdma_query_gid - Query port GID
 * @ibdev: device pointer from stack
 * @port: port number
 * @index: Entry index
 * @gid: Global ID
 */
static int irdma_query_gid(struct ib_device *ibdev, u32 port, int index,
			   union ib_gid *gid)
{
	struct irdma_device *iwdev = to_iwdev(ibdev);

	memset(gid->raw, 0, sizeof(gid->raw));
	ether_addr_copy(gid->raw, iwdev->netdev->dev_addr);

	return 0;
}

/**
 * mcast_list_add -  Add a new mcast item to list
 * @rf: RDMA PCI function
 * @new_elem: pointer to element to add
 */
static void mcast_list_add(struct irdma_pci_f *rf,
			   struct mc_table_list *new_elem)
{
	list_add(&new_elem->list, &rf->mc_qht_list.list);
}

/**
 * mcast_list_del - Remove an mcast item from list
 * @mc_qht_elem: pointer to mcast table list element
 */
static void mcast_list_del(struct mc_table_list *mc_qht_elem)
{
	if (mc_qht_elem)
		list_del(&mc_qht_elem->list);
}

/**
 * mcast_list_lookup_ip - Search mcast list for address
 * @rf: RDMA PCI function
 * @ip_mcast: pointer to mcast IP address
 */
static struct mc_table_list *mcast_list_lookup_ip(struct irdma_pci_f *rf,
						  u32 *ip_mcast)
{
	struct mc_table_list *mc_qht_el;
	struct list_head *pos, *q;

	list_for_each_safe (pos, q, &rf->mc_qht_list.list) {
		mc_qht_el = list_entry(pos, struct mc_table_list, list);
		if (!memcmp(mc_qht_el->mc_info.dest_ip, ip_mcast,
			    sizeof(mc_qht_el->mc_info.dest_ip)))
			return mc_qht_el;
	}

	return NULL;
}

/**
 * irdma_mcast_cqp_op - perform a mcast cqp operation
 * @iwdev: irdma device
 * @mc_grp_ctx: mcast group info
 * @op: operation
 *
 * returns error status
 */
static int irdma_mcast_cqp_op(struct irdma_device *iwdev,
			      struct irdma_mcast_grp_info *mc_grp_ctx, u8 op)
{
	struct cqp_cmds_info *cqp_info;
	struct irdma_cqp_request *cqp_request;
	int status;

	cqp_request = irdma_alloc_and_get_cqp_request(&iwdev->rf->cqp, true);
	if (!cqp_request)
		return -ENOMEM;

	cqp_request->info.in.u.mc_create.info = *mc_grp_ctx;
	cqp_info = &cqp_request->info;
	cqp_info->cqp_cmd = op;
	cqp_info->post_sq = 1;
	cqp_info->in.u.mc_create.scratch = (uintptr_t)cqp_request;
	cqp_info->in.u.mc_create.cqp = &iwdev->rf->cqp.sc_cqp;
	status = irdma_handle_cqp_op(iwdev->rf, cqp_request);
	irdma_put_cqp_request(&iwdev->rf->cqp, cqp_request);

	return status;
}

/**
 * irdma_mcast_mac - Get the multicast MAC for an IP address
 * @ip_addr: IPv4 or IPv6 address
 * @mac: pointer to result MAC address
 * @ipv4: flag indicating IPv4 or IPv6
 *
 */
void irdma_mcast_mac(u32 *ip_addr, u8 *mac, bool ipv4)
{
	u8 *ip = (u8 *)ip_addr;

	if (ipv4) {
		unsigned char mac4[ETH_ALEN] = {0x01, 0x00, 0x5E, 0x00,
						0x00, 0x00};

		mac4[3] = ip[2] & 0x7F;
		mac4[4] = ip[1];
		mac4[5] = ip[0];
		ether_addr_copy(mac, mac4);
	} else {
		unsigned char mac6[ETH_ALEN] = {0x33, 0x33, 0x00, 0x00,
						0x00, 0x00};

		mac6[2] = ip[3];
		mac6[3] = ip[2];
		mac6[4] = ip[1];
		mac6[5] = ip[0];
		ether_addr_copy(mac, mac6);
	}
}

/**
 * irdma_attach_mcast - attach a qp to a multicast group
 * @ibqp: ptr to qp
 * @ibgid: pointer to global ID
 * @lid: local ID
 *
 * returns error status
 */
static int irdma_attach_mcast(struct ib_qp *ibqp, union ib_gid *ibgid, u16 lid)
{
	struct irdma_qp *iwqp = to_iwqp(ibqp);
	struct irdma_device *iwdev = iwqp->iwdev;
	struct irdma_pci_f *rf = iwdev->rf;
	struct mc_table_list *mc_qht_elem;
	struct irdma_mcast_grp_ctx_entry_info mcg_info = {};
	unsigned long flags;
	u32 ip_addr[4] = {};
	u32 mgn;
	u32 no_mgs;
	int ret = 0;
	bool ipv4;
	u16 vlan_id;
	union irdma_sockaddr sgid_addr;
	unsigned char dmac[ETH_ALEN];

	rdma_gid2ip((struct sockaddr *)&sgid_addr, ibgid);

	if (!ipv6_addr_v4mapped((struct in6_addr *)ibgid)) {
		irdma_copy_ip_ntohl(ip_addr,
				    sgid_addr.saddr_in6.sin6_addr.in6_u.u6_addr32);
		irdma_get_vlan_mac_ipv6(ip_addr, &vlan_id, NULL);
		ipv4 = false;
		ibdev_dbg(&iwdev->ibdev,
			  "VERBS: qp_id=%d, IP6address=%pI6\n", ibqp->qp_num,
			  ip_addr);
		irdma_mcast_mac(ip_addr, dmac, false);
	} else {
		ip_addr[0] = ntohl(sgid_addr.saddr_in.sin_addr.s_addr);
		ipv4 = true;
		vlan_id = irdma_get_vlan_ipv4(ip_addr);
		irdma_mcast_mac(ip_addr, dmac, true);
		ibdev_dbg(&iwdev->ibdev,
			  "VERBS: qp_id=%d, IP4address=%pI4, MAC=%pM\n",
			  ibqp->qp_num, ip_addr, dmac);
	}

	spin_lock_irqsave(&rf->qh_list_lock, flags);
	mc_qht_elem = mcast_list_lookup_ip(rf, ip_addr);
	if (!mc_qht_elem) {
		struct irdma_dma_mem *dma_mem_mc;

		spin_unlock_irqrestore(&rf->qh_list_lock, flags);
		mc_qht_elem = kzalloc(sizeof(*mc_qht_elem), GFP_KERNEL);
		if (!mc_qht_elem)
			return -ENOMEM;

		mc_qht_elem->mc_info.ipv4_valid = ipv4;
		memcpy(mc_qht_elem->mc_info.dest_ip, ip_addr,
		       sizeof(mc_qht_elem->mc_info.dest_ip));
		ret = irdma_alloc_rsrc(rf, rf->allocated_mcgs, rf->max_mcg,
				       &mgn, &rf->next_mcg);
		if (ret) {
			kfree(mc_qht_elem);
			return -ENOMEM;
		}

		mc_qht_elem->mc_info.mgn = mgn;
		dma_mem_mc = &mc_qht_elem->mc_grp_ctx.dma_mem_mc;
		dma_mem_mc->size = ALIGN(sizeof(u64) * IRDMA_MAX_MGS_PER_CTX,
					 IRDMA_HW_PAGE_SIZE);
		dma_mem_mc->va = dma_alloc_coherent(rf->hw.device,
						    dma_mem_mc->size,
						    &dma_mem_mc->pa,
						    GFP_KERNEL);
		if (!dma_mem_mc->va) {
			irdma_free_rsrc(rf, rf->allocated_mcgs, mgn);
			kfree(mc_qht_elem);
			return -ENOMEM;
		}

		mc_qht_elem->mc_grp_ctx.mg_id = (u16)mgn;
		memcpy(mc_qht_elem->mc_grp_ctx.dest_ip_addr, ip_addr,
		       sizeof(mc_qht_elem->mc_grp_ctx.dest_ip_addr));
		mc_qht_elem->mc_grp_ctx.ipv4_valid = ipv4;
		mc_qht_elem->mc_grp_ctx.vlan_id = vlan_id;
		if (vlan_id < VLAN_N_VID)
			mc_qht_elem->mc_grp_ctx.vlan_valid = true;
		mc_qht_elem->mc_grp_ctx.hmc_fcn_id = iwdev->rf->sc_dev.hmc_fn_id;
		mc_qht_elem->mc_grp_ctx.qs_handle =
			iwqp->sc_qp.vsi->qos[iwqp->sc_qp.user_pri].qs_handle;
		ether_addr_copy(mc_qht_elem->mc_grp_ctx.dest_mac_addr, dmac);

		spin_lock_irqsave(&rf->qh_list_lock, flags);
		mcast_list_add(rf, mc_qht_elem);
	} else {
		if (mc_qht_elem->mc_grp_ctx.no_of_mgs ==
		    IRDMA_MAX_MGS_PER_CTX) {
			spin_unlock_irqrestore(&rf->qh_list_lock, flags);
			return -ENOMEM;
		}
	}

	mcg_info.qp_id = iwqp->ibqp.qp_num;
	no_mgs = mc_qht_elem->mc_grp_ctx.no_of_mgs;
	irdma_sc_add_mcast_grp(&mc_qht_elem->mc_grp_ctx, &mcg_info);
	spin_unlock_irqrestore(&rf->qh_list_lock, flags);

	/* Only if there is a change do we need to modify or create */
	if (!no_mgs) {
		ret = irdma_mcast_cqp_op(iwdev, &mc_qht_elem->mc_grp_ctx,
					 IRDMA_OP_MC_CREATE);
	} else if (no_mgs != mc_qht_elem->mc_grp_ctx.no_of_mgs) {
		ret = irdma_mcast_cqp_op(iwdev, &mc_qht_elem->mc_grp_ctx,
					 IRDMA_OP_MC_MODIFY);
	} else {
		return 0;
	}

	if (ret)
		goto error;

	return 0;

error:
	irdma_sc_del_mcast_grp(&mc_qht_elem->mc_grp_ctx, &mcg_info);
	if (!mc_qht_elem->mc_grp_ctx.no_of_mgs) {
		mcast_list_del(mc_qht_elem);
		dma_free_coherent(rf->hw.device,
				  mc_qht_elem->mc_grp_ctx.dma_mem_mc.size,
				  mc_qht_elem->mc_grp_ctx.dma_mem_mc.va,
				  mc_qht_elem->mc_grp_ctx.dma_mem_mc.pa);
		mc_qht_elem->mc_grp_ctx.dma_mem_mc.va = NULL;
		irdma_free_rsrc(rf, rf->allocated_mcgs,
				mc_qht_elem->mc_grp_ctx.mg_id);
		kfree(mc_qht_elem);
	}

	return ret;
}

/**
 * irdma_detach_mcast - detach a qp from a multicast group
 * @ibqp: ptr to qp
 * @ibgid: pointer to global ID
 * @lid: local ID
 *
 * returns error status
 */
static int irdma_detach_mcast(struct ib_qp *ibqp, union ib_gid *ibgid, u16 lid)
{
	struct irdma_qp *iwqp = to_iwqp(ibqp);
	struct irdma_device *iwdev = iwqp->iwdev;
	struct irdma_pci_f *rf = iwdev->rf;
	u32 ip_addr[4] = {};
	struct mc_table_list *mc_qht_elem;
	struct irdma_mcast_grp_ctx_entry_info mcg_info = {};
	int ret;
	unsigned long flags;
	union irdma_sockaddr sgid_addr;

	rdma_gid2ip((struct sockaddr *)&sgid_addr, ibgid);
	if (!ipv6_addr_v4mapped((struct in6_addr *)ibgid))
		irdma_copy_ip_ntohl(ip_addr,
				    sgid_addr.saddr_in6.sin6_addr.in6_u.u6_addr32);
	else
		ip_addr[0] = ntohl(sgid_addr.saddr_in.sin_addr.s_addr);

	spin_lock_irqsave(&rf->qh_list_lock, flags);
	mc_qht_elem = mcast_list_lookup_ip(rf, ip_addr);
	if (!mc_qht_elem) {
		spin_unlock_irqrestore(&rf->qh_list_lock, flags);
		ibdev_dbg(&iwdev->ibdev,
			  "VERBS: address not found MCG\n");
		return 0;
	}

	mcg_info.qp_id = iwqp->ibqp.qp_num;
	irdma_sc_del_mcast_grp(&mc_qht_elem->mc_grp_ctx, &mcg_info);
	if (!mc_qht_elem->mc_grp_ctx.no_of_mgs) {
		mcast_list_del(mc_qht_elem);
		spin_unlock_irqrestore(&rf->qh_list_lock, flags);
		ret = irdma_mcast_cqp_op(iwdev, &mc_qht_elem->mc_grp_ctx,
					 IRDMA_OP_MC_DESTROY);
		if (ret) {
			ibdev_dbg(&iwdev->ibdev,
				  "VERBS: failed MC_DESTROY MCG\n");
			spin_lock_irqsave(&rf->qh_list_lock, flags);
			mcast_list_add(rf, mc_qht_elem);
			spin_unlock_irqrestore(&rf->qh_list_lock, flags);
			return -EAGAIN;
		}

		dma_free_coherent(rf->hw.device,
				  mc_qht_elem->mc_grp_ctx.dma_mem_mc.size,
				  mc_qht_elem->mc_grp_ctx.dma_mem_mc.va,
				  mc_qht_elem->mc_grp_ctx.dma_mem_mc.pa);
		mc_qht_elem->mc_grp_ctx.dma_mem_mc.va = NULL;
		irdma_free_rsrc(rf, rf->allocated_mcgs,
				mc_qht_elem->mc_grp_ctx.mg_id);
		kfree(mc_qht_elem);
	} else {
		spin_unlock_irqrestore(&rf->qh_list_lock, flags);
		ret = irdma_mcast_cqp_op(iwdev, &mc_qht_elem->mc_grp_ctx,
					 IRDMA_OP_MC_MODIFY);
		if (ret) {
			ibdev_dbg(&iwdev->ibdev,
				  "VERBS: failed Modify MCG\n");
			return ret;
		}
	}

	return 0;
}

static int irdma_create_hw_ah(struct irdma_device *iwdev, struct irdma_ah *ah, bool sleep)
{
	struct irdma_pci_f *rf = iwdev->rf;
	int err;

	err = irdma_alloc_rsrc(rf, rf->allocated_ahs, rf->max_ah, &ah->sc_ah.ah_info.ah_idx,
			       &rf->next_ah);
	if (err)
		return err;

	err = irdma_ah_cqp_op(rf, &ah->sc_ah, IRDMA_OP_AH_CREATE, sleep,
			      irdma_gsi_ud_qp_ah_cb, &ah->sc_ah);

	if (err) {
		ibdev_dbg(&iwdev->ibdev, "VERBS: CQP-OP Create AH fail");
		goto err_ah_create;
	}

	if (!sleep) {
		int cnt = CQP_COMPL_WAIT_TIME_MS * CQP_TIMEOUT_THRESHOLD;

		do {
			irdma_cqp_ce_handler(rf, &rf->ccq.sc_cq);
			mdelay(1);
		} while (!ah->sc_ah.ah_info.ah_valid && --cnt);

		if (!cnt) {
			ibdev_dbg(&iwdev->ibdev, "VERBS: CQP create AH timed out");
			err = -ETIMEDOUT;
			goto err_ah_create;
		}
	}
	return 0;

err_ah_create:
	irdma_free_rsrc(iwdev->rf, iwdev->rf->allocated_ahs, ah->sc_ah.ah_info.ah_idx);

	return err;
}

static int irdma_setup_ah(struct ib_ah *ibah, struct rdma_ah_init_attr *attr)
{
	struct irdma_pd *pd = to_iwpd(ibah->pd);
	struct irdma_ah *ah = container_of(ibah, struct irdma_ah, ibah);
	struct rdma_ah_attr *ah_attr = attr->ah_attr;
	const struct ib_gid_attr *sgid_attr;
	struct irdma_device *iwdev = to_iwdev(ibah->pd->device);
	struct irdma_pci_f *rf = iwdev->rf;
	struct irdma_sc_ah *sc_ah;
	struct irdma_ah_info *ah_info;
	union irdma_sockaddr sgid_addr, dgid_addr;
	int err;
	u8 dmac[ETH_ALEN];

	ah->pd = pd;
	sc_ah = &ah->sc_ah;
	sc_ah->ah_info.vsi = &iwdev->vsi;
	irdma_sc_init_ah(&rf->sc_dev, sc_ah);
	ah->sgid_index = ah_attr->grh.sgid_index;
	sgid_attr = ah_attr->grh.sgid_attr;
	memcpy(&ah->dgid, &ah_attr->grh.dgid, sizeof(ah->dgid));
	rdma_gid2ip((struct sockaddr *)&sgid_addr, &sgid_attr->gid);
	rdma_gid2ip((struct sockaddr *)&dgid_addr, &ah_attr->grh.dgid);
	ah->av.attrs = *ah_attr;
	ah->av.net_type = rdma_gid_attr_network_type(sgid_attr);
	ah_info = &sc_ah->ah_info;
	ah_info->pd_idx = pd->sc_pd.pd_id;
	if (ah_attr->ah_flags & IB_AH_GRH) {
		ah_info->flow_label = ah_attr->grh.flow_label;
		ah_info->hop_ttl = ah_attr->grh.hop_limit;
		ah_info->tc_tos = ah_attr->grh.traffic_class;
	}

	ether_addr_copy(dmac, ah_attr->roce.dmac);
	if (ah->av.net_type == RDMA_NETWORK_IPV4) {
		ah_info->ipv4_valid = true;
		ah_info->dest_ip_addr[0] =
			ntohl(dgid_addr.saddr_in.sin_addr.s_addr);
		ah_info->src_ip_addr[0] =
			ntohl(sgid_addr.saddr_in.sin_addr.s_addr);
		ah_info->do_lpbk = irdma_ipv4_is_lpb(ah_info->src_ip_addr[0],
						     ah_info->dest_ip_addr[0]);
		if (ipv4_is_multicast(dgid_addr.saddr_in.sin_addr.s_addr)) {
			ah_info->do_lpbk = true;
			irdma_mcast_mac(ah_info->dest_ip_addr, dmac, true);
		}
	} else {
		irdma_copy_ip_ntohl(ah_info->dest_ip_addr,
				    dgid_addr.saddr_in6.sin6_addr.in6_u.u6_addr32);
		irdma_copy_ip_ntohl(ah_info->src_ip_addr,
				    sgid_addr.saddr_in6.sin6_addr.in6_u.u6_addr32);
		ah_info->do_lpbk = irdma_ipv6_is_lpb(ah_info->src_ip_addr,
						     ah_info->dest_ip_addr);
		if (rdma_is_multicast_addr(&dgid_addr.saddr_in6.sin6_addr)) {
			ah_info->do_lpbk = true;
			irdma_mcast_mac(ah_info->dest_ip_addr, dmac, false);
		}
	}

	err = rdma_read_gid_l2_fields(sgid_attr, &ah_info->vlan_tag,
				      ah_info->mac_addr);
	if (err)
		return err;

	ah_info->dst_arpindex = irdma_add_arp(iwdev->rf, ah_info->dest_ip_addr,
					      ah_info->ipv4_valid, dmac);

	if (ah_info->dst_arpindex == -1)
		return -EINVAL;

	if (ah_info->vlan_tag >= VLAN_N_VID && iwdev->dcb_vlan_mode)
		ah_info->vlan_tag = 0;

	if (ah_info->vlan_tag < VLAN_N_VID) {
		u8 prio = rt_tos2priority(ah_info->tc_tos);

		prio = irdma_roce_get_vlan_prio(sgid_attr, prio);

		ah_info->vlan_tag |= (u16)prio << VLAN_PRIO_SHIFT;
		ah_info->insert_vlan_tag = true;
	}

	return 0;
}

/**
 * irdma_ah_exists - Check for existing identical AH
 * @iwdev: irdma device
 * @new_ah: AH to check for
 *
 * returns true if AH is found, false if not found.
 */
static bool irdma_ah_exists(struct irdma_device *iwdev,
			    struct irdma_ah *new_ah)
{
	struct irdma_ah *ah;
	u32 key = new_ah->sc_ah.ah_info.dest_ip_addr[0] ^
		  new_ah->sc_ah.ah_info.dest_ip_addr[1] ^
		  new_ah->sc_ah.ah_info.dest_ip_addr[2] ^
		  new_ah->sc_ah.ah_info.dest_ip_addr[3];

	hash_for_each_possible(iwdev->ah_hash_tbl, ah, list, key) {
		/* Set ah_valid and ah_id the same so memcmp can work */
		new_ah->sc_ah.ah_info.ah_idx = ah->sc_ah.ah_info.ah_idx;
		new_ah->sc_ah.ah_info.ah_valid = ah->sc_ah.ah_info.ah_valid;
		if (!memcmp(&ah->sc_ah.ah_info, &new_ah->sc_ah.ah_info,
			    sizeof(ah->sc_ah.ah_info))) {
			refcount_inc(&ah->refcnt);
			new_ah->parent_ah = ah;
			return true;
		}
	}

	return false;
}

/**
 * irdma_destroy_ah - Destroy address handle
 * @ibah: pointer to address handle
 * @ah_flags: flags for sleepable
 */
static int irdma_destroy_ah(struct ib_ah *ibah, u32 ah_flags)
{
	struct irdma_device *iwdev = to_iwdev(ibah->device);
	struct irdma_ah *ah = to_iwah(ibah);

	if ((ah_flags & RDMA_DESTROY_AH_SLEEPABLE) && ah->parent_ah) {
		mutex_lock(&iwdev->ah_tbl_lock);
		if (!refcount_dec_and_test(&ah->parent_ah->refcnt)) {
			mutex_unlock(&iwdev->ah_tbl_lock);
			return 0;
		}
		hash_del(&ah->parent_ah->list);
		kfree(ah->parent_ah);
		mutex_unlock(&iwdev->ah_tbl_lock);
	}

	irdma_ah_cqp_op(iwdev->rf, &ah->sc_ah, IRDMA_OP_AH_DESTROY,
			false, NULL, ah);

	irdma_free_rsrc(iwdev->rf, iwdev->rf->allocated_ahs,
			ah->sc_ah.ah_info.ah_idx);

	return 0;
}

/**
 * irdma_create_user_ah - create user address handle
 * @ibah: address handle
 * @attr: address handle attributes
 * @udata: User data
 *
 * returns 0 on success, error otherwise
 */
static int irdma_create_user_ah(struct ib_ah *ibah,
				struct rdma_ah_init_attr *attr,
				struct ib_udata *udata)
{
#define IRDMA_CREATE_AH_MIN_RESP_LEN offsetofend(struct irdma_create_ah_resp, rsvd)
	struct irdma_ah *ah = container_of(ibah, struct irdma_ah, ibah);
	struct irdma_device *iwdev = to_iwdev(ibah->pd->device);
	struct irdma_create_ah_resp uresp;
	struct irdma_ah *parent_ah;
	int err;

	if (udata && udata->outlen < IRDMA_CREATE_AH_MIN_RESP_LEN)
		return -EINVAL;

	err = irdma_setup_ah(ibah, attr);
	if (err)
		return err;
	mutex_lock(&iwdev->ah_tbl_lock);
	if (!irdma_ah_exists(iwdev, ah)) {
		err = irdma_create_hw_ah(iwdev, ah, true);
		if (err) {
			mutex_unlock(&iwdev->ah_tbl_lock);
			return err;
		}
		/* Add new AH to list */
		parent_ah = kmemdup(ah, sizeof(*ah), GFP_KERNEL);
		if (parent_ah) {
			u32 key = parent_ah->sc_ah.ah_info.dest_ip_addr[0] ^
				  parent_ah->sc_ah.ah_info.dest_ip_addr[1] ^
				  parent_ah->sc_ah.ah_info.dest_ip_addr[2] ^
				  parent_ah->sc_ah.ah_info.dest_ip_addr[3];

			ah->parent_ah = parent_ah;
			hash_add(iwdev->ah_hash_tbl, &parent_ah->list, key);
			refcount_set(&parent_ah->refcnt, 1);
		}
	}
	mutex_unlock(&iwdev->ah_tbl_lock);

	uresp.ah_id = ah->sc_ah.ah_info.ah_idx;
	err = ib_copy_to_udata(udata, &uresp, min(sizeof(uresp), udata->outlen));
	if (err)
		irdma_destroy_ah(ibah, attr->flags);

	return err;
}

/**
 * irdma_create_ah - create address handle
 * @ibah: address handle
 * @attr: address handle attributes
 * @udata: NULL
 *
 * returns 0 on success, error otherwise
 */
static int irdma_create_ah(struct ib_ah *ibah, struct rdma_ah_init_attr *attr,
			   struct ib_udata *udata)
{
	struct irdma_ah *ah = container_of(ibah, struct irdma_ah, ibah);
	struct irdma_device *iwdev = to_iwdev(ibah->pd->device);
	int err;

	err = irdma_setup_ah(ibah, attr);
	if (err)
		return err;
	err = irdma_create_hw_ah(iwdev, ah, attr->flags & RDMA_CREATE_AH_SLEEPABLE);

	return err;
}

/**
 * irdma_query_ah - Query address handle
 * @ibah: pointer to address handle
 * @ah_attr: address handle attributes
 */
static int irdma_query_ah(struct ib_ah *ibah, struct rdma_ah_attr *ah_attr)
{
	struct irdma_ah *ah = to_iwah(ibah);

	memset(ah_attr, 0, sizeof(*ah_attr));
	if (ah->av.attrs.ah_flags & IB_AH_GRH) {
		ah_attr->ah_flags = IB_AH_GRH;
		ah_attr->grh.flow_label = ah->sc_ah.ah_info.flow_label;
		ah_attr->grh.traffic_class = ah->sc_ah.ah_info.tc_tos;
		ah_attr->grh.hop_limit = ah->sc_ah.ah_info.hop_ttl;
		ah_attr->grh.sgid_index = ah->sgid_index;
		memcpy(&ah_attr->grh.dgid, &ah->dgid,
		       sizeof(ah_attr->grh.dgid));
	}

	return 0;
}

static enum rdma_link_layer irdma_get_link_layer(struct ib_device *ibdev,
						 u32 port_num)
{
	return IB_LINK_LAYER_ETHERNET;
}

static const struct ib_device_ops irdma_roce_dev_ops = {
	.attach_mcast = irdma_attach_mcast,
	.create_ah = irdma_create_ah,
	.create_user_ah = irdma_create_user_ah,
	.destroy_ah = irdma_destroy_ah,
	.detach_mcast = irdma_detach_mcast,
	.get_link_layer = irdma_get_link_layer,
	.get_port_immutable = irdma_roce_port_immutable,
	.modify_qp = irdma_modify_qp_roce,
	.query_ah = irdma_query_ah,
	.query_pkey = irdma_query_pkey,
};

static const struct ib_device_ops irdma_iw_dev_ops = {
	.get_port_immutable = irdma_iw_port_immutable,
	.iw_accept = irdma_accept,
	.iw_add_ref = irdma_qp_add_ref,
	.iw_connect = irdma_connect,
	.iw_create_listen = irdma_create_listen,
	.iw_destroy_listen = irdma_destroy_listen,
	.iw_get_qp = irdma_get_qp,
	.iw_reject = irdma_reject,
	.iw_rem_ref = irdma_qp_rem_ref,
	.modify_qp = irdma_modify_qp,
	.query_gid = irdma_query_gid,
};

static const struct ib_device_ops irdma_dev_ops = {
	.owner = THIS_MODULE,
	.driver_id = RDMA_DRIVER_IRDMA,
	.uverbs_abi_ver = IRDMA_ABI_VER,

	.alloc_hw_port_stats = irdma_alloc_hw_port_stats,
	.alloc_mr = irdma_alloc_mr,
	.alloc_mw = irdma_alloc_mw,
	.alloc_pd = irdma_alloc_pd,
	.alloc_ucontext = irdma_alloc_ucontext,
	.create_cq = irdma_create_cq,
	.create_qp = irdma_create_qp,
	.dealloc_driver = irdma_ib_dealloc_device,
	.dealloc_mw = irdma_dealloc_mw,
	.dealloc_pd = irdma_dealloc_pd,
	.dealloc_ucontext = irdma_dealloc_ucontext,
	.dereg_mr = irdma_dereg_mr,
	.destroy_cq = irdma_destroy_cq,
	.destroy_qp = irdma_destroy_qp,
	.disassociate_ucontext = irdma_disassociate_ucontext,
	.get_dev_fw_str = irdma_get_dev_fw_str,
	.get_dma_mr = irdma_get_dma_mr,
	.get_hw_stats = irdma_get_hw_stats,
	.map_mr_sg = irdma_map_mr_sg,
	.mmap = irdma_mmap,
	.mmap_free = irdma_mmap_free,
	.poll_cq = irdma_poll_cq,
	.post_recv = irdma_post_recv,
	.post_send = irdma_post_send,
	.query_device = irdma_query_device,
	.query_port = irdma_query_port,
	.query_qp = irdma_query_qp,
	.reg_user_mr = irdma_reg_user_mr,
	.reg_user_mr_dmabuf = irdma_reg_user_mr_dmabuf,
	.req_notify_cq = irdma_req_notify_cq,
	.resize_cq = irdma_resize_cq,
	INIT_RDMA_OBJ_SIZE(ib_pd, irdma_pd, ibpd),
	INIT_RDMA_OBJ_SIZE(ib_ucontext, irdma_ucontext, ibucontext),
	INIT_RDMA_OBJ_SIZE(ib_ah, irdma_ah, ibah),
	INIT_RDMA_OBJ_SIZE(ib_cq, irdma_cq, ibcq),
	INIT_RDMA_OBJ_SIZE(ib_mw, irdma_mr, ibmw),
	INIT_RDMA_OBJ_SIZE(ib_qp, irdma_qp, ibqp),
};

/**
 * irdma_init_roce_device - initialization of roce rdma device
 * @iwdev: irdma device
 */
static void irdma_init_roce_device(struct irdma_device *iwdev)
{
	iwdev->ibdev.node_type = RDMA_NODE_IB_CA;
	addrconf_addr_eui48((u8 *)&iwdev->ibdev.node_guid,
			    iwdev->netdev->dev_addr);
	ib_set_device_ops(&iwdev->ibdev, &irdma_roce_dev_ops);
}

/**
 * irdma_init_iw_device - initialization of iwarp rdma device
 * @iwdev: irdma device
 */
static void irdma_init_iw_device(struct irdma_device *iwdev)
{
	struct net_device *netdev = iwdev->netdev;

	iwdev->ibdev.node_type = RDMA_NODE_RNIC;
	addrconf_addr_eui48((u8 *)&iwdev->ibdev.node_guid,
			    netdev->dev_addr);
	memcpy(iwdev->ibdev.iw_ifname, netdev->name,
	       sizeof(iwdev->ibdev.iw_ifname));
	ib_set_device_ops(&iwdev->ibdev, &irdma_iw_dev_ops);
}

/**
 * irdma_init_rdma_device - initialization of rdma device
 * @iwdev: irdma device
 */
static void irdma_init_rdma_device(struct irdma_device *iwdev)
{
	struct pci_dev *pcidev = iwdev->rf->pcidev;

	if (iwdev->roce_mode)
		irdma_init_roce_device(iwdev);
	else
		irdma_init_iw_device(iwdev);

	iwdev->ibdev.phys_port_cnt = 1;
	iwdev->ibdev.num_comp_vectors = iwdev->rf->ceqs_count;
	iwdev->ibdev.dev.parent = &pcidev->dev;
	ib_set_device_ops(&iwdev->ibdev, &irdma_dev_ops);
}

/**
 * irdma_port_ibevent - indicate port event
 * @iwdev: irdma device
 */
void irdma_port_ibevent(struct irdma_device *iwdev)
{
	struct ib_event event;

	event.device = &iwdev->ibdev;
	event.element.port_num = 1;
	event.event =
		iwdev->iw_status ? IB_EVENT_PORT_ACTIVE : IB_EVENT_PORT_ERR;
	ib_dispatch_event(&event);
}

/**
 * irdma_ib_unregister_device - unregister rdma device from IB
 * core
 * @iwdev: irdma device
 */
void irdma_ib_unregister_device(struct irdma_device *iwdev)
{
	iwdev->iw_status = 0;
	irdma_port_ibevent(iwdev);
	ib_unregister_device(&iwdev->ibdev);
}

/**
 * irdma_ib_register_device - register irdma device to IB core
 * @iwdev: irdma device
 */
int irdma_ib_register_device(struct irdma_device *iwdev)
{
	int ret;

	irdma_init_rdma_device(iwdev);

	ret = ib_device_set_netdev(&iwdev->ibdev, iwdev->netdev, 1);
	if (ret)
		goto error;
	dma_set_max_seg_size(iwdev->rf->hw.device, UINT_MAX);
	ret = ib_register_device(&iwdev->ibdev, "irdma%d", iwdev->rf->hw.device);
	if (ret)
		goto error;

	iwdev->iw_status = 1;
	irdma_port_ibevent(iwdev);

	return 0;

error:
	if (ret)
		ibdev_dbg(&iwdev->ibdev, "VERBS: Register RDMA device fail\n");

	return ret;
}

/**
 * irdma_ib_dealloc_device
 * @ibdev: ib device
 *
 * callback from ibdev dealloc_driver to deallocate resources
 * unber irdma device
 */
void irdma_ib_dealloc_device(struct ib_device *ibdev)
{
	struct irdma_device *iwdev = to_iwdev(ibdev);

	irdma_rt_deinit_hw(iwdev);
	irdma_ctrl_deinit_hw(iwdev->rf);
	kfree(iwdev->rf);
}
{
	struct irdma_pbl *iwpbl = &iwmr->iwpbl;
	struct irdma_reg_ns_stag_info *stag_info;
	struct irdma_pd *iwpd = to_iwpd(iwmr->ibmr.pd);
	struct irdma_pble_alloc *palloc = &iwpbl->pble_alloc;
	struct irdma_cqp_request *cqp_request;
	struct cqp_cmds_info *cqp_info;
	int ret;

	cqp_request = irdma_alloc_and_get_cqp_request(&iwdev->rf->cqp, true);
	if (!cqp_request)
		return -ENOMEM;

	cqp_info = &cqp_request->info;
	stag_info = &cqp_info->in.u.mr_reg_non_shared.info;
	memset(stag_info, 0, sizeof(*stag_info));
	stag_info->va = iwpbl->user_base;
	stag_info->stag_idx = iwmr->stag >> IRDMA_CQPSQ_STAG_IDX_S;
	stag_info->stag_key = (u8)iwmr->stag;
	stag_info->total_len = iwmr->len;
	stag_info->access_rights = irdma_get_mr_access(access);
	stag_info->pd_id = iwpd->sc_pd.pd_id;
	if (stag_info->access_rights & IRDMA_ACCESS_FLAGS_ZERO_BASED)
		stag_info->addr_type = IRDMA_ADDR_TYPE_ZERO_BASED;
	else
		stag_info->addr_type = IRDMA_ADDR_TYPE_VA_BASED;
	stag_info->page_size = iwmr->page_size;

	if (iwpbl->pbl_allocated) {
		if (palloc->level == PBLE_LEVEL_1) {
			stag_info->first_pm_pbl_index = palloc->level1.idx;
			stag_info->chunk_size = 1;
		} else {
			stag_info->first_pm_pbl_index = palloc->level2.root.idx;
			stag_info->chunk_size = 3;
		}
	} else {
		stag_info->reg_addr_pa = iwmr->pgaddrmem[0];
	}

	cqp_info->cqp_cmd = IRDMA_OP_MR_REG_NON_SHARED;
	cqp_info->post_sq = 1;
	cqp_info->in.u.mr_reg_non_shared.dev = &iwdev->rf->sc_dev;
	cqp_info->in.u.mr_reg_non_shared.scratch = (uintptr_t)cqp_request;
	ret = irdma_handle_cqp_op(iwdev->rf, cqp_request);
	irdma_put_cqp_request(&iwdev->rf->cqp, cqp_request);

	return ret;
}

static int irdma_reg_user_mr_type_mem(struct irdma_mr *iwmr, int access)
{
	struct irdma_device *iwdev = to_iwdev(iwmr->ibmr.device);
	struct irdma_pbl *iwpbl = &iwmr->iwpbl;
	u32 stag;
	u8 lvl;
	int err;

	lvl = iwmr->page_cnt != 1 ? PBLE_LEVEL_1 | PBLE_LEVEL_2 : PBLE_LEVEL_0;

	err = irdma_setup_pbles(iwdev->rf, iwmr, lvl);
	if (err)
		return err;

	if (lvl) {
		err = irdma_check_mr_contiguous(&iwpbl->pble_alloc,
						iwmr->page_size);
		if (err) {
			irdma_free_pble(iwdev->rf->pble_rsrc, &iwpbl->pble_alloc);
			iwpbl->pbl_allocated = false;
		}
	}

	stag = irdma_create_stag(iwdev);
	if (!stag) {
		err = -ENOMEM;
		goto free_pble;
	}

	iwmr->stag = stag;
	iwmr->ibmr.rkey = stag;
	iwmr->ibmr.lkey = stag;
	err = irdma_hwreg_mr(iwdev, iwmr, access);
	if (err)
		goto err_hwreg;

	return 0;

err_hwreg:
	irdma_free_stag(iwdev, stag);

free_pble:
	if (iwpbl->pble_alloc.level != PBLE_LEVEL_0 && iwpbl->pbl_allocated)
		irdma_free_pble(iwdev->rf->pble_rsrc, &iwpbl->pble_alloc);

	return err;
}

static struct irdma_mr *irdma_alloc_iwmr(struct ib_umem *region,
					 struct ib_pd *pd, u64 virt,
					 enum irdma_memreg_type reg_type)
{
	struct irdma_device *iwdev = to_iwdev(pd->device);
	struct irdma_pbl *iwpbl;
	struct irdma_mr *iwmr;
	unsigned long pgsz_bitmap;

	iwmr = kzalloc(sizeof(*iwmr), GFP_KERNEL);
	if (!iwmr)
		return ERR_PTR(-ENOMEM);

	iwpbl = &iwmr->iwpbl;
	iwpbl->iwmr = iwmr;
	iwmr->region = region;
	iwmr->ibmr.pd = pd;
	iwmr->ibmr.device = pd->device;
	iwmr->ibmr.iova = virt;
	iwmr->type = reg_type;

	pgsz_bitmap = (reg_type == IRDMA_MEMREG_TYPE_MEM) ?
		iwdev->rf->sc_dev.hw_attrs.page_size_cap : PAGE_SIZE;

	iwmr->page_size = ib_umem_find_best_pgsz(region, pgsz_bitmap, virt);
	if (unlikely(!iwmr->page_size)) {
		kfree(iwmr);
		return ERR_PTR(-EOPNOTSUPP);
	}

	iwmr->len = region->length;
	iwpbl->user_base = virt;
	iwmr->page_cnt = ib_umem_num_dma_blocks(region, iwmr->page_size);

	return iwmr;
}

static void irdma_free_iwmr(struct irdma_mr *iwmr)
{
	kfree(iwmr);
}

static int irdma_reg_user_mr_type_qp(struct irdma_mem_reg_req req,
				     struct ib_udata *udata,
				     struct irdma_mr *iwmr)
{
	struct irdma_device *iwdev = to_iwdev(iwmr->ibmr.device);
	struct irdma_pbl *iwpbl = &iwmr->iwpbl;
	struct irdma_ucontext *ucontext = NULL;
	unsigned long flags;
	u32 total;
	int err;
	u8 lvl;

	total = req.sq_pages + req.rq_pages + 1;
	if (total > iwmr->page_cnt)
		return -EINVAL;

	total = req.sq_pages + req.rq_pages;
	lvl = total > 2 ? PBLE_LEVEL_1 : PBLE_LEVEL_0;
	err = irdma_handle_q_mem(iwdev, &req, iwpbl, lvl);
	if (err)
		return err;

	ucontext = rdma_udata_to_drv_context(udata, struct irdma_ucontext,
					     ibucontext);
	spin_lock_irqsave(&ucontext->qp_reg_mem_list_lock, flags);
	list_add_tail(&iwpbl->list, &ucontext->qp_reg_mem_list);
	iwpbl->on_list = true;
	spin_unlock_irqrestore(&ucontext->qp_reg_mem_list_lock, flags);

	return 0;
}

static int irdma_reg_user_mr_type_cq(struct irdma_mem_reg_req req,
				     struct ib_udata *udata,
				     struct irdma_mr *iwmr)
{
	struct irdma_device *iwdev = to_iwdev(iwmr->ibmr.device);
	struct irdma_pbl *iwpbl = &iwmr->iwpbl;
	struct irdma_ucontext *ucontext = NULL;
	u8 shadow_pgcnt = 1;
	unsigned long flags;
	u32 total;
	int err;
	u8 lvl;

	if (iwdev->rf->sc_dev.hw_attrs.uk_attrs.feature_flags & IRDMA_FEATURE_CQ_RESIZE)
		shadow_pgcnt = 0;
	total = req.cq_pages + shadow_pgcnt;
	if (total > iwmr->page_cnt)
		return -EINVAL;

	lvl = req.cq_pages > 1 ? PBLE_LEVEL_1 : PBLE_LEVEL_0;
	err = irdma_handle_q_mem(iwdev, &req, iwpbl, lvl);
	if (err)
		return err;

	ucontext = rdma_udata_to_drv_context(udata, struct irdma_ucontext,
					     ibucontext);
	spin_lock_irqsave(&ucontext->cq_reg_mem_list_lock, flags);
	list_add_tail(&iwpbl->list, &ucontext->cq_reg_mem_list);
	iwpbl->on_list = true;
	spin_unlock_irqrestore(&ucontext->cq_reg_mem_list_lock, flags);

	return 0;
}

/**
 * irdma_reg_user_mr - Register a user memory region
 * @pd: ptr of pd
 * @start: virtual start address
 * @len: length of mr
 * @virt: virtual address
 * @access: access of mr
 * @udata: user data
 */
static struct ib_mr *irdma_reg_user_mr(struct ib_pd *pd, u64 start, u64 len,
				       u64 virt, int access,
				       struct ib_udata *udata)
{
#define IRDMA_MEM_REG_MIN_REQ_LEN offsetofend(struct irdma_mem_reg_req, sq_pages)
	struct irdma_device *iwdev = to_iwdev(pd->device);
	struct irdma_mem_reg_req req = {};
	struct ib_umem *region = NULL;
	struct irdma_mr *iwmr = NULL;
	int err;

	if (len > iwdev->rf->sc_dev.hw_attrs.max_mr_size)
		return ERR_PTR(-EINVAL);

	if (udata->inlen < IRDMA_MEM_REG_MIN_REQ_LEN)
		return ERR_PTR(-EINVAL);

	region = ib_umem_get(pd->device, start, len, access);

	if (IS_ERR(region)) {
		ibdev_dbg(&iwdev->ibdev,
			  "VERBS: Failed to create ib_umem region\n");
		return (struct ib_mr *)region;
	}

	if (ib_copy_from_udata(&req, udata, min(sizeof(req), udata->inlen))) {
		ib_umem_release(region);
		return ERR_PTR(-EFAULT);
	}

	iwmr = irdma_alloc_iwmr(region, pd, virt, req.reg_type);
	if (IS_ERR(iwmr)) {
		ib_umem_release(region);
		return (struct ib_mr *)iwmr;
	}

	switch (req.reg_type) {
	case IRDMA_MEMREG_TYPE_QP:
		err = irdma_reg_user_mr_type_qp(req, udata, iwmr);
		if (err)
			goto error;

		break;
	case IRDMA_MEMREG_TYPE_CQ:
		err = irdma_reg_user_mr_type_cq(req, udata, iwmr);
		if (err)
			goto error;
		break;
	case IRDMA_MEMREG_TYPE_MEM:
		err = irdma_reg_user_mr_type_mem(iwmr, access);
		if (err)
			goto error;

		break;
	default:
		err = -EINVAL;
		goto error;
	}

	return &iwmr->ibmr;
error:
	ib_umem_release(region);
	irdma_free_iwmr(iwmr);

	return ERR_PTR(err);
}

static struct ib_mr *irdma_reg_user_mr_dmabuf(struct ib_pd *pd, u64 start,
					      u64 len, u64 virt,
					      int fd, int access,
					      struct ib_udata *udata)
{
	struct irdma_device *iwdev = to_iwdev(pd->device);
	struct ib_umem_dmabuf *umem_dmabuf;
	struct irdma_mr *iwmr;
	int err;

	if (len > iwdev->rf->sc_dev.hw_attrs.max_mr_size)
		return ERR_PTR(-EINVAL);

	umem_dmabuf = ib_umem_dmabuf_get_pinned(pd->device, start, len, fd, access);
	if (IS_ERR(umem_dmabuf)) {
		err = PTR_ERR(umem_dmabuf);
		ibdev_dbg(&iwdev->ibdev, "Failed to get dmabuf umem[%d]\n", err);
		return ERR_PTR(err);
	}

	iwmr = irdma_alloc_iwmr(&umem_dmabuf->umem, pd, virt, IRDMA_MEMREG_TYPE_MEM);
	if (IS_ERR(iwmr)) {
		err = PTR_ERR(iwmr);
		goto err_release;
	}

	err = irdma_reg_user_mr_type_mem(iwmr, access);
	if (err)
		goto err_iwmr;

	return &iwmr->ibmr;

err_iwmr:
	irdma_free_iwmr(iwmr);

err_release:
	ib_umem_release(&umem_dmabuf->umem);

	return ERR_PTR(err);
}

/**
 * irdma_reg_phys_mr - register kernel physical memory
 * @pd: ibpd pointer
 * @addr: physical address of memory to register
 * @size: size of memory to register
 * @access: Access rights
 * @iova_start: start of virtual address for physical buffers
 */
struct ib_mr *irdma_reg_phys_mr(struct ib_pd *pd, u64 addr, u64 size, int access,
				u64 *iova_start)
{
	struct irdma_device *iwdev = to_iwdev(pd->device);
	struct irdma_pbl *iwpbl;
	struct irdma_mr *iwmr;
	u32 stag;
	int ret;

	iwmr = kzalloc(sizeof(*iwmr), GFP_KERNEL);
	if (!iwmr)
		return ERR_PTR(-ENOMEM);

	iwmr->ibmr.pd = pd;
	iwmr->ibmr.device = pd->device;
	iwpbl = &iwmr->iwpbl;
	iwpbl->iwmr = iwmr;
	iwmr->type = IRDMA_MEMREG_TYPE_MEM;
	iwpbl->user_base = *iova_start;
	stag = irdma_create_stag(iwdev);
	if (!stag) {
		ret = -ENOMEM;
		goto err;
	}

	iwmr->stag = stag;
	iwmr->ibmr.iova = *iova_start;
	iwmr->ibmr.rkey = stag;
	iwmr->ibmr.lkey = stag;
	iwmr->page_cnt = 1;
	iwmr->pgaddrmem[0] = addr;
	iwmr->len = size;
	iwmr->page_size = SZ_4K;
	ret = irdma_hwreg_mr(iwdev, iwmr, access);
	if (ret) {
		irdma_free_stag(iwdev, stag);
		goto err;
	}

	return &iwmr->ibmr;

err:
	kfree(iwmr);

	return ERR_PTR(ret);
}

/**
 * irdma_get_dma_mr - register physical mem
 * @pd: ptr of pd
 * @acc: access for memory
 */
static struct ib_mr *irdma_get_dma_mr(struct ib_pd *pd, int acc)
{
	u64 kva = 0;

	return irdma_reg_phys_mr(pd, 0, 0, acc, &kva);
}

/**
 * irdma_del_memlist - Deleting pbl list entries for CQ/QP
 * @iwmr: iwmr for IB's user page addresses
 * @ucontext: ptr to user context
 */
static void irdma_del_memlist(struct irdma_mr *iwmr,
			      struct irdma_ucontext *ucontext)
{
	struct irdma_pbl *iwpbl = &iwmr->iwpbl;
	unsigned long flags;

	switch (iwmr->type) {
	case IRDMA_MEMREG_TYPE_CQ:
		spin_lock_irqsave(&ucontext->cq_reg_mem_list_lock, flags);
		if (iwpbl->on_list) {
			iwpbl->on_list = false;
			list_del(&iwpbl->list);
		}
		spin_unlock_irqrestore(&ucontext->cq_reg_mem_list_lock, flags);
		break;
	case IRDMA_MEMREG_TYPE_QP:
		spin_lock_irqsave(&ucontext->qp_reg_mem_list_lock, flags);
		if (iwpbl->on_list) {
			iwpbl->on_list = false;
			list_del(&iwpbl->list);
		}
		spin_unlock_irqrestore(&ucontext->qp_reg_mem_list_lock, flags);
		break;
	default:
		break;
	}
}

/**
 * irdma_dereg_mr - deregister mr
 * @ib_mr: mr ptr for dereg
 * @udata: user data
 */
static int irdma_dereg_mr(struct ib_mr *ib_mr, struct ib_udata *udata)
{
	struct ib_pd *ibpd = ib_mr->pd;
	struct irdma_pd *iwpd = to_iwpd(ibpd);
	struct irdma_mr *iwmr = to_iwmr(ib_mr);
	struct irdma_device *iwdev = to_iwdev(ib_mr->device);
	struct irdma_dealloc_stag_info *info;
	struct irdma_pbl *iwpbl = &iwmr->iwpbl;
	struct irdma_pble_alloc *palloc = &iwpbl->pble_alloc;
	struct irdma_cqp_request *cqp_request;
	struct cqp_cmds_info *cqp_info;
	int status;

	if (iwmr->type != IRDMA_MEMREG_TYPE_MEM) {
		if (iwmr->region) {
			struct irdma_ucontext *ucontext;

			ucontext = rdma_udata_to_drv_context(udata,
						struct irdma_ucontext,
						ibucontext);
			irdma_del_memlist(iwmr, ucontext);
		}
		goto done;
	}

	cqp_request = irdma_alloc_and_get_cqp_request(&iwdev->rf->cqp, true);
	if (!cqp_request)
		return -ENOMEM;

	cqp_info = &cqp_request->info;
	info = &cqp_info->in.u.dealloc_stag.info;
	memset(info, 0, sizeof(*info));
	info->pd_id = iwpd->sc_pd.pd_id;
	info->stag_idx = ib_mr->rkey >> IRDMA_CQPSQ_STAG_IDX_S;
	info->mr = true;
	if (iwpbl->pbl_allocated)
		info->dealloc_pbl = true;

	cqp_info->cqp_cmd = IRDMA_OP_DEALLOC_STAG;
	cqp_info->post_sq = 1;
	cqp_info->in.u.dealloc_stag.dev = &iwdev->rf->sc_dev;
	cqp_info->in.u.dealloc_stag.scratch = (uintptr_t)cqp_request;
	status = irdma_handle_cqp_op(iwdev->rf, cqp_request);
	irdma_put_cqp_request(&iwdev->rf->cqp, cqp_request);
	if (status)
		return status;

	irdma_free_stag(iwdev, iwmr->stag);
done:
	if (iwpbl->pbl_allocated)
		irdma_free_pble(iwdev->rf->pble_rsrc, palloc);
	ib_umem_release(iwmr->region);
	kfree(iwmr);

	return 0;
}

/**
 * irdma_post_send -  kernel application wr
 * @ibqp: qp ptr for wr
 * @ib_wr: work request ptr
 * @bad_wr: return of bad wr if err
 */
static int irdma_post_send(struct ib_qp *ibqp,
			   const struct ib_send_wr *ib_wr,
			   const struct ib_send_wr **bad_wr)
{
	struct irdma_qp *iwqp;
	struct irdma_qp_uk *ukqp;
	struct irdma_sc_dev *dev;
	struct irdma_post_sq_info info;
	int err = 0;
	unsigned long flags;
	bool inv_stag;
	struct irdma_ah *ah;

	iwqp = to_iwqp(ibqp);
	ukqp = &iwqp->sc_qp.qp_uk;
	dev = &iwqp->iwdev->rf->sc_dev;

	spin_lock_irqsave(&iwqp->lock, flags);
	while (ib_wr) {
		memset(&info, 0, sizeof(info));
		inv_stag = false;
		info.wr_id = (ib_wr->wr_id);
		if ((ib_wr->send_flags & IB_SEND_SIGNALED) || iwqp->sig_all)
			info.signaled = true;
		if (ib_wr->send_flags & IB_SEND_FENCE)
			info.read_fence = true;
		switch (ib_wr->opcode) {
		case IB_WR_SEND_WITH_IMM:
			if (ukqp->qp_caps & IRDMA_SEND_WITH_IMM) {
				info.imm_data_valid = true;
				info.imm_data = ntohl(ib_wr->ex.imm_data);
			} else {
				err = -EINVAL;
				break;
			}
			fallthrough;
		case IB_WR_SEND:
		case IB_WR_SEND_WITH_INV:
			if (ib_wr->opcode == IB_WR_SEND ||
			    ib_wr->opcode == IB_WR_SEND_WITH_IMM) {
				if (ib_wr->send_flags & IB_SEND_SOLICITED)
					info.op_type = IRDMA_OP_TYPE_SEND_SOL;
				else
					info.op_type = IRDMA_OP_TYPE_SEND;
			} else {
				if (ib_wr->send_flags & IB_SEND_SOLICITED)
					info.op_type = IRDMA_OP_TYPE_SEND_SOL_INV;
				else
					info.op_type = IRDMA_OP_TYPE_SEND_INV;
				info.stag_to_inv = ib_wr->ex.invalidate_rkey;
			}

			info.op.send.num_sges = ib_wr->num_sge;
			info.op.send.sg_list = ib_wr->sg_list;
			if (iwqp->ibqp.qp_type == IB_QPT_UD ||
			    iwqp->ibqp.qp_type == IB_QPT_GSI) {
				ah = to_iwah(ud_wr(ib_wr)->ah);
				info.op.send.ah_id = ah->sc_ah.ah_info.ah_idx;
				info.op.send.qkey = ud_wr(ib_wr)->remote_qkey;
				info.op.send.dest_qp = ud_wr(ib_wr)->remote_qpn;
			}

			if (ib_wr->send_flags & IB_SEND_INLINE)
				err = irdma_uk_inline_send(ukqp, &info, false);
			else
				err = irdma_uk_send(ukqp, &info, false);
			break;
		case IB_WR_RDMA_WRITE_WITH_IMM:
			if (ukqp->qp_caps & IRDMA_WRITE_WITH_IMM) {
				info.imm_data_valid = true;
				info.imm_data = ntohl(ib_wr->ex.imm_data);
			} else {
				err = -EINVAL;
				break;
			}
			fallthrough;
		case IB_WR_RDMA_WRITE:
			if (ib_wr->send_flags & IB_SEND_SOLICITED)
				info.op_type = IRDMA_OP_TYPE_RDMA_WRITE_SOL;
			else
				info.op_type = IRDMA_OP_TYPE_RDMA_WRITE;

			info.op.rdma_write.num_lo_sges = ib_wr->num_sge;
			info.op.rdma_write.lo_sg_list = ib_wr->sg_list;
			info.op.rdma_write.rem_addr.addr =
				rdma_wr(ib_wr)->remote_addr;
			info.op.rdma_write.rem_addr.lkey = rdma_wr(ib_wr)->rkey;
			if (ib_wr->send_flags & IB_SEND_INLINE)
				err = irdma_uk_inline_rdma_write(ukqp, &info, false);
			else
				err = irdma_uk_rdma_write(ukqp, &info, false);
			break;
		case IB_WR_RDMA_READ_WITH_INV:
			inv_stag = true;
			fallthrough;
		case IB_WR_RDMA_READ:
			if (ib_wr->num_sge >
			    dev->hw_attrs.uk_attrs.max_hw_read_sges) {
				err = -EINVAL;
				break;
			}
			info.op_type = IRDMA_OP_TYPE_RDMA_READ;
			info.op.rdma_read.rem_addr.addr = rdma_wr(ib_wr)->remote_addr;
			info.op.rdma_read.rem_addr.lkey = rdma_wr(ib_wr)->rkey;
			info.op.rdma_read.lo_sg_list = (void *)ib_wr->sg_list;
			info.op.rdma_read.num_lo_sges = ib_wr->num_sge;
			err = irdma_uk_rdma_read(ukqp, &info, inv_stag, false);
			break;
		case IB_WR_LOCAL_INV:
			info.op_type = IRDMA_OP_TYPE_INV_STAG;
			info.local_fence = info.read_fence;
			info.op.inv_local_stag.target_stag = ib_wr->ex.invalidate_rkey;
			err = irdma_uk_stag_local_invalidate(ukqp, &info, true);
			break;
		case IB_WR_REG_MR: {
			struct irdma_mr *iwmr = to_iwmr(reg_wr(ib_wr)->mr);
			struct irdma_pble_alloc *palloc = &iwmr->iwpbl.pble_alloc;
			struct irdma_fast_reg_stag_info stag_info = {};

			stag_info.signaled = info.signaled;
			stag_info.read_fence = info.read_fence;
			stag_info.access_rights = irdma_get_mr_access(reg_wr(ib_wr)->access);
			stag_info.stag_key = reg_wr(ib_wr)->key & 0xff;
			stag_info.stag_idx = reg_wr(ib_wr)->key >> 8;
			stag_info.page_size = reg_wr(ib_wr)->mr->page_size;
			stag_info.wr_id = ib_wr->wr_id;
			stag_info.addr_type = IRDMA_ADDR_TYPE_VA_BASED;
			stag_info.va = (void *)(uintptr_t)iwmr->ibmr.iova;
			stag_info.total_len = iwmr->ibmr.length;
			stag_info.reg_addr_pa = *palloc->level1.addr;
			stag_info.first_pm_pbl_index = palloc->level1.idx;
			stag_info.local_fence = ib_wr->send_flags & IB_SEND_FENCE;
			if (iwmr->npages > IRDMA_MIN_PAGES_PER_FMR)
				stag_info.chunk_size = 1;
			err = irdma_sc_mr_fast_register(&iwqp->sc_qp, &stag_info,
							true);
			break;
		}
		default:
			err = -EINVAL;
			ibdev_dbg(&iwqp->iwdev->ibdev,
				  "VERBS: upost_send bad opcode = 0x%x\n",
				  ib_wr->opcode);
			break;
		}

		if (err)
			break;
		ib_wr = ib_wr->next;
	}

	if (!iwqp->flush_issued) {
		if (iwqp->hw_iwarp_state <= IRDMA_QP_STATE_RTS)
			irdma_uk_qp_post_wr(ukqp);
		spin_unlock_irqrestore(&iwqp->lock, flags);
	} else {
		spin_unlock_irqrestore(&iwqp->lock, flags);
		mod_delayed_work(iwqp->iwdev->cleanup_wq, &iwqp->dwork_flush,
				 msecs_to_jiffies(IRDMA_FLUSH_DELAY_MS));
	}
	if (err)
		*bad_wr = ib_wr;

	return err;
}

/**
 * irdma_post_recv - post receive wr for kernel application
 * @ibqp: ib qp pointer
 * @ib_wr: work request for receive
 * @bad_wr: bad wr caused an error
 */
static int irdma_post_recv(struct ib_qp *ibqp,
			   const struct ib_recv_wr *ib_wr,
			   const struct ib_recv_wr **bad_wr)
{
	struct irdma_qp *iwqp;
	struct irdma_qp_uk *ukqp;
	struct irdma_post_rq_info post_recv = {};
	unsigned long flags;
	int err = 0;

	iwqp = to_iwqp(ibqp);
	ukqp = &iwqp->sc_qp.qp_uk;

	spin_lock_irqsave(&iwqp->lock, flags);
	while (ib_wr) {
		post_recv.num_sges = ib_wr->num_sge;
		post_recv.wr_id = ib_wr->wr_id;
		post_recv.sg_list = ib_wr->sg_list;
		err = irdma_uk_post_receive(ukqp, &post_recv);
		if (err) {
			ibdev_dbg(&iwqp->iwdev->ibdev,
				  "VERBS: post_recv err %d\n", err);
			goto out;
		}

		ib_wr = ib_wr->next;
	}

out:
	spin_unlock_irqrestore(&iwqp->lock, flags);
	if (iwqp->flush_issued)
		mod_delayed_work(iwqp->iwdev->cleanup_wq, &iwqp->dwork_flush,
				 msecs_to_jiffies(IRDMA_FLUSH_DELAY_MS));

	if (err)
		*bad_wr = ib_wr;

	return err;
}

/**
 * irdma_flush_err_to_ib_wc_status - return change flush error code to IB status
 * @opcode: iwarp flush code
 */
static enum ib_wc_status irdma_flush_err_to_ib_wc_status(enum irdma_flush_opcode opcode)
{
	switch (opcode) {
	case FLUSH_PROT_ERR:
		return IB_WC_LOC_PROT_ERR;
	case FLUSH_REM_ACCESS_ERR:
		return IB_WC_REM_ACCESS_ERR;
	case FLUSH_LOC_QP_OP_ERR:
		return IB_WC_LOC_QP_OP_ERR;
	case FLUSH_REM_OP_ERR:
		return IB_WC_REM_OP_ERR;
	case FLUSH_LOC_LEN_ERR:
		return IB_WC_LOC_LEN_ERR;
	case FLUSH_GENERAL_ERR:
		return IB_WC_WR_FLUSH_ERR;
	case FLUSH_RETRY_EXC_ERR:
		return IB_WC_RETRY_EXC_ERR;
	case FLUSH_MW_BIND_ERR:
		return IB_WC_MW_BIND_ERR;
	case FLUSH_REM_INV_REQ_ERR:
		return IB_WC_REM_INV_REQ_ERR;
	case FLUSH_FATAL_ERR:
	default:
		return IB_WC_FATAL_ERR;
	}
}

/**
 * irdma_process_cqe - process cqe info
 * @entry: processed cqe
 * @cq_poll_info: cqe info
 */
static void irdma_process_cqe(struct ib_wc *entry,
			      struct irdma_cq_poll_info *cq_poll_info)
{
	struct irdma_sc_qp *qp;

	entry->wc_flags = 0;
	entry->pkey_index = 0;
	entry->wr_id = cq_poll_info->wr_id;

	qp = cq_poll_info->qp_handle;
	entry->qp = qp->qp_uk.back_qp;

	if (cq_poll_info->error) {
		entry->status = (cq_poll_info->comp_status == IRDMA_COMPL_STATUS_FLUSHED) ?
				irdma_flush_err_to_ib_wc_status(cq_poll_info->minor_err) : IB_WC_GENERAL_ERR;

		entry->vendor_err = cq_poll_info->major_err << 16 |
				    cq_poll_info->minor_err;
	} else {
		entry->status = IB_WC_SUCCESS;
		if (cq_poll_info->imm_valid) {
			entry->ex.imm_data = htonl(cq_poll_info->imm_data);
			entry->wc_flags |= IB_WC_WITH_IMM;
		}
		if (cq_poll_info->ud_smac_valid) {
			ether_addr_copy(entry->smac, cq_poll_info->ud_smac);
			entry->wc_flags |= IB_WC_WITH_SMAC;
		}

		if (cq_poll_info->ud_vlan_valid) {
			u16 vlan = cq_poll_info->ud_vlan & VLAN_VID_MASK;

			entry->sl = cq_poll_info->ud_vlan >> VLAN_PRIO_SHIFT;
			if (vlan) {
				entry->vlan_id = vlan;
				entry->wc_flags |= IB_WC_WITH_VLAN;
			}
		} else {
			entry->sl = 0;
		}
	}

	if (cq_poll_info->q_type == IRDMA_CQE_QTYPE_SQ) {
		set_ib_wc_op_sq(cq_poll_info, entry);
	} else {
		set_ib_wc_op_rq(cq_poll_info, entry,
				qp->qp_uk.qp_caps & IRDMA_SEND_WITH_IMM);
		if (qp->qp_uk.qp_type != IRDMA_QP_TYPE_ROCE_UD &&
		    cq_poll_info->stag_invalid_set) {
			entry->ex.invalidate_rkey = cq_poll_info->inv_stag;
			entry->wc_flags |= IB_WC_WITH_INVALIDATE;
		}
	}

	if (qp->qp_uk.qp_type == IRDMA_QP_TYPE_ROCE_UD) {
		entry->src_qp = cq_poll_info->ud_src_qpn;
		entry->slid = 0;
		entry->wc_flags |=
			(IB_WC_GRH | IB_WC_WITH_NETWORK_HDR_TYPE);
		entry->network_hdr_type = cq_poll_info->ipv4 ?
						  RDMA_NETWORK_IPV4 :
						  RDMA_NETWORK_IPV6;
	} else {
		entry->src_qp = cq_poll_info->qp_id;
	}

	entry->byte_len = cq_poll_info->bytes_xfered;
}

/**
 * irdma_poll_one - poll one entry of the CQ
 * @ukcq: ukcq to poll
 * @cur_cqe: current CQE info to be filled in
 * @entry: ibv_wc object to be filled for non-extended CQ or NULL for extended CQ
 *
 * Returns the internal irdma device error code or 0 on success
 */
static inline int irdma_poll_one(struct irdma_cq_uk *ukcq,
				 struct irdma_cq_poll_info *cur_cqe,
				 struct ib_wc *entry)
{
	int ret = irdma_uk_cq_poll_cmpl(ukcq, cur_cqe);

	if (ret)
		return ret;

	irdma_process_cqe(entry, cur_cqe);

	return 0;
}

/**
 * __irdma_poll_cq - poll cq for completion (kernel apps)
 * @iwcq: cq to poll
 * @num_entries: number of entries to poll
 * @entry: wr of a completed entry
 */
static int __irdma_poll_cq(struct irdma_cq *iwcq, int num_entries, struct ib_wc *entry)
{
	struct list_head *tmp_node, *list_node;
	struct irdma_cq_buf *last_buf = NULL;
	struct irdma_cq_poll_info *cur_cqe = &iwcq->cur_cqe;
	struct irdma_cq_buf *cq_buf;
	int ret;
	struct irdma_device *iwdev;
	struct irdma_cq_uk *ukcq;
	bool cq_new_cqe = false;
	int resized_bufs = 0;
	int npolled = 0;

	iwdev = to_iwdev(iwcq->ibcq.device);
	ukcq = &iwcq->sc_cq.cq_uk;

	/* go through the list of previously resized CQ buffers */
	list_for_each_safe(list_node, tmp_node, &iwcq->resize_list) {
		cq_buf = container_of(list_node, struct irdma_cq_buf, list);
		while (npolled < num_entries) {
			ret = irdma_poll_one(&cq_buf->cq_uk, cur_cqe, entry + npolled);
			if (!ret) {
				++npolled;
				cq_new_cqe = true;
				continue;
			}
			if (ret == -ENOENT)
				break;
			 /* QP using the CQ is destroyed. Skip reporting this CQE */
			if (ret == -EFAULT) {
				cq_new_cqe = true;
				continue;
			}
			goto error;
		}

		/* save the resized CQ buffer which received the last cqe */
		if (cq_new_cqe)
			last_buf = cq_buf;
		cq_new_cqe = false;
	}

	/* check the current CQ for new cqes */
	while (npolled < num_entries) {
		ret = irdma_poll_one(ukcq, cur_cqe, entry + npolled);
		if (ret == -ENOENT) {
			ret = irdma_generated_cmpls(iwcq, cur_cqe);
			if (!ret)
				irdma_process_cqe(entry + npolled, cur_cqe);
		}
		if (!ret) {
			++npolled;
			cq_new_cqe = true;
			continue;
		}

		if (ret == -ENOENT)
			break;
		/* QP using the CQ is destroyed. Skip reporting this CQE */
		if (ret == -EFAULT) {
			cq_new_cqe = true;
			continue;
		}
		goto error;
	}

	if (cq_new_cqe)
		/* all previous CQ resizes are complete */
		resized_bufs = irdma_process_resize_list(iwcq, iwdev, NULL);
	else if (last_buf)
		/* only CQ resizes up to the last_buf are complete */
		resized_bufs = irdma_process_resize_list(iwcq, iwdev, last_buf);
	if (resized_bufs)
		/* report to the HW the number of complete CQ resizes */
		irdma_uk_cq_set_resized_cnt(ukcq, resized_bufs);

	return npolled;
error:
	ibdev_dbg(&iwdev->ibdev, "%s: Error polling CQ, irdma_err: %d\n",
		  __func__, ret);

	return ret;
}

/**
 * irdma_poll_cq - poll cq for completion (kernel apps)
 * @ibcq: cq to poll
 * @num_entries: number of entries to poll
 * @entry: wr of a completed entry
 */
static int irdma_poll_cq(struct ib_cq *ibcq, int num_entries,
			 struct ib_wc *entry)
{
	struct irdma_cq *iwcq;
	unsigned long flags;
	int ret;

	iwcq = to_iwcq(ibcq);

	spin_lock_irqsave(&iwcq->lock, flags);
	ret = __irdma_poll_cq(iwcq, num_entries, entry);
	spin_unlock_irqrestore(&iwcq->lock, flags);

	return ret;
}

/**
 * irdma_req_notify_cq - arm cq kernel application
 * @ibcq: cq to arm
 * @notify_flags: notofication flags
 */
static int irdma_req_notify_cq(struct ib_cq *ibcq,
			       enum ib_cq_notify_flags notify_flags)
{
	struct irdma_cq *iwcq;
	struct irdma_cq_uk *ukcq;
	unsigned long flags;
	enum irdma_cmpl_notify cq_notify;
	bool promo_event = false;
	int ret = 0;

	cq_notify = notify_flags == IB_CQ_SOLICITED ?
		    IRDMA_CQ_COMPL_SOLICITED : IRDMA_CQ_COMPL_EVENT;
	iwcq = to_iwcq(ibcq);
	ukcq = &iwcq->sc_cq.cq_uk;

	spin_lock_irqsave(&iwcq->lock, flags);
	/* Only promote to arm the CQ for any event if the last arm event was solicited. */
	if (iwcq->last_notify == IRDMA_CQ_COMPL_SOLICITED && notify_flags != IB_CQ_SOLICITED)
		promo_event = true;

	if (!atomic_cmpxchg(&iwcq->armed, 0, 1) || promo_event) {
		iwcq->last_notify = cq_notify;
		irdma_uk_cq_request_notification(ukcq, cq_notify);
	}

	if ((notify_flags & IB_CQ_REPORT_MISSED_EVENTS) &&
	    (!irdma_cq_empty(iwcq) || !list_empty(&iwcq->cmpl_generated)))
		ret = 1;
	spin_unlock_irqrestore(&iwcq->lock, flags);

	return ret;
}

static int irdma_roce_port_immutable(struct ib_device *ibdev, u32 port_num,
				     struct ib_port_immutable *immutable)
{
	struct ib_port_attr attr;
	int err;

	immutable->core_cap_flags = RDMA_CORE_PORT_IBA_ROCE_UDP_ENCAP;
	err = ib_query_port(ibdev, port_num, &attr);
	if (err)
		return err;

	immutable->max_mad_size = IB_MGMT_MAD_SIZE;
	immutable->pkey_tbl_len = attr.pkey_tbl_len;
	immutable->gid_tbl_len = attr.gid_tbl_len;

	return 0;
}

static int irdma_iw_port_immutable(struct ib_device *ibdev, u32 port_num,
				   struct ib_port_immutable *immutable)
{
	struct ib_port_attr attr;
	int err;

	immutable->core_cap_flags = RDMA_CORE_PORT_IWARP;
	err = ib_query_port(ibdev, port_num, &attr);
	if (err)
		return err;
	immutable->gid_tbl_len = attr.gid_tbl_len;

	return 0;
}

static const struct rdma_stat_desc irdma_hw_stat_names[] = {
	/* gen1 - 32-bit */
	[IRDMA_HW_STAT_INDEX_IP4RXDISCARD].name		= "ip4InDiscards",
	[IRDMA_HW_STAT_INDEX_IP4RXTRUNC].name		= "ip4InTruncatedPkts",
	[IRDMA_HW_STAT_INDEX_IP4TXNOROUTE].name		= "ip4OutNoRoutes",
	[IRDMA_HW_STAT_INDEX_IP6RXDISCARD].name		= "ip6InDiscards",
	[IRDMA_HW_STAT_INDEX_IP6RXTRUNC].name		= "ip6InTruncatedPkts",
	[IRDMA_HW_STAT_INDEX_IP6TXNOROUTE].name		= "ip6OutNoRoutes",
	[IRDMA_HW_STAT_INDEX_TCPRTXSEG].name		= "tcpRetransSegs",
	[IRDMA_HW_STAT_INDEX_TCPRXOPTERR].name		= "tcpInOptErrors",
	[IRDMA_HW_STAT_INDEX_TCPRXPROTOERR].name	= "tcpInProtoErrors",
	[IRDMA_HW_STAT_INDEX_RXVLANERR].name		= "rxVlanErrors",
	/* gen1 - 64-bit */
	[IRDMA_HW_STAT_INDEX_IP4RXOCTS].name		= "ip4InOctets",
	[IRDMA_HW_STAT_INDEX_IP4RXPKTS].name		= "ip4InPkts",
	[IRDMA_HW_STAT_INDEX_IP4RXFRAGS].name		= "ip4InReasmRqd",
	[IRDMA_HW_STAT_INDEX_IP4RXMCPKTS].name		= "ip4InMcastPkts",
	[IRDMA_HW_STAT_INDEX_IP4TXOCTS].name		= "ip4OutOctets",
	[IRDMA_HW_STAT_INDEX_IP4TXPKTS].name		= "ip4OutPkts",
	[IRDMA_HW_STAT_INDEX_IP4TXFRAGS].name		= "ip4OutSegRqd",
	[IRDMA_HW_STAT_INDEX_IP4TXMCPKTS].name		= "ip4OutMcastPkts",
	[IRDMA_HW_STAT_INDEX_IP6RXOCTS].name		= "ip6InOctets",
	[IRDMA_HW_STAT_INDEX_IP6RXPKTS].name		= "ip6InPkts",
	[IRDMA_HW_STAT_INDEX_IP6RXFRAGS].name		= "ip6InReasmRqd",
	[IRDMA_HW_STAT_INDEX_IP6RXMCPKTS].name		= "ip6InMcastPkts",
	[IRDMA_HW_STAT_INDEX_IP6TXOCTS].name		= "ip6OutOctets",
	[IRDMA_HW_STAT_INDEX_IP6TXPKTS].name		= "ip6OutPkts",
	[IRDMA_HW_STAT_INDEX_IP6TXFRAGS].name		= "ip6OutSegRqd",
	[IRDMA_HW_STAT_INDEX_IP6TXMCPKTS].name		= "ip6OutMcastPkts",
	[IRDMA_HW_STAT_INDEX_TCPRXSEGS].name		= "tcpInSegs",
	[IRDMA_HW_STAT_INDEX_TCPTXSEG].name		= "tcpOutSegs",
	[IRDMA_HW_STAT_INDEX_RDMARXRDS].name		= "iwInRdmaReads",
	[IRDMA_HW_STAT_INDEX_RDMARXSNDS].name		= "iwInRdmaSends",
	[IRDMA_HW_STAT_INDEX_RDMARXWRS].name		= "iwInRdmaWrites",
	[IRDMA_HW_STAT_INDEX_RDMATXRDS].name		= "iwOutRdmaReads",
	[IRDMA_HW_STAT_INDEX_RDMATXSNDS].name		= "iwOutRdmaSends",
	[IRDMA_HW_STAT_INDEX_RDMATXWRS].name		= "iwOutRdmaWrites",
	[IRDMA_HW_STAT_INDEX_RDMAVBND].name		= "iwRdmaBnd",
	[IRDMA_HW_STAT_INDEX_RDMAVINV].name		= "iwRdmaInv",

	/* gen2 - 32-bit */
	[IRDMA_HW_STAT_INDEX_RXRPCNPHANDLED].name	= "cnpHandled",
	[IRDMA_HW_STAT_INDEX_RXRPCNPIGNORED].name	= "cnpIgnored",
	[IRDMA_HW_STAT_INDEX_TXNPCNPSENT].name		= "cnpSent",
	/* gen2 - 64-bit */
	[IRDMA_HW_STAT_INDEX_IP4RXMCOCTS].name		= "ip4InMcastOctets",
	[IRDMA_HW_STAT_INDEX_IP4TXMCOCTS].name		= "ip4OutMcastOctets",
	[IRDMA_HW_STAT_INDEX_IP6RXMCOCTS].name		= "ip6InMcastOctets",
	[IRDMA_HW_STAT_INDEX_IP6TXMCOCTS].name		= "ip6OutMcastOctets",
	[IRDMA_HW_STAT_INDEX_UDPRXPKTS].name		= "RxUDP",
	[IRDMA_HW_STAT_INDEX_UDPTXPKTS].name		= "TxUDP",
	[IRDMA_HW_STAT_INDEX_RXNPECNMARKEDPKTS].name	= "RxECNMrkd",

};

static void irdma_get_dev_fw_str(struct ib_device *dev, char *str)
{
	struct irdma_device *iwdev = to_iwdev(dev);

	snprintf(str, IB_FW_VERSION_NAME_MAX, "%u.%u",
		 irdma_fw_major_ver(&iwdev->rf->sc_dev),
		 irdma_fw_minor_ver(&iwdev->rf->sc_dev));
}

/**
 * irdma_alloc_hw_port_stats - Allocate a hw stats structure
 * @ibdev: device pointer from stack
 * @port_num: port number
 */
static struct rdma_hw_stats *irdma_alloc_hw_port_stats(struct ib_device *ibdev,
						       u32 port_num)
{
	struct irdma_device *iwdev = to_iwdev(ibdev);
	struct irdma_sc_dev *dev = &iwdev->rf->sc_dev;

	int num_counters = dev->hw_attrs.max_stat_idx;
	unsigned long lifespan = RDMA_HW_STATS_DEFAULT_LIFESPAN;

	return rdma_alloc_hw_stats_struct(irdma_hw_stat_names, num_counters,
					  lifespan);
}

/**
 * irdma_get_hw_stats - Populates the rdma_hw_stats structure
 * @ibdev: device pointer from stack
 * @stats: stats pointer from stack
 * @port_num: port number
 * @index: which hw counter the stack is requesting we update
 */
static int irdma_get_hw_stats(struct ib_device *ibdev,
			      struct rdma_hw_stats *stats, u32 port_num,
			      int index)
{
	struct irdma_device *iwdev = to_iwdev(ibdev);
	struct irdma_dev_hw_stats *hw_stats = &iwdev->vsi.pestat->hw_stats;

	if (iwdev->rf->rdma_ver >= IRDMA_GEN_2)
		irdma_cqp_gather_stats_cmd(&iwdev->rf->sc_dev, iwdev->vsi.pestat, true);
	else
		irdma_cqp_gather_stats_gen1(&iwdev->rf->sc_dev, iwdev->vsi.pestat);

	memcpy(&stats->value[0], hw_stats, sizeof(u64) * stats->num_counters);

	return stats->num_counters;
}

/**
 * irdma_query_gid - Query port GID
 * @ibdev: device pointer from stack
 * @port: port number
 * @index: Entry index
 * @gid: Global ID
 */
static int irdma_query_gid(struct ib_device *ibdev, u32 port, int index,
			   union ib_gid *gid)
{
	struct irdma_device *iwdev = to_iwdev(ibdev);

	memset(gid->raw, 0, sizeof(gid->raw));
	ether_addr_copy(gid->raw, iwdev->netdev->dev_addr);

	return 0;
}

/**
 * mcast_list_add -  Add a new mcast item to list
 * @rf: RDMA PCI function
 * @new_elem: pointer to element to add
 */
static void mcast_list_add(struct irdma_pci_f *rf,
			   struct mc_table_list *new_elem)
{
	list_add(&new_elem->list, &rf->mc_qht_list.list);
}

/**
 * mcast_list_del - Remove an mcast item from list
 * @mc_qht_elem: pointer to mcast table list element
 */
static void mcast_list_del(struct mc_table_list *mc_qht_elem)
{
	if (mc_qht_elem)
		list_del(&mc_qht_elem->list);
}

/**
 * mcast_list_lookup_ip - Search mcast list for address
 * @rf: RDMA PCI function
 * @ip_mcast: pointer to mcast IP address
 */
static struct mc_table_list *mcast_list_lookup_ip(struct irdma_pci_f *rf,
						  u32 *ip_mcast)
{
	struct mc_table_list *mc_qht_el;
	struct list_head *pos, *q;

	list_for_each_safe (pos, q, &rf->mc_qht_list.list) {
		mc_qht_el = list_entry(pos, struct mc_table_list, list);
		if (!memcmp(mc_qht_el->mc_info.dest_ip, ip_mcast,
			    sizeof(mc_qht_el->mc_info.dest_ip)))
			return mc_qht_el;
	}

	return NULL;
}

/**
 * irdma_mcast_cqp_op - perform a mcast cqp operation
 * @iwdev: irdma device
 * @mc_grp_ctx: mcast group info
 * @op: operation
 *
 * returns error status
 */
static int irdma_mcast_cqp_op(struct irdma_device *iwdev,
			      struct irdma_mcast_grp_info *mc_grp_ctx, u8 op)
{
	struct cqp_cmds_info *cqp_info;
	struct irdma_cqp_request *cqp_request;
	int status;

	cqp_request = irdma_alloc_and_get_cqp_request(&iwdev->rf->cqp, true);
	if (!cqp_request)
		return -ENOMEM;

	cqp_request->info.in.u.mc_create.info = *mc_grp_ctx;
	cqp_info = &cqp_request->info;
	cqp_info->cqp_cmd = op;
	cqp_info->post_sq = 1;
	cqp_info->in.u.mc_create.scratch = (uintptr_t)cqp_request;
	cqp_info->in.u.mc_create.cqp = &iwdev->rf->cqp.sc_cqp;
	status = irdma_handle_cqp_op(iwdev->rf, cqp_request);
	irdma_put_cqp_request(&iwdev->rf->cqp, cqp_request);

	return status;
}

/**
 * irdma_mcast_mac - Get the multicast MAC for an IP address
 * @ip_addr: IPv4 or IPv6 address
 * @mac: pointer to result MAC address
 * @ipv4: flag indicating IPv4 or IPv6
 *
 */
void irdma_mcast_mac(u32 *ip_addr, u8 *mac, bool ipv4)
{
	u8 *ip = (u8 *)ip_addr;

	if (ipv4) {
		unsigned char mac4[ETH_ALEN] = {0x01, 0x00, 0x5E, 0x00,
						0x00, 0x00};

		mac4[3] = ip[2] & 0x7F;
		mac4[4] = ip[1];
		mac4[5] = ip[0];
		ether_addr_copy(mac, mac4);
	} else {
		unsigned char mac6[ETH_ALEN] = {0x33, 0x33, 0x00, 0x00,
						0x00, 0x00};

		mac6[2] = ip[3];
		mac6[3] = ip[2];
		mac6[4] = ip[1];
		mac6[5] = ip[0];
		ether_addr_copy(mac, mac6);
	}
}

/**
 * irdma_attach_mcast - attach a qp to a multicast group
 * @ibqp: ptr to qp
 * @ibgid: pointer to global ID
 * @lid: local ID
 *
 * returns error status
 */
static int irdma_attach_mcast(struct ib_qp *ibqp, union ib_gid *ibgid, u16 lid)
{
	struct irdma_qp *iwqp = to_iwqp(ibqp);
	struct irdma_device *iwdev = iwqp->iwdev;
	struct irdma_pci_f *rf = iwdev->rf;
	struct mc_table_list *mc_qht_elem;
	struct irdma_mcast_grp_ctx_entry_info mcg_info = {};
	unsigned long flags;
	u32 ip_addr[4] = {};
	u32 mgn;
	u32 no_mgs;
	int ret = 0;
	bool ipv4;
	u16 vlan_id;
	union irdma_sockaddr sgid_addr;
	unsigned char dmac[ETH_ALEN];

	rdma_gid2ip((struct sockaddr *)&sgid_addr, ibgid);

	if (!ipv6_addr_v4mapped((struct in6_addr *)ibgid)) {
		irdma_copy_ip_ntohl(ip_addr,
				    sgid_addr.saddr_in6.sin6_addr.in6_u.u6_addr32);
		irdma_get_vlan_mac_ipv6(ip_addr, &vlan_id, NULL);
		ipv4 = false;
		ibdev_dbg(&iwdev->ibdev,
			  "VERBS: qp_id=%d, IP6address=%pI6\n", ibqp->qp_num,
			  ip_addr);
		irdma_mcast_mac(ip_addr, dmac, false);
	} else {
		ip_addr[0] = ntohl(sgid_addr.saddr_in.sin_addr.s_addr);
		ipv4 = true;
		vlan_id = irdma_get_vlan_ipv4(ip_addr);
		irdma_mcast_mac(ip_addr, dmac, true);
		ibdev_dbg(&iwdev->ibdev,
			  "VERBS: qp_id=%d, IP4address=%pI4, MAC=%pM\n",
			  ibqp->qp_num, ip_addr, dmac);
	}

	spin_lock_irqsave(&rf->qh_list_lock, flags);
	mc_qht_elem = mcast_list_lookup_ip(rf, ip_addr);
	if (!mc_qht_elem) {
		struct irdma_dma_mem *dma_mem_mc;

		spin_unlock_irqrestore(&rf->qh_list_lock, flags);
		mc_qht_elem = kzalloc(sizeof(*mc_qht_elem), GFP_KERNEL);
		if (!mc_qht_elem)
			return -ENOMEM;

		mc_qht_elem->mc_info.ipv4_valid = ipv4;
		memcpy(mc_qht_elem->mc_info.dest_ip, ip_addr,
		       sizeof(mc_qht_elem->mc_info.dest_ip));
		ret = irdma_alloc_rsrc(rf, rf->allocated_mcgs, rf->max_mcg,
				       &mgn, &rf->next_mcg);
		if (ret) {
			kfree(mc_qht_elem);
			return -ENOMEM;
		}

		mc_qht_elem->mc_info.mgn = mgn;
		dma_mem_mc = &mc_qht_elem->mc_grp_ctx.dma_mem_mc;
		dma_mem_mc->size = ALIGN(sizeof(u64) * IRDMA_MAX_MGS_PER_CTX,
					 IRDMA_HW_PAGE_SIZE);
		dma_mem_mc->va = dma_alloc_coherent(rf->hw.device,
						    dma_mem_mc->size,
						    &dma_mem_mc->pa,
						    GFP_KERNEL);
		if (!dma_mem_mc->va) {
			irdma_free_rsrc(rf, rf->allocated_mcgs, mgn);
			kfree(mc_qht_elem);
			return -ENOMEM;
		}

		mc_qht_elem->mc_grp_ctx.mg_id = (u16)mgn;
		memcpy(mc_qht_elem->mc_grp_ctx.dest_ip_addr, ip_addr,
		       sizeof(mc_qht_elem->mc_grp_ctx.dest_ip_addr));
		mc_qht_elem->mc_grp_ctx.ipv4_valid = ipv4;
		mc_qht_elem->mc_grp_ctx.vlan_id = vlan_id;
		if (vlan_id < VLAN_N_VID)
			mc_qht_elem->mc_grp_ctx.vlan_valid = true;
		mc_qht_elem->mc_grp_ctx.hmc_fcn_id = iwdev->rf->sc_dev.hmc_fn_id;
		mc_qht_elem->mc_grp_ctx.qs_handle =
			iwqp->sc_qp.vsi->qos[iwqp->sc_qp.user_pri].qs_handle;
		ether_addr_copy(mc_qht_elem->mc_grp_ctx.dest_mac_addr, dmac);

		spin_lock_irqsave(&rf->qh_list_lock, flags);
		mcast_list_add(rf, mc_qht_elem);
	} else {
		if (mc_qht_elem->mc_grp_ctx.no_of_mgs ==
		    IRDMA_MAX_MGS_PER_CTX) {
			spin_unlock_irqrestore(&rf->qh_list_lock, flags);
			return -ENOMEM;
		}
	}

	mcg_info.qp_id = iwqp->ibqp.qp_num;
	no_mgs = mc_qht_elem->mc_grp_ctx.no_of_mgs;
	irdma_sc_add_mcast_grp(&mc_qht_elem->mc_grp_ctx, &mcg_info);
	spin_unlock_irqrestore(&rf->qh_list_lock, flags);

	/* Only if there is a change do we need to modify or create */
	if (!no_mgs) {
		ret = irdma_mcast_cqp_op(iwdev, &mc_qht_elem->mc_grp_ctx,
					 IRDMA_OP_MC_CREATE);
	} else if (no_mgs != mc_qht_elem->mc_grp_ctx.no_of_mgs) {
		ret = irdma_mcast_cqp_op(iwdev, &mc_qht_elem->mc_grp_ctx,
					 IRDMA_OP_MC_MODIFY);
	} else {
		return 0;
	}

	if (ret)
		goto error;

	return 0;

error:
	irdma_sc_del_mcast_grp(&mc_qht_elem->mc_grp_ctx, &mcg_info);
	if (!mc_qht_elem->mc_grp_ctx.no_of_mgs) {
		mcast_list_del(mc_qht_elem);
		dma_free_coherent(rf->hw.device,
				  mc_qht_elem->mc_grp_ctx.dma_mem_mc.size,
				  mc_qht_elem->mc_grp_ctx.dma_mem_mc.va,
				  mc_qht_elem->mc_grp_ctx.dma_mem_mc.pa);
		mc_qht_elem->mc_grp_ctx.dma_mem_mc.va = NULL;
		irdma_free_rsrc(rf, rf->allocated_mcgs,
				mc_qht_elem->mc_grp_ctx.mg_id);
		kfree(mc_qht_elem);
	}

	return ret;
}

/**
 * irdma_detach_mcast - detach a qp from a multicast group
 * @ibqp: ptr to qp
 * @ibgid: pointer to global ID
 * @lid: local ID
 *
 * returns error status
 */
static int irdma_detach_mcast(struct ib_qp *ibqp, union ib_gid *ibgid, u16 lid)
{
	struct irdma_qp *iwqp = to_iwqp(ibqp);
	struct irdma_device *iwdev = iwqp->iwdev;
	struct irdma_pci_f *rf = iwdev->rf;
	u32 ip_addr[4] = {};
	struct mc_table_list *mc_qht_elem;
	struct irdma_mcast_grp_ctx_entry_info mcg_info = {};
	int ret;
	unsigned long flags;
	union irdma_sockaddr sgid_addr;

	rdma_gid2ip((struct sockaddr *)&sgid_addr, ibgid);
	if (!ipv6_addr_v4mapped((struct in6_addr *)ibgid))
		irdma_copy_ip_ntohl(ip_addr,
				    sgid_addr.saddr_in6.sin6_addr.in6_u.u6_addr32);
	else
		ip_addr[0] = ntohl(sgid_addr.saddr_in.sin_addr.s_addr);

	spin_lock_irqsave(&rf->qh_list_lock, flags);
	mc_qht_elem = mcast_list_lookup_ip(rf, ip_addr);
	if (!mc_qht_elem) {
		spin_unlock_irqrestore(&rf->qh_list_lock, flags);
		ibdev_dbg(&iwdev->ibdev,
			  "VERBS: address not found MCG\n");
		return 0;
	}

	mcg_info.qp_id = iwqp->ibqp.qp_num;
	irdma_sc_del_mcast_grp(&mc_qht_elem->mc_grp_ctx, &mcg_info);
	if (!mc_qht_elem->mc_grp_ctx.no_of_mgs) {
		mcast_list_del(mc_qht_elem);
		spin_unlock_irqrestore(&rf->qh_list_lock, flags);
		ret = irdma_mcast_cqp_op(iwdev, &mc_qht_elem->mc_grp_ctx,
					 IRDMA_OP_MC_DESTROY);
		if (ret) {
			ibdev_dbg(&iwdev->ibdev,
				  "VERBS: failed MC_DESTROY MCG\n");
			spin_lock_irqsave(&rf->qh_list_lock, flags);
			mcast_list_add(rf, mc_qht_elem);
			spin_unlock_irqrestore(&rf->qh_list_lock, flags);
			return -EAGAIN;
		}

		dma_free_coherent(rf->hw.device,
				  mc_qht_elem->mc_grp_ctx.dma_mem_mc.size,
				  mc_qht_elem->mc_grp_ctx.dma_mem_mc.va,
				  mc_qht_elem->mc_grp_ctx.dma_mem_mc.pa);
		mc_qht_elem->mc_grp_ctx.dma_mem_mc.va = NULL;
		irdma_free_rsrc(rf, rf->allocated_mcgs,
				mc_qht_elem->mc_grp_ctx.mg_id);
		kfree(mc_qht_elem);
	} else {
		spin_unlock_irqrestore(&rf->qh_list_lock, flags);
		ret = irdma_mcast_cqp_op(iwdev, &mc_qht_elem->mc_grp_ctx,
					 IRDMA_OP_MC_MODIFY);
		if (ret) {
			ibdev_dbg(&iwdev->ibdev,
				  "VERBS: failed Modify MCG\n");
			return ret;
		}
	}

	return 0;
}

static int irdma_create_hw_ah(struct irdma_device *iwdev, struct irdma_ah *ah, bool sleep)
{
	struct irdma_pci_f *rf = iwdev->rf;
	int err;

	err = irdma_alloc_rsrc(rf, rf->allocated_ahs, rf->max_ah, &ah->sc_ah.ah_info.ah_idx,
			       &rf->next_ah);
	if (err)
		return err;

	err = irdma_ah_cqp_op(rf, &ah->sc_ah, IRDMA_OP_AH_CREATE, sleep,
			      irdma_gsi_ud_qp_ah_cb, &ah->sc_ah);

	if (err) {
		ibdev_dbg(&iwdev->ibdev, "VERBS: CQP-OP Create AH fail");
		goto err_ah_create;
	}

	if (!sleep) {
		int cnt = CQP_COMPL_WAIT_TIME_MS * CQP_TIMEOUT_THRESHOLD;

		do {
			irdma_cqp_ce_handler(rf, &rf->ccq.sc_cq);
			mdelay(1);
		} while (!ah->sc_ah.ah_info.ah_valid && --cnt);

		if (!cnt) {
			ibdev_dbg(&iwdev->ibdev, "VERBS: CQP create AH timed out");
			err = -ETIMEDOUT;
			goto err_ah_create;
		}
	}
	return 0;

err_ah_create:
	irdma_free_rsrc(iwdev->rf, iwdev->rf->allocated_ahs, ah->sc_ah.ah_info.ah_idx);

	return err;
}

static int irdma_setup_ah(struct ib_ah *ibah, struct rdma_ah_init_attr *attr)
{
	struct irdma_pd *pd = to_iwpd(ibah->pd);
	struct irdma_ah *ah = container_of(ibah, struct irdma_ah, ibah);
	struct rdma_ah_attr *ah_attr = attr->ah_attr;
	const struct ib_gid_attr *sgid_attr;
	struct irdma_device *iwdev = to_iwdev(ibah->pd->device);
	struct irdma_pci_f *rf = iwdev->rf;
	struct irdma_sc_ah *sc_ah;
	struct irdma_ah_info *ah_info;
	union irdma_sockaddr sgid_addr, dgid_addr;
	int err;
	u8 dmac[ETH_ALEN];

	ah->pd = pd;
	sc_ah = &ah->sc_ah;
	sc_ah->ah_info.vsi = &iwdev->vsi;
	irdma_sc_init_ah(&rf->sc_dev, sc_ah);
	ah->sgid_index = ah_attr->grh.sgid_index;
	sgid_attr = ah_attr->grh.sgid_attr;
	memcpy(&ah->dgid, &ah_attr->grh.dgid, sizeof(ah->dgid));
	rdma_gid2ip((struct sockaddr *)&sgid_addr, &sgid_attr->gid);
	rdma_gid2ip((struct sockaddr *)&dgid_addr, &ah_attr->grh.dgid);
	ah->av.attrs = *ah_attr;
	ah->av.net_type = rdma_gid_attr_network_type(sgid_attr);
	ah_info = &sc_ah->ah_info;
	ah_info->pd_idx = pd->sc_pd.pd_id;
	if (ah_attr->ah_flags & IB_AH_GRH) {
		ah_info->flow_label = ah_attr->grh.flow_label;
		ah_info->hop_ttl = ah_attr->grh.hop_limit;
		ah_info->tc_tos = ah_attr->grh.traffic_class;
	}

	ether_addr_copy(dmac, ah_attr->roce.dmac);
	if (ah->av.net_type == RDMA_NETWORK_IPV4) {
		ah_info->ipv4_valid = true;
		ah_info->dest_ip_addr[0] =
			ntohl(dgid_addr.saddr_in.sin_addr.s_addr);
		ah_info->src_ip_addr[0] =
			ntohl(sgid_addr.saddr_in.sin_addr.s_addr);
		ah_info->do_lpbk = irdma_ipv4_is_lpb(ah_info->src_ip_addr[0],
						     ah_info->dest_ip_addr[0]);
		if (ipv4_is_multicast(dgid_addr.saddr_in.sin_addr.s_addr)) {
			ah_info->do_lpbk = true;
			irdma_mcast_mac(ah_info->dest_ip_addr, dmac, true);
		}
	} else {
		irdma_copy_ip_ntohl(ah_info->dest_ip_addr,
				    dgid_addr.saddr_in6.sin6_addr.in6_u.u6_addr32);
		irdma_copy_ip_ntohl(ah_info->src_ip_addr,
				    sgid_addr.saddr_in6.sin6_addr.in6_u.u6_addr32);
		ah_info->do_lpbk = irdma_ipv6_is_lpb(ah_info->src_ip_addr,
						     ah_info->dest_ip_addr);
		if (rdma_is_multicast_addr(&dgid_addr.saddr_in6.sin6_addr)) {
			ah_info->do_lpbk = true;
			irdma_mcast_mac(ah_info->dest_ip_addr, dmac, false);
		}
	}

	err = rdma_read_gid_l2_fields(sgid_attr, &ah_info->vlan_tag,
				      ah_info->mac_addr);
	if (err)
		return err;

	ah_info->dst_arpindex = irdma_add_arp(iwdev->rf, ah_info->dest_ip_addr,
					      ah_info->ipv4_valid, dmac);

	if (ah_info->dst_arpindex == -1)
		return -EINVAL;

	if (ah_info->vlan_tag >= VLAN_N_VID && iwdev->dcb_vlan_mode)
		ah_info->vlan_tag = 0;

	if (ah_info->vlan_tag < VLAN_N_VID) {
		u8 prio = rt_tos2priority(ah_info->tc_tos);

		prio = irdma_roce_get_vlan_prio(sgid_attr, prio);

		ah_info->vlan_tag |= (u16)prio << VLAN_PRIO_SHIFT;
		ah_info->insert_vlan_tag = true;
	}

	return 0;
}

/**
 * irdma_ah_exists - Check for existing identical AH
 * @iwdev: irdma device
 * @new_ah: AH to check for
 *
 * returns true if AH is found, false if not found.
 */
static bool irdma_ah_exists(struct irdma_device *iwdev,
			    struct irdma_ah *new_ah)
{
	struct irdma_ah *ah;
	u32 key = new_ah->sc_ah.ah_info.dest_ip_addr[0] ^
		  new_ah->sc_ah.ah_info.dest_ip_addr[1] ^
		  new_ah->sc_ah.ah_info.dest_ip_addr[2] ^
		  new_ah->sc_ah.ah_info.dest_ip_addr[3];

	hash_for_each_possible(iwdev->ah_hash_tbl, ah, list, key) {
		/* Set ah_valid and ah_id the same so memcmp can work */
		new_ah->sc_ah.ah_info.ah_idx = ah->sc_ah.ah_info.ah_idx;
		new_ah->sc_ah.ah_info.ah_valid = ah->sc_ah.ah_info.ah_valid;
		if (!memcmp(&ah->sc_ah.ah_info, &new_ah->sc_ah.ah_info,
			    sizeof(ah->sc_ah.ah_info))) {
			refcount_inc(&ah->refcnt);
			new_ah->parent_ah = ah;
			return true;
		}
	}

	return false;
}

/**
 * irdma_destroy_ah - Destroy address handle
 * @ibah: pointer to address handle
 * @ah_flags: flags for sleepable
 */
static int irdma_destroy_ah(struct ib_ah *ibah, u32 ah_flags)
{
	struct irdma_device *iwdev = to_iwdev(ibah->device);
	struct irdma_ah *ah = to_iwah(ibah);

	if ((ah_flags & RDMA_DESTROY_AH_SLEEPABLE) && ah->parent_ah) {
		mutex_lock(&iwdev->ah_tbl_lock);
		if (!refcount_dec_and_test(&ah->parent_ah->refcnt)) {
			mutex_unlock(&iwdev->ah_tbl_lock);
			return 0;
		}
		hash_del(&ah->parent_ah->list);
		kfree(ah->parent_ah);
		mutex_unlock(&iwdev->ah_tbl_lock);
	}

	irdma_ah_cqp_op(iwdev->rf, &ah->sc_ah, IRDMA_OP_AH_DESTROY,
			false, NULL, ah);

	irdma_free_rsrc(iwdev->rf, iwdev->rf->allocated_ahs,
			ah->sc_ah.ah_info.ah_idx);

	return 0;
}

/**
 * irdma_create_user_ah - create user address handle
 * @ibah: address handle
 * @attr: address handle attributes
 * @udata: User data
 *
 * returns 0 on success, error otherwise
 */
static int irdma_create_user_ah(struct ib_ah *ibah,
				struct rdma_ah_init_attr *attr,
				struct ib_udata *udata)
{
#define IRDMA_CREATE_AH_MIN_RESP_LEN offsetofend(struct irdma_create_ah_resp, rsvd)
	struct irdma_ah *ah = container_of(ibah, struct irdma_ah, ibah);
	struct irdma_device *iwdev = to_iwdev(ibah->pd->device);
	struct irdma_create_ah_resp uresp;
	struct irdma_ah *parent_ah;
	int err;

	if (udata && udata->outlen < IRDMA_CREATE_AH_MIN_RESP_LEN)
		return -EINVAL;

	err = irdma_setup_ah(ibah, attr);
	if (err)
		return err;
	mutex_lock(&iwdev->ah_tbl_lock);
	if (!irdma_ah_exists(iwdev, ah)) {
		err = irdma_create_hw_ah(iwdev, ah, true);
		if (err) {
			mutex_unlock(&iwdev->ah_tbl_lock);
			return err;
		}
		/* Add new AH to list */
		parent_ah = kmemdup(ah, sizeof(*ah), GFP_KERNEL);
		if (parent_ah) {
			u32 key = parent_ah->sc_ah.ah_info.dest_ip_addr[0] ^
				  parent_ah->sc_ah.ah_info.dest_ip_addr[1] ^
				  parent_ah->sc_ah.ah_info.dest_ip_addr[2] ^
				  parent_ah->sc_ah.ah_info.dest_ip_addr[3];

			ah->parent_ah = parent_ah;
			hash_add(iwdev->ah_hash_tbl, &parent_ah->list, key);
			refcount_set(&parent_ah->refcnt, 1);
		}
	}
	mutex_unlock(&iwdev->ah_tbl_lock);

	uresp.ah_id = ah->sc_ah.ah_info.ah_idx;
	err = ib_copy_to_udata(udata, &uresp, min(sizeof(uresp), udata->outlen));
	if (err)
		irdma_destroy_ah(ibah, attr->flags);

	return err;
}

/**
 * irdma_create_ah - create address handle
 * @ibah: address handle
 * @attr: address handle attributes
 * @udata: NULL
 *
 * returns 0 on success, error otherwise
 */
static int irdma_create_ah(struct ib_ah *ibah, struct rdma_ah_init_attr *attr,
			   struct ib_udata *udata)
{
	struct irdma_ah *ah = container_of(ibah, struct irdma_ah, ibah);
	struct irdma_device *iwdev = to_iwdev(ibah->pd->device);
	int err;

	err = irdma_setup_ah(ibah, attr);
	if (err)
		return err;
	err = irdma_create_hw_ah(iwdev, ah, attr->flags & RDMA_CREATE_AH_SLEEPABLE);

	return err;
}

/**
 * irdma_query_ah - Query address handle
 * @ibah: pointer to address handle
 * @ah_attr: address handle attributes
 */
static int irdma_query_ah(struct ib_ah *ibah, struct rdma_ah_attr *ah_attr)
{
	struct irdma_ah *ah = to_iwah(ibah);

	memset(ah_attr, 0, sizeof(*ah_attr));
	if (ah->av.attrs.ah_flags & IB_AH_GRH) {
		ah_attr->ah_flags = IB_AH_GRH;
		ah_attr->grh.flow_label = ah->sc_ah.ah_info.flow_label;
		ah_attr->grh.traffic_class = ah->sc_ah.ah_info.tc_tos;
		ah_attr->grh.hop_limit = ah->sc_ah.ah_info.hop_ttl;
		ah_attr->grh.sgid_index = ah->sgid_index;
		memcpy(&ah_attr->grh.dgid, &ah->dgid,
		       sizeof(ah_attr->grh.dgid));
	}

	return 0;
}

static enum rdma_link_layer irdma_get_link_layer(struct ib_device *ibdev,
						 u32 port_num)
{
	return IB_LINK_LAYER_ETHERNET;
}

static const struct ib_device_ops irdma_roce_dev_ops = {
	.attach_mcast = irdma_attach_mcast,
	.create_ah = irdma_create_ah,
	.create_user_ah = irdma_create_user_ah,
	.destroy_ah = irdma_destroy_ah,
	.detach_mcast = irdma_detach_mcast,
	.get_link_layer = irdma_get_link_layer,
	.get_port_immutable = irdma_roce_port_immutable,
	.modify_qp = irdma_modify_qp_roce,
	.query_ah = irdma_query_ah,
	.query_pkey = irdma_query_pkey,
};

static const struct ib_device_ops irdma_iw_dev_ops = {
	.get_port_immutable = irdma_iw_port_immutable,
	.iw_accept = irdma_accept,
	.iw_add_ref = irdma_qp_add_ref,
	.iw_connect = irdma_connect,
	.iw_create_listen = irdma_create_listen,
	.iw_destroy_listen = irdma_destroy_listen,
	.iw_get_qp = irdma_get_qp,
	.iw_reject = irdma_reject,
	.iw_rem_ref = irdma_qp_rem_ref,
	.modify_qp = irdma_modify_qp,
	.query_gid = irdma_query_gid,
};

static const struct ib_device_ops irdma_dev_ops = {
	.owner = THIS_MODULE,
	.driver_id = RDMA_DRIVER_IRDMA,
	.uverbs_abi_ver = IRDMA_ABI_VER,

	.alloc_hw_port_stats = irdma_alloc_hw_port_stats,
	.alloc_mr = irdma_alloc_mr,
	.alloc_mw = irdma_alloc_mw,
	.alloc_pd = irdma_alloc_pd,
	.alloc_ucontext = irdma_alloc_ucontext,
	.create_cq = irdma_create_cq,
	.create_qp = irdma_create_qp,
	.dealloc_driver = irdma_ib_dealloc_device,
	.dealloc_mw = irdma_dealloc_mw,
	.dealloc_pd = irdma_dealloc_pd,
	.dealloc_ucontext = irdma_dealloc_ucontext,
	.dereg_mr = irdma_dereg_mr,
	.destroy_cq = irdma_destroy_cq,
	.destroy_qp = irdma_destroy_qp,
	.disassociate_ucontext = irdma_disassociate_ucontext,
	.get_dev_fw_str = irdma_get_dev_fw_str,
	.get_dma_mr = irdma_get_dma_mr,
	.get_hw_stats = irdma_get_hw_stats,
	.map_mr_sg = irdma_map_mr_sg,
	.mmap = irdma_mmap,
	.mmap_free = irdma_mmap_free,
	.poll_cq = irdma_poll_cq,
	.post_recv = irdma_post_recv,
	.post_send = irdma_post_send,
	.query_device = irdma_query_device,
	.query_port = irdma_query_port,
	.query_qp = irdma_query_qp,
	.reg_user_mr = irdma_reg_user_mr,
	.reg_user_mr_dmabuf = irdma_reg_user_mr_dmabuf,
	.req_notify_cq = irdma_req_notify_cq,
	.resize_cq = irdma_resize_cq,
	INIT_RDMA_OBJ_SIZE(ib_pd, irdma_pd, ibpd),
	INIT_RDMA_OBJ_SIZE(ib_ucontext, irdma_ucontext, ibucontext),
	INIT_RDMA_OBJ_SIZE(ib_ah, irdma_ah, ibah),
	INIT_RDMA_OBJ_SIZE(ib_cq, irdma_cq, ibcq),
	INIT_RDMA_OBJ_SIZE(ib_mw, irdma_mr, ibmw),
	INIT_RDMA_OBJ_SIZE(ib_qp, irdma_qp, ibqp),
};

/**
 * irdma_init_roce_device - initialization of roce rdma device
 * @iwdev: irdma device
 */
static void irdma_init_roce_device(struct irdma_device *iwdev)
{
	iwdev->ibdev.node_type = RDMA_NODE_IB_CA;
	addrconf_addr_eui48((u8 *)&iwdev->ibdev.node_guid,
			    iwdev->netdev->dev_addr);
	ib_set_device_ops(&iwdev->ibdev, &irdma_roce_dev_ops);
}

/**
 * irdma_init_iw_device - initialization of iwarp rdma device
 * @iwdev: irdma device
 */
static void irdma_init_iw_device(struct irdma_device *iwdev)
{
	struct net_device *netdev = iwdev->netdev;

	iwdev->ibdev.node_type = RDMA_NODE_RNIC;
	addrconf_addr_eui48((u8 *)&iwdev->ibdev.node_guid,
			    netdev->dev_addr);
	memcpy(iwdev->ibdev.iw_ifname, netdev->name,
	       sizeof(iwdev->ibdev.iw_ifname));
	ib_set_device_ops(&iwdev->ibdev, &irdma_iw_dev_ops);
}

/**
 * irdma_init_rdma_device - initialization of rdma device
 * @iwdev: irdma device
 */
static void irdma_init_rdma_device(struct irdma_device *iwdev)
{
	struct pci_dev *pcidev = iwdev->rf->pcidev;

	if (iwdev->roce_mode)
		irdma_init_roce_device(iwdev);
	else
		irdma_init_iw_device(iwdev);

	iwdev->ibdev.phys_port_cnt = 1;
	iwdev->ibdev.num_comp_vectors = iwdev->rf->ceqs_count;
	iwdev->ibdev.dev.parent = &pcidev->dev;
	ib_set_device_ops(&iwdev->ibdev, &irdma_dev_ops);
}

/**
 * irdma_port_ibevent - indicate port event
 * @iwdev: irdma device
 */
void irdma_port_ibevent(struct irdma_device *iwdev)
{
	struct ib_event event;

	event.device = &iwdev->ibdev;
	event.element.port_num = 1;
	event.event =
		iwdev->iw_status ? IB_EVENT_PORT_ACTIVE : IB_EVENT_PORT_ERR;
	ib_dispatch_event(&event);
}

/**
 * irdma_ib_unregister_device - unregister rdma device from IB
 * core
 * @iwdev: irdma device
 */
void irdma_ib_unregister_device(struct irdma_device *iwdev)
{
	iwdev->iw_status = 0;
	irdma_port_ibevent(iwdev);
	ib_unregister_device(&iwdev->ibdev);
}

/**
 * irdma_ib_register_device - register irdma device to IB core
 * @iwdev: irdma device
 */
int irdma_ib_register_device(struct irdma_device *iwdev)
{
	int ret;

	irdma_init_rdma_device(iwdev);

	ret = ib_device_set_netdev(&iwdev->ibdev, iwdev->netdev, 1);
	if (ret)
		goto error;
	dma_set_max_seg_size(iwdev->rf->hw.device, UINT_MAX);
	ret = ib_register_device(&iwdev->ibdev, "irdma%d", iwdev->rf->hw.device);
	if (ret)
		goto error;

	iwdev->iw_status = 1;
	irdma_port_ibevent(iwdev);

	return 0;

error:
	if (ret)
		ibdev_dbg(&iwdev->ibdev, "VERBS: Register RDMA device fail\n");

	return ret;
}

/**
 * irdma_ib_dealloc_device
 * @ibdev: ib device
 *
 * callback from ibdev dealloc_driver to deallocate resources
 * unber irdma device
 */
void irdma_ib_dealloc_device(struct ib_device *ibdev)
{
	struct irdma_device *iwdev = to_iwdev(ibdev);

	irdma_rt_deinit_hw(iwdev);
	irdma_ctrl_deinit_hw(iwdev->rf);
	kfree(iwdev->rf);
}
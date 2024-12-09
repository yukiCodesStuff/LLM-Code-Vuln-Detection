{
	struct hns_roce_dev *hr_dev = to_hr_dev(pd->device);
	struct hns_roce_srq *srq;
	int srq_desc_size;
	int srq_buf_size;
	u32 page_shift;
	int ret = 0;
	u32 npages;
	u32 cqn;

	/* Check the actual SRQ wqe and SRQ sge num */
	if (srq_init_attr->attr.max_wr >= hr_dev->caps.max_srq_wrs ||
	    srq_init_attr->attr.max_sge > hr_dev->caps.max_srq_sges)
		return ERR_PTR(-EINVAL);

	srq = kzalloc(sizeof(*srq), GFP_KERNEL);
	if (!srq)
		return ERR_PTR(-ENOMEM);

	mutex_init(&srq->mutex);
	spin_lock_init(&srq->lock);

	srq->max = roundup_pow_of_two(srq_init_attr->attr.max_wr + 1);
	srq->max_gs = srq_init_attr->attr.max_sge;

	srq_desc_size = max(16, 16 * srq->max_gs);

	srq->wqe_shift = ilog2(srq_desc_size);

	srq_buf_size = srq->max * srq_desc_size;

	srq->idx_que.entry_sz = HNS_ROCE_IDX_QUE_ENTRY_SZ;
	srq->idx_que.buf_size = srq->max * srq->idx_que.entry_sz;
	srq->mtt.mtt_type = MTT_TYPE_SRQWQE;
	srq->idx_que.mtt.mtt_type = MTT_TYPE_IDX;

	if (udata) {
		struct hns_roce_ib_create_srq  ucmd;

		if (ib_copy_from_udata(&ucmd, udata, sizeof(ucmd))) {
			ret = -EFAULT;
			goto err_srq;
		}

		srq->umem = ib_umem_get(pd->uobject->context, ucmd.buf_addr,
					srq_buf_size, 0, 0);
		if (IS_ERR(srq->umem)) {
			ret = PTR_ERR(srq->umem);
			goto err_srq;
		}

		if (hr_dev->caps.srqwqe_buf_pg_sz) {
			npages = (ib_umem_page_count(srq->umem) +
				  (1 << hr_dev->caps.srqwqe_buf_pg_sz) - 1) /
				  (1 << hr_dev->caps.srqwqe_buf_pg_sz);
			page_shift = PAGE_SHIFT + hr_dev->caps.srqwqe_buf_pg_sz;
			ret = hns_roce_mtt_init(hr_dev, npages,
						page_shift,
						&srq->mtt);
		} else
			ret = hns_roce_mtt_init(hr_dev,
						ib_umem_page_count(srq->umem),
						srq->umem->page_shift,
						&srq->mtt);
		if (ret)
			goto err_buf;

		ret = hns_roce_ib_umem_write_mtt(hr_dev, &srq->mtt, srq->umem);
		if (ret)
			goto err_srq_mtt;

		/* config index queue BA */
		srq->idx_que.umem = ib_umem_get(pd->uobject->context,
						ucmd.que_addr,
						srq->idx_que.buf_size, 0, 0);
		if (IS_ERR(srq->idx_que.umem)) {
			dev_err(hr_dev->dev,
				"ib_umem_get error for index queue\n");
			ret = PTR_ERR(srq->idx_que.umem);
			goto err_srq_mtt;
		}

		if (hr_dev->caps.idx_buf_pg_sz) {
			npages = (ib_umem_page_count(srq->idx_que.umem) +
				  (1 << hr_dev->caps.idx_buf_pg_sz) - 1) /
				  (1 << hr_dev->caps.idx_buf_pg_sz);
			page_shift = PAGE_SHIFT + hr_dev->caps.idx_buf_pg_sz;
			ret = hns_roce_mtt_init(hr_dev, npages,
						page_shift, &srq->idx_que.mtt);
		} else {
			ret = hns_roce_mtt_init(hr_dev,
				       ib_umem_page_count(srq->idx_que.umem),
				       srq->idx_que.umem->page_shift,
				       &srq->idx_que.mtt);
		}

		if (ret) {
			dev_err(hr_dev->dev,
				"hns_roce_mtt_init error for idx que\n");
			goto err_idx_mtt;
		}

		ret = hns_roce_ib_umem_write_mtt(hr_dev, &srq->idx_que.mtt,
						 srq->idx_que.umem);
		if (ret) {
			dev_err(hr_dev->dev,
			      "hns_roce_ib_umem_write_mtt error for idx que\n");
			goto err_idx_buf;
		}
	} else {
		page_shift = PAGE_SHIFT + hr_dev->caps.srqwqe_buf_pg_sz;
		if (hns_roce_buf_alloc(hr_dev, srq_buf_size,
				      (1 << page_shift) * 2,
				      &srq->buf, page_shift)) {
			ret = -ENOMEM;
			goto err_srq;
		}

		srq->head = 0;
		srq->tail = srq->max - 1;

		ret = hns_roce_mtt_init(hr_dev, srq->buf.npages,
					srq->buf.page_shift, &srq->mtt);
		if (ret)
			goto err_buf;

		ret = hns_roce_buf_write_mtt(hr_dev, &srq->mtt, &srq->buf);
		if (ret)
			goto err_srq_mtt;

		page_shift = PAGE_SHIFT + hr_dev->caps.idx_buf_pg_sz;
		ret = hns_roce_create_idx_que(pd, srq, page_shift);
		if (ret) {
			dev_err(hr_dev->dev, "Create idx queue fail(%d)!\n",
				ret);
			goto err_srq_mtt;
		}

		/* Init mtt table for idx_que */
		ret = hns_roce_mtt_init(hr_dev, srq->idx_que.idx_buf.npages,
					srq->idx_que.idx_buf.page_shift,
					&srq->idx_que.mtt);
		if (ret)
			goto err_create_idx;

		/* Write buffer address into the mtt table */
		ret = hns_roce_buf_write_mtt(hr_dev, &srq->idx_que.mtt,
					     &srq->idx_que.idx_buf);
		if (ret)
			goto err_idx_buf;

		srq->wrid = kvmalloc_array(srq->max, sizeof(u64), GFP_KERNEL);
		if (!srq->wrid) {
			ret = -ENOMEM;
			goto err_idx_buf;
		}
	}

	cqn = ib_srq_has_cq(srq_init_attr->srq_type) ?
	      to_hr_cq(srq_init_attr->ext.cq)->cqn : 0;

	srq->db_reg_l = hr_dev->reg_base + SRQ_DB_REG;

	ret = hns_roce_srq_alloc(hr_dev, to_hr_pd(pd)->pdn, cqn, 0,
				 &srq->mtt, 0, srq);
	if (ret)
		goto err_wrid;

	srq->event = hns_roce_ib_srq_event;
	srq->ibsrq.ext.xrc.srq_num = srq->srqn;

	if (udata) {
		if (ib_copy_to_udata(udata, &srq->srqn, sizeof(__u32))) {
			ret = -EFAULT;
			goto err_wrid;
		}
	}

	return &srq->ibsrq;

err_wrid:
	kvfree(srq->wrid);

err_idx_buf:
	hns_roce_mtt_cleanup(hr_dev, &srq->idx_que.mtt);

err_idx_mtt:
	if (udata)
		ib_umem_release(srq->idx_que.umem);

err_create_idx:
	hns_roce_buf_free(hr_dev, srq->idx_que.buf_size,
			  &srq->idx_que.idx_buf);
	kfree(srq->idx_que.bitmap);

err_srq_mtt:
	hns_roce_mtt_cleanup(hr_dev, &srq->mtt);

err_buf:
	if (udata)
		ib_umem_release(srq->umem);
	else
		hns_roce_buf_free(hr_dev, srq_buf_size, &srq->buf);

err_srq:
	kfree(srq);
	return ERR_PTR(ret);
}

int hns_roce_destroy_srq(struct ib_srq *ibsrq)
{
	struct hns_roce_dev *hr_dev = to_hr_dev(ibsrq->device);
	struct hns_roce_srq *srq = to_hr_srq(ibsrq);

	hns_roce_srq_free(hr_dev, srq);
	hns_roce_mtt_cleanup(hr_dev, &srq->mtt);

	if (ibsrq->uobject) {
		hns_roce_mtt_cleanup(hr_dev, &srq->idx_que.mtt);
		ib_umem_release(srq->idx_que.umem);
		ib_umem_release(srq->umem);
	} else {
		kvfree(srq->wrid);
		hns_roce_buf_free(hr_dev, srq->max << srq->wqe_shift,
				  &srq->buf);
	}

	kfree(srq);

	return 0;
}

int hns_roce_init_srq_table(struct hns_roce_dev *hr_dev)
{
	struct hns_roce_srq_table *srq_table = &hr_dev->srq_table;

	xa_init(&srq_table->xa);

	return hns_roce_bitmap_init(&srq_table->bitmap, hr_dev->caps.num_srqs,
				    hr_dev->caps.num_srqs - 1,
				    hr_dev->caps.reserved_srqs, 0);
}

void hns_roce_cleanup_srq_table(struct hns_roce_dev *hr_dev)
{
	hns_roce_bitmap_cleanup(&hr_dev->srq_table.bitmap);
}
		if (!srq->wrid) {
			ret = -ENOMEM;
			goto err_idx_buf;
		}
	}

	cqn = ib_srq_has_cq(srq_init_attr->srq_type) ?
	      to_hr_cq(srq_init_attr->ext.cq)->cqn : 0;

	srq->db_reg_l = hr_dev->reg_base + SRQ_DB_REG;

	ret = hns_roce_srq_alloc(hr_dev, to_hr_pd(pd)->pdn, cqn, 0,
				 &srq->mtt, 0, srq);
	if (ret)
		goto err_wrid;

	srq->event = hns_roce_ib_srq_event;
	srq->ibsrq.ext.xrc.srq_num = srq->srqn;

	if (udata) {
		if (ib_copy_to_udata(udata, &srq->srqn, sizeof(__u32))) {
			ret = -EFAULT;
			goto err_wrid;
		}
	}
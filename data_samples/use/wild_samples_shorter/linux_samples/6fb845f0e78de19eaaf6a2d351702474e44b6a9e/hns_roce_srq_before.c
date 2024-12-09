				   struct ib_udata *udata)
{
	struct hns_roce_dev *hr_dev = to_hr_dev(pd->device);
	struct hns_roce_srq *srq;
	int srq_desc_size;
	int srq_buf_size;
	u32 page_shift;

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
	struct prefetch_mr_work *w =
		container_of(work, struct prefetch_mr_work, work);

	if (w->dev->ib_dev.reg_state == IB_DEV_REGISTERED)
		mlx5_ib_prefetch_sg_list(w->dev, w->pf_flags, w->sg_list,
					 w->num_sge);

	kfree(w);
}

int mlx5_ib_advise_mr_prefetch(struct ib_pd *pd,
		return mlx5_ib_prefetch_sg_list(dev, pf_flags, sg_list,
						num_sge);

	if (dev->ib_dev.reg_state != IB_DEV_REGISTERED)
		return -ENODEV;

	work = kvzalloc(struct_size(work, sg_list, num_sge), GFP_KERNEL);
	if (!work)
		return -ENOMEM;

	memcpy(work->sg_list, sg_list, num_sge * sizeof(struct ib_sge));

	work->dev = dev;
	work->pf_flags = pf_flags;
	work->num_sge = num_sge;

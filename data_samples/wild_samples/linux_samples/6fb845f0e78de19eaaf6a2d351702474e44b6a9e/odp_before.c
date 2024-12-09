{
	struct prefetch_mr_work *w =
		container_of(work, struct prefetch_mr_work, work);

	if (w->dev->ib_dev.reg_state == IB_DEV_REGISTERED)
		mlx5_ib_prefetch_sg_list(w->dev, w->pf_flags, w->sg_list,
					 w->num_sge);

	kfree(w);
}

int mlx5_ib_advise_mr_prefetch(struct ib_pd *pd,
			       enum ib_uverbs_advise_mr_advice advice,
			       u32 flags, struct ib_sge *sg_list, u32 num_sge)
{
	struct mlx5_ib_dev *dev = to_mdev(pd->device);
	u32 pf_flags = MLX5_PF_FLAGS_PREFETCH;
	struct prefetch_mr_work *work;

	if (advice == IB_UVERBS_ADVISE_MR_ADVICE_PREFETCH)
		pf_flags |= MLX5_PF_FLAGS_DOWNGRADE;

	if (flags & IB_UVERBS_ADVISE_MR_FLAG_FLUSH)
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

	INIT_WORK(&work->work, mlx5_ib_prefetch_mr_work);
	schedule_work(&work->work);
	return 0;
}
{
	struct mlx5_ib_dev *dev = to_mdev(pd->device);
	u32 pf_flags = MLX5_PF_FLAGS_PREFETCH;
	struct prefetch_mr_work *work;

	if (advice == IB_UVERBS_ADVISE_MR_ADVICE_PREFETCH)
		pf_flags |= MLX5_PF_FLAGS_DOWNGRADE;

	if (flags & IB_UVERBS_ADVISE_MR_FLAG_FLUSH)
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

	INIT_WORK(&work->work, mlx5_ib_prefetch_mr_work);
	schedule_work(&work->work);
	return 0;
}
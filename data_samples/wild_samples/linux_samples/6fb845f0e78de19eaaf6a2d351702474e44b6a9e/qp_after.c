{
	struct mlx5_core_rsc_common *common;
	unsigned long flags;

	spin_lock_irqsave(&table->lock, flags);

	common = radix_tree_lookup(&table->tree, rsn);
	if (common)
		atomic_inc(&common->refcount);

	spin_unlock_irqrestore(&table->lock, flags);

	return common;
}
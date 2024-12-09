	if (slave->offset >= parent->size) {
		/* let's register it anyway to preserve ordering */
		slave->offset = 0;
		slave->mtd.size = 0;

		/* Initialize ->erasesize to make add_mtd_device() happy. */
		slave->mtd.erasesize = parent->erasesize;

		printk(KERN_ERR"mtd: partition \"%s\" is out of reach -- disabled\n",
			part->name);
		goto out_register;
	}
	if (slave->offset + slave->mtd.size > parent->size) {
		slave->mtd.size = parent->size - slave->offset;
		printk(KERN_WARNING"mtd: partition \"%s\" extends beyond the end of device \"%s\" -- size truncated to %#llx\n",
			part->name, parent->name, (unsigned long long)slave->mtd.size);
	}
	if (parent->numeraseregions > 1) {
		/* Deal with variable erase size stuff */
		int i, max = parent->numeraseregions;
		u64 end = slave->offset + slave->mtd.size;
		struct mtd_erase_region_info *regions = parent->eraseregions;

		/* Find the first erase regions which is part of this
		 * partition. */
		for (i = 0; i < max && regions[i].offset <= slave->offset; i++)
			;
		/* The loop searched for the region _behind_ the first one */
		if (i > 0)
			i--;

		/* Pick biggest erasesize */
		for (; i < max && regions[i].offset < end; i++) {
			if (slave->mtd.erasesize < regions[i].erasesize) {
				slave->mtd.erasesize = regions[i].erasesize;
			}
		}
{
	struct mtd_partition part;
	struct mtd_part *new;
	int ret = 0;

	/* the direct offset is expected */
	if (offset == MTDPART_OFS_APPEND ||
	    offset == MTDPART_OFS_NXTBLK)
		return -EINVAL;

	if (length == MTDPART_SIZ_FULL)
		length = parent->size - offset;

	if (length <= 0)
		return -EINVAL;

	memset(&part, 0, sizeof(part));
	part.name = name;
	part.size = length;
	part.offset = offset;

	new = allocate_partition(parent, &part, -1, offset);
	if (IS_ERR(new))
		return PTR_ERR(new);

	mutex_lock(&mtd_partitions_mutex);
	list_add(&new->list, &mtd_partitions);
	mutex_unlock(&mtd_partitions_mutex);

	ret = add_mtd_device(&new->mtd);
	if (ret)
		goto err_remove_part;

	mtd_add_partition_attrs(new);

	return 0;

err_remove_part:
	mutex_lock(&mtd_partitions_mutex);
	list_del(&new->list);
	mutex_unlock(&mtd_partitions_mutex);

	free_partition(new);

	return ret;
}
EXPORT_SYMBOL_GPL(mtd_add_partition);

/**
 * __mtd_del_partition - delete MTD partition
 *
 * @priv: internal MTD struct for partition to be deleted
 *
 * This function must be called with the partitions mutex locked.
 */
static int __mtd_del_partition(struct mtd_part *priv)
{
	struct mtd_part *child, *next;
	int err;

	list_for_each_entry_safe(child, next, &mtd_partitions, list) {
		if (child->parent == &priv->mtd) {
			err = __mtd_del_partition(child);
			if (err)
				return err;
		}
	}
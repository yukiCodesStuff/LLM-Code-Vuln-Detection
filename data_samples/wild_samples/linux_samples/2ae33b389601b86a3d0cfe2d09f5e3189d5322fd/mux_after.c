	if (found > 1) {
		pr_err("%s: Multiple signal paths (%i) for %s\n", __func__,
		       found, muxname);
		return -EINVAL;
	}

	return -ENODEV;
}

int __init omap_mux_get_by_name(const char *muxname,
			struct omap_mux_partition **found_partition,
			struct omap_mux **found_mux)
{
	struct omap_mux_partition *partition;

	list_for_each_entry(partition, &mux_partitions, node) {
		struct omap_mux *mux = NULL;
		int mux_mode = _omap_mux_get_by_name(partition, muxname, &mux);
		if (mux_mode < 0)
			continue;

		*found_partition = partition;
		*found_mux = mux;

		return mux_mode;
	}

	pr_err("%s: Could not find signal %s\n", __func__, muxname);

	return -ENODEV;
}

int __init omap_mux_init_signal(const char *muxname, int val)
{
	struct omap_mux_partition *partition = NULL;
	struct omap_mux *mux = NULL;
	u16 old_mode;
	int mux_mode;

	mux_mode = omap_mux_get_by_name(muxname, &partition, &mux);
	if (mux_mode < 0 || !mux)
		return mux_mode;

	old_mode = omap_mux_read(partition, mux->reg_offset);
	mux_mode |= val;
	pr_debug("%s: Setting signal %s 0x%04x -> 0x%04x\n",
			 __func__, muxname, old_mode, mux_mode);
	omap_mux_write(partition, mux_mode, mux->reg_offset);

	return 0;
}

struct omap_hwmod_mux_info * __init
omap_hwmod_mux_init(struct omap_device_pad *bpads, int nr_pads)
{
	struct omap_hwmod_mux_info *hmux;
	int i, nr_pads_dynamic = 0;

	if (!bpads || nr_pads < 1)
		return NULL;

	hmux = kzalloc(sizeof(struct omap_hwmod_mux_info), GFP_KERNEL);
	if (!hmux)
		goto err1;

	hmux->nr_pads = nr_pads;

	hmux->pads = kzalloc(sizeof(struct omap_device_pad) *
				nr_pads, GFP_KERNEL);
	if (!hmux->pads)
		goto err2;

	for (i = 0; i < hmux->nr_pads; i++) {
		struct omap_mux_partition *partition;
		struct omap_device_pad *bpad = &bpads[i], *pad = &hmux->pads[i];
		struct omap_mux *mux;
		int mux_mode;

		mux_mode = omap_mux_get_by_name(bpad->name, &partition, &mux);
		if (mux_mode < 0)
			goto err3;
		if (!pad->partition)
			pad->partition = partition;
		if (!pad->mux)
			pad->mux = mux;

		pad->name = kzalloc(strlen(bpad->name) + 1, GFP_KERNEL);
		if (!pad->name) {
			int j;

			for (j = i - 1; j >= 0; j--)
				kfree(hmux->pads[j].name);
			goto err3;
		}
	list_for_each_entry(partition, &mux_partitions, node) {
		struct omap_mux *mux = NULL;
		int mux_mode = _omap_mux_get_by_name(partition, muxname, &mux);
		if (mux_mode < 0)
			continue;

		*found_partition = partition;
		*found_mux = mux;

		return mux_mode;
	}

	pr_err("%s: Could not find signal %s\n", __func__, muxname);

	return -ENODEV;
}

int __init omap_mux_init_signal(const char *muxname, int val)
{
	struct omap_mux_partition *partition = NULL;
	struct omap_mux *mux = NULL;
	u16 old_mode;
	int mux_mode;

	mux_mode = omap_mux_get_by_name(muxname, &partition, &mux);
	if (mux_mode < 0 || !mux)
		return mux_mode;

	old_mode = omap_mux_read(partition, mux->reg_offset);
	mux_mode |= val;
	pr_debug("%s: Setting signal %s 0x%04x -> 0x%04x\n",
			 __func__, muxname, old_mode, mux_mode);
	omap_mux_write(partition, mux_mode, mux->reg_offset);

	return 0;
}

struct omap_hwmod_mux_info * __init
omap_hwmod_mux_init(struct omap_device_pad *bpads, int nr_pads)
{
	struct omap_hwmod_mux_info *hmux;
	int i, nr_pads_dynamic = 0;

	if (!bpads || nr_pads < 1)
		return NULL;

	hmux = kzalloc(sizeof(struct omap_hwmod_mux_info), GFP_KERNEL);
	if (!hmux)
		goto err1;

	hmux->nr_pads = nr_pads;

	hmux->pads = kzalloc(sizeof(struct omap_device_pad) *
				nr_pads, GFP_KERNEL);
	if (!hmux->pads)
		goto err2;

	for (i = 0; i < hmux->nr_pads; i++) {
		struct omap_mux_partition *partition;
		struct omap_device_pad *bpad = &bpads[i], *pad = &hmux->pads[i];
		struct omap_mux *mux;
		int mux_mode;

		mux_mode = omap_mux_get_by_name(bpad->name, &partition, &mux);
		if (mux_mode < 0)
			goto err3;
		if (!pad->partition)
			pad->partition = partition;
		if (!pad->mux)
			pad->mux = mux;

		pad->name = kzalloc(strlen(bpad->name) + 1, GFP_KERNEL);
		if (!pad->name) {
			int j;

			for (j = i - 1; j >= 0; j--)
				kfree(hmux->pads[j].name);
			goto err3;
		}
	list_for_each_entry(e, &partition->muxmodes, node) {
		struct omap_mux *m = &e->mux;

		(void)debugfs_create_file(m->muxnames[0], S_IWUSR | S_IRUGO,
					  mux_dbg_dir, m,
					  &omap_mux_dbg_signal_fops);
	}
}

static void __init omap_mux_dbg_init(void)
{
	struct omap_mux_partition *partition;
	static struct dentry *mux_dbg_board_dir;

	mux_dbg_dir = debugfs_create_dir("omap_mux", NULL);
	if (!mux_dbg_dir)
		return;

	mux_dbg_board_dir = debugfs_create_dir("board", mux_dbg_dir);
	if (!mux_dbg_board_dir)
		return;

	list_for_each_entry(partition, &mux_partitions, node) {
		omap_mux_dbg_create_entry(partition, mux_dbg_dir);
		(void)debugfs_create_file(partition->name, S_IRUGO,
					  mux_dbg_board_dir, partition,
					  &omap_mux_dbg_board_fops);
	}
}

#else
static inline void omap_mux_dbg_init(void)
{
}
#endif	/* CONFIG_DEBUG_FS */

static void __init omap_mux_free_names(struct omap_mux *m)
{
	int i;

	for (i = 0; i < OMAP_MUX_NR_MODES; i++)
		kfree(m->muxnames[i]);

#ifdef CONFIG_DEBUG_FS
	for (i = 0; i < OMAP_MUX_NR_SIDES; i++)
		kfree(m->balls[i]);
#endif

}

/* Free all data except for GPIO pins unless CONFIG_DEBUG_FS is set */
int __init omap_mux_late_init(void)
{
	struct omap_mux_partition *partition;
	int ret;

	list_for_each_entry(partition, &mux_partitions, node) {
		struct omap_mux_entry *e, *tmp;
		list_for_each_entry_safe(e, tmp, &partition->muxmodes, node) {
			struct omap_mux *m = &e->mux;
			u16 mode = omap_mux_read(partition, m->reg_offset);

			if (OMAP_MODE_GPIO(partition, mode))
				continue;

#ifndef CONFIG_DEBUG_FS
			mutex_lock(&muxmode_mutex);
			list_del(&e->node);
			mutex_unlock(&muxmode_mutex);
			omap_mux_free_names(m);
			kfree(m);
#endif
		}
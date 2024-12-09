static bool try_merge_free_space(struct btrfs_free_space_ctl *ctl,
			  struct btrfs_free_space *info, bool update_stat)
{
	struct btrfs_free_space *left_info;
	struct btrfs_free_space *right_info;
	bool merged = false;
	u64 offset = info->offset;
	u64 bytes = info->bytes;
	if (right_info && rb_prev(&right_info->offset_index))
		left_info = rb_entry(rb_prev(&right_info->offset_index),
				     struct btrfs_free_space, offset_index);
	else
		left_info = tree_search_offset(ctl, offset - 1, 0, 0);

	/* See try_merge_free_space() comment. */
	if (right_info && !right_info->bitmap &&
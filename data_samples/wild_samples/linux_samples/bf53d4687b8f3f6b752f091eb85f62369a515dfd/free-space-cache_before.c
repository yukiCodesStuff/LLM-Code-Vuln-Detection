	if (info) {
		if (info->bitmap)
			kmem_cache_free(btrfs_free_space_bitmap_cachep,
					info->bitmap);
		kmem_cache_free(btrfs_free_space_cachep, info);
	}

	return ret;
}

/*
 * Free space merging rules:
 *  1) Merge trimmed areas together
 *  2) Let untrimmed areas coalesce with trimmed areas
 *  3) Always pull neighboring regions from bitmaps
 *
 * The above rules are for when we merge free space based on btrfs_trim_state.
 * Rules 2 and 3 are subtle because they are suboptimal, but are done for the
 * same reason: to promote larger extent regions which makes life easier for
 * find_free_extent().  Rule 2 enables coalescing based on the common path
 * being returning free space from btrfs_finish_extent_commit().  So when free
 * space is trimmed, it will prevent aggregating trimmed new region and
 * untrimmed regions in the rb_tree.  Rule 3 is purely to obtain larger extents
 * and provide find_free_extent() with the largest extents possible hoping for
 * the reuse path.
 */
static bool try_merge_free_space(struct btrfs_free_space_ctl *ctl,
			  struct btrfs_free_space *info, bool update_stat)
{
	struct btrfs_free_space *left_info;
	struct btrfs_free_space *right_info;
	bool merged = false;
	u64 offset = info->offset;
	u64 bytes = info->bytes;
	const bool is_trimmed = btrfs_free_space_trimmed(info);

	/*
	 * first we want to see if there is free space adjacent to the range we
	 * are adding, if there is remove that struct and add a new one to
	 * cover the entire range
	 */
	right_info = tree_search_offset(ctl, offset + bytes, 0, 0);
	if (right_info && rb_prev(&right_info->offset_index))
		left_info = rb_entry(rb_prev(&right_info->offset_index),
				     struct btrfs_free_space, offset_index);
	else
		left_info = tree_search_offset(ctl, offset - 1, 0, 0);

	/* See try_merge_free_space() comment. */
	if (right_info && !right_info->bitmap &&
	    (!is_trimmed || btrfs_free_space_trimmed(right_info))) {
		if (update_stat)
			unlink_free_space(ctl, right_info);
		else
			__unlink_free_space(ctl, right_info);
		info->bytes += right_info->bytes;
		kmem_cache_free(btrfs_free_space_cachep, right_info);
		merged = true;
	}

	/* See try_merge_free_space() comment. */
	if (left_info && !left_info->bitmap &&
	    left_info->offset + left_info->bytes == offset &&
	    (!is_trimmed || btrfs_free_space_trimmed(left_info))) {
		if (update_stat)
			unlink_free_space(ctl, left_info);
		else
			__unlink_free_space(ctl, left_info);
		info->offset = left_info->offset;
		info->bytes += left_info->bytes;
		kmem_cache_free(btrfs_free_space_cachep, left_info);
		merged = true;
	}

	return merged;
}
{
	struct btrfs_free_space *left_info;
	struct btrfs_free_space *right_info;
	bool merged = false;
	u64 offset = info->offset;
	u64 bytes = info->bytes;
	const bool is_trimmed = btrfs_free_space_trimmed(info);

	/*
	 * first we want to see if there is free space adjacent to the range we
	 * are adding, if there is remove that struct and add a new one to
	 * cover the entire range
	 */
	right_info = tree_search_offset(ctl, offset + bytes, 0, 0);
	if (right_info && rb_prev(&right_info->offset_index))
		left_info = rb_entry(rb_prev(&right_info->offset_index),
				     struct btrfs_free_space, offset_index);
	else
		left_info = tree_search_offset(ctl, offset - 1, 0, 0);

	/* See try_merge_free_space() comment. */
	if (right_info && !right_info->bitmap &&
	    (!is_trimmed || btrfs_free_space_trimmed(right_info))) {
		if (update_stat)
			unlink_free_space(ctl, right_info);
		else
			__unlink_free_space(ctl, right_info);
		info->bytes += right_info->bytes;
		kmem_cache_free(btrfs_free_space_cachep, right_info);
		merged = true;
	}
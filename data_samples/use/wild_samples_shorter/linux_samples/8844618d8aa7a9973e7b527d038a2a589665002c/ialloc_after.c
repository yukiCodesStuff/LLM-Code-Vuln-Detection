	}

	ext4_lock_group(sb, block_group);
	if (ext4_has_group_desc_csum(sb) &&
	    (desc->bg_flags & cpu_to_le16(EXT4_BG_INODE_UNINIT))) {
		if (block_group == 0) {
			ext4_unlock_group(sb, block_group);
			unlock_buffer(bh);
			ext4_error(sb, "Inode bitmap for bg 0 marked "
				   "uninitialized");
			err = -EFSCORRUPTED;
			goto out;
		}
		memset(bh->b_data, 0, (EXT4_INODES_PER_GROUP(sb) + 7) / 8);
		ext4_mark_bitmap_end(EXT4_INODES_PER_GROUP(sb),
				     sb->s_blocksize * 8, bh->b_data);
		set_bitmap_uptodate(bh);

		/* recheck and clear flag under lock if we still need to */
		ext4_lock_group(sb, group);
		if (ext4_has_group_desc_csum(sb) &&
		    (gdp->bg_flags & cpu_to_le16(EXT4_BG_BLOCK_UNINIT))) {
			gdp->bg_flags &= cpu_to_le16(~EXT4_BG_BLOCK_UNINIT);
			ext4_free_group_clusters_set(sb, gdp,
				ext4_free_clusters_after_init(sb, group, gdp));
			ext4_block_bitmap_csum_set(sb, group, gdp,
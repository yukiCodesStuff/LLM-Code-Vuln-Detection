{
	struct buffer_head *bh;

	if (!EXT4_HAS_RO_COMPAT_FEATURE(sb,
					EXT4_FEATURE_RO_COMPAT_METADATA_CSUM))
		return 0;

	bh = ext4_get_bitmap(sb, group_data->inode_bitmap);
	if (!bh)
		return -EIO;
	ext4_inode_bitmap_csum_set(sb, group, gdp, bh,
				   EXT4_INODES_PER_GROUP(sb) / 8);
	brelse(bh);

	bh = ext4_get_bitmap(sb, group_data->block_bitmap);
	if (!bh)
		return -EIO;
	ext4_block_bitmap_csum_set(sb, group, gdp, bh,
				   EXT4_BLOCKS_PER_GROUP(sb) / 8);
	brelse(bh);

	return 0;
}

/*
 * ext4_setup_new_descs() will set up the group descriptor descriptors of a flex bg
 */
static int ext4_setup_new_descs(handle_t *handle, struct super_block *sb,
				struct ext4_new_flex_group_data *flex_gd)
{
	struct ext4_new_group_data	*group_data = flex_gd->groups;
	struct ext4_group_desc		*gdp;
	struct ext4_sb_info		*sbi = EXT4_SB(sb);
	struct buffer_head		*gdb_bh;
	ext4_group_t			group;
	__u16				*bg_flags = flex_gd->bg_flags;
	int				i, gdb_off, gdb_num, err = 0;
	

	for (i = 0; i < flex_gd->count; i++, group_data++, bg_flags++) {
		group = group_data->group;

		gdb_off = group % EXT4_DESC_PER_BLOCK(sb);
		gdb_num = group / EXT4_DESC_PER_BLOCK(sb);

		/*
		 * get_write_access() has been called on gdb_bh by ext4_add_new_desc().
		 */
		gdb_bh = sbi->s_group_desc[gdb_num];
		/* Update group descriptor block for new group */
		gdp = (struct ext4_group_desc *)(gdb_bh->b_data +
						 gdb_off * EXT4_DESC_SIZE(sb));

		memset(gdp, 0, EXT4_DESC_SIZE(sb));
		ext4_block_bitmap_set(sb, gdp, group_data->block_bitmap);
		ext4_inode_bitmap_set(sb, gdp, group_data->inode_bitmap);
		err = ext4_set_bitmap_checksums(sb, group, gdp, group_data);
		if (err) {
			ext4_std_error(sb, err);
			break;
		}

		ext4_inode_table_set(sb, gdp, group_data->inode_table);
		ext4_free_group_clusters_set(sb, gdp,
					     EXT4_B2C(sbi, group_data->free_blocks_count));
		ext4_free_inodes_set(sb, gdp, EXT4_INODES_PER_GROUP(sb));
		if (ext4_has_group_desc_csum(sb))
			ext4_itable_unused_set(sb, gdp,
					       EXT4_INODES_PER_GROUP(sb));
		gdp->bg_flags = cpu_to_le16(*bg_flags);
		ext4_group_desc_csum_set(sb, group, gdp);

		err = ext4_handle_dirty_metadata(handle, NULL, gdb_bh);
		if (unlikely(err)) {
			ext4_std_error(sb, err);
			break;
		}

		/*
		 * We can allocate memory for mb_alloc based on the new group
		 * descriptor
		 */
		err = ext4_mb_add_groupinfo(sb, group, gdp);
		if (err)
			break;
	}
	if (ext4_has_feature_flex_bg(sb)) {
		/* with FLEX_BG, the inode/block bitmaps and itable
		 * blocks may not be in the group at all
		 * so the bitmap validation will be skipped for those groups
		 * or it has to also read the block group where the bitmaps
		 * are located to verify they are set.
		 */
		return 0;
	}
	group_first_block = ext4_group_first_block_no(sb, block_group);

	/* check whether block bitmap block number is set */
	blk = ext4_block_bitmap(sb, desc);
	offset = blk - group_first_block;
	if (offset < 0 || EXT4_B2C(sbi, offset) >= sb->s_blocksize ||
	    !ext4_test_bit(EXT4_B2C(sbi, offset), bh->b_data))
		/* bad block bitmap */
		return blk;

	/* check whether the inode bitmap block number is set */
	blk = ext4_inode_bitmap(sb, desc);
	offset = blk - group_first_block;
	if (offset < 0 || EXT4_B2C(sbi, offset) >= sb->s_blocksize ||
	    !ext4_test_bit(EXT4_B2C(sbi, offset), bh->b_data))
		/* bad block bitmap */
		return blk;

	/* check whether the inode table block number is set */
	blk = ext4_inode_table(sb, desc);
	offset = blk - group_first_block;
	if (offset < 0 || EXT4_B2C(sbi, offset) >= sb->s_blocksize ||
	    EXT4_B2C(sbi, offset + sbi->s_itb_per_group) >= sb->s_blocksize)
		return blk;
	next_zero_bit = ext4_find_next_zero_bit(bh->b_data,
			EXT4_B2C(sbi, offset + sbi->s_itb_per_group),
			EXT4_B2C(sbi, offset));
	if (next_zero_bit <
	    EXT4_B2C(sbi, offset + sbi->s_itb_per_group))
		/* bad bitmap for inode tables */
		return blk;
	return 0;
}

static int ext4_validate_block_bitmap(struct super_block *sb,
				      struct ext4_group_desc *desc,
				      ext4_group_t block_group,
				      struct buffer_head *bh)
{
	ext4_fsblk_t	blk;
	struct ext4_group_info *grp = ext4_get_group_info(sb, block_group);
	struct ext4_sb_info *sbi = EXT4_SB(sb);

	if (buffer_verified(bh))
		return 0;
	if (EXT4_MB_GRP_BBITMAP_CORRUPT(grp))
		return -EFSCORRUPTED;

	ext4_lock_group(sb, block_group);
	if (unlikely(!ext4_block_bitmap_csum_verify(sb, block_group,
			desc, bh))) {
		ext4_unlock_group(sb, block_group);
		ext4_error(sb, "bg %u: bad block bitmap checksum", block_group);
		if (!EXT4_MB_GRP_BBITMAP_CORRUPT(grp))
			percpu_counter_sub(&sbi->s_freeclusters_counter,
					   grp->bb_free);
		set_bit(EXT4_GROUP_INFO_BBITMAP_CORRUPT_BIT, &grp->bb_state);
		return -EFSBADCRC;
	}
{
	struct ext4_group_desc *desc;
	struct ext4_sb_info *sbi = EXT4_SB(sb);
	struct buffer_head *bh;
	ext4_fsblk_t bitmap_blk;
	int err;

	desc = ext4_get_group_desc(sb, block_group, NULL);
	if (!desc)
		return ERR_PTR(-EFSCORRUPTED);
	bitmap_blk = ext4_block_bitmap(sb, desc);
	if ((bitmap_blk <= le32_to_cpu(sbi->s_es->s_first_data_block)) ||
	    (bitmap_blk >= ext4_blocks_count(sbi->s_es))) {
		ext4_error(sb, "Invalid block bitmap block %llu in "
			   "block_group %u", bitmap_blk, block_group);
		return ERR_PTR(-EFSCORRUPTED);
	}
	bh = sb_getblk(sb, bitmap_blk);
	if (unlikely(!bh)) {
		ext4_error(sb, "Cannot get buffer for block bitmap - "
			   "block_group = %u, block_bitmap = %llu",
			   block_group, bitmap_blk);
		return ERR_PTR(-ENOMEM);
	}

	if (bitmap_uptodate(bh))
		goto verify;

	lock_buffer(bh);
	if (bitmap_uptodate(bh)) {
		unlock_buffer(bh);
		goto verify;
	}
	ext4_lock_group(sb, block_group);
	if (desc->bg_flags & cpu_to_le16(EXT4_BG_BLOCK_UNINIT)) {
		err = ext4_init_block_bitmap(sb, bh, block_group, desc);
		set_bitmap_uptodate(bh);
		set_buffer_uptodate(bh);
		set_buffer_verified(bh);
		ext4_unlock_group(sb, block_group);
		unlock_buffer(bh);
		if (err) {
			ext4_error(sb, "Failed to init block bitmap for group "
				   "%u: %d", block_group, err);
			goto out;
		}
		goto verify;
	}
	ext4_unlock_group(sb, block_group);
	if (buffer_uptodate(bh)) {
		/*
		 * if not uninit if bh is uptodate,
		 * bitmap is also uptodate
		 */
		set_bitmap_uptodate(bh);
		unlock_buffer(bh);
		goto verify;
	}
	/*
	 * submit the buffer_head for reading
	 */
	set_buffer_new(bh);
	trace_ext4_read_block_bitmap_load(sb, block_group);
	bh->b_end_io = ext4_end_bitmap_read;
	get_bh(bh);
	submit_bh(REQ_OP_READ, REQ_META | REQ_PRIO, bh);
	return bh;
verify:
	err = ext4_validate_block_bitmap(sb, desc, block_group, bh);
	if (err)
		goto out;
	return bh;
out:
	put_bh(bh);
	return ERR_PTR(err);
}
{
	struct ext4_group_desc *desc;
	struct ext4_sb_info *sbi = EXT4_SB(sb);
	struct buffer_head *bh;
	ext4_fsblk_t bitmap_blk;
	int err;

	desc = ext4_get_group_desc(sb, block_group, NULL);
	if (!desc)
		return ERR_PTR(-EFSCORRUPTED);
	bitmap_blk = ext4_block_bitmap(sb, desc);
	if ((bitmap_blk <= le32_to_cpu(sbi->s_es->s_first_data_block)) ||
	    (bitmap_blk >= ext4_blocks_count(sbi->s_es))) {
		ext4_error(sb, "Invalid block bitmap block %llu in "
			   "block_group %u", bitmap_blk, block_group);
		return ERR_PTR(-EFSCORRUPTED);
	}
	}
	len = ext4_free_group_clusters(sb, gdp) - ac->ac_b_ex.fe_len;
	ext4_free_group_clusters_set(sb, gdp, len);
	ext4_block_bitmap_csum_set(sb, ac->ac_b_ex.fe_group, gdp, bitmap_bh,
				   EXT4_BLOCKS_PER_GROUP(sb) / 8);
	ext4_group_desc_csum_set(sb, ac->ac_b_ex.fe_group, gdp);

	ext4_unlock_group(sb, ac->ac_b_ex.fe_group);
	percpu_counter_sub(&sbi->s_freeclusters_counter, ac->ac_b_ex.fe_len);

	ret = ext4_free_group_clusters(sb, gdp) + count_clusters;
	ext4_free_group_clusters_set(sb, gdp, ret);
	ext4_block_bitmap_csum_set(sb, block_group, gdp, bitmap_bh,
				   EXT4_BLOCKS_PER_GROUP(sb) / 8);
	ext4_group_desc_csum_set(sb, block_group, gdp);
	ext4_unlock_group(sb, block_group);
	percpu_counter_add(&sbi->s_freeclusters_counter, count_clusters);

	mb_free_blocks(NULL, &e4b, bit, count);
	blk_free_count = blocks_freed + ext4_free_group_clusters(sb, desc);
	ext4_free_group_clusters_set(sb, desc, blk_free_count);
	ext4_block_bitmap_csum_set(sb, block_group, desc, bitmap_bh,
				   EXT4_BLOCKS_PER_GROUP(sb) / 8);
	ext4_group_desc_csum_set(sb, block_group, desc);
	ext4_unlock_group(sb, block_group);
	percpu_counter_add(&sbi->s_freeclusters_counter,
			   EXT4_B2C(sbi, blocks_freed));
	minlen = EXT4_NUM_B2C(EXT4_SB(sb),
			      range->minlen >> sb->s_blocksize_bits);

	if (unlikely(minlen > EXT4_CLUSTERS_PER_GROUP(sb)) ||
	    unlikely(start >= max_blks))
		return -EINVAL;
	if (end >= max_blks)
		end = max_blks - 1;
	if (end <= first_data_blk)
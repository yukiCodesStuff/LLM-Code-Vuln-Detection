	bh = ext4_get_bitmap(sb, group_data->block_bitmap);
	if (!bh)
		return -EIO;
	ext4_block_bitmap_csum_set(sb, group, gdp, bh,
				   EXT4_BLOCKS_PER_GROUP(sb) / 8);
	brelse(bh);

	return 0;
}
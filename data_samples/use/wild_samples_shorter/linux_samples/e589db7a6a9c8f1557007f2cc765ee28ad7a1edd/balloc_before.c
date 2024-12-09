		ext4_free_inodes_set(sb, gdp, 0);
		ext4_itable_unused_set(sb, gdp, 0);
		memset(bh->b_data, 0xff, sb->s_blocksize);
		ext4_block_bitmap_csum_set(sb, block_group, gdp, bh,
					   EXT4_BLOCKS_PER_GROUP(sb) / 8);
		return;
	}
	memset(bh->b_data, 0, sb->s_blocksize);

	 */
	ext4_mark_bitmap_end(num_clusters_in_group(sb, block_group),
			     sb->s_blocksize * 8, bh->b_data);
	ext4_block_bitmap_csum_set(sb, block_group, gdp, bh,
				   EXT4_BLOCKS_PER_GROUP(sb) / 8);
	ext4_group_desc_csum_set(sb, block_group, gdp);
}

/* Return the number of free blocks in a block group.  It is used when
		return;
	}
	if (unlikely(!ext4_block_bitmap_csum_verify(sb, block_group,
			desc, bh, EXT4_BLOCKS_PER_GROUP(sb) / 8))) {
		ext4_unlock_group(sb, block_group);
		ext4_error(sb, "bg %u: bad block bitmap checksum", block_group);
		return;
	}
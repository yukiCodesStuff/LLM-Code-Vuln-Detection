	 * initialize bb_free to be able to skip
	 * empty groups without initialization
	 */
	if (desc->bg_flags & cpu_to_le16(EXT4_BG_BLOCK_UNINIT)) {
		meta_group_info[i]->bb_free =
			ext4_free_clusters_after_init(sb, group, desc);
	} else {
		meta_group_info[i]->bb_free =
#endif
	ext4_set_bits(bitmap_bh->b_data, ac->ac_b_ex.fe_start,
		      ac->ac_b_ex.fe_len);
	if (gdp->bg_flags & cpu_to_le16(EXT4_BG_BLOCK_UNINIT)) {
		gdp->bg_flags &= cpu_to_le16(~EXT4_BG_BLOCK_UNINIT);
		ext4_free_group_clusters_set(sb, gdp,
					     ext4_free_clusters_after_init(sb,
						ac->ac_b_ex.fe_group, gdp));
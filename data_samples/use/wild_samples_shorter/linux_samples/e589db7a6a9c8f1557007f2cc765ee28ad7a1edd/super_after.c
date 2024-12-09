	return es->s_checksum == ext4_superblock_csum(sb, es);
}

void ext4_superblock_csum_set(struct super_block *sb)
{
	struct ext4_super_block *es = EXT4_SB(sb)->s_es;

	if (!EXT4_HAS_RO_COMPAT_FEATURE(sb,
		EXT4_FEATURE_RO_COMPAT_METADATA_CSUM))
		return;

		sbi->s_log_groups_per_flex = 0;
		return 1;
	}
	groups_per_flex = 1U << sbi->s_log_groups_per_flex;

	err = ext4_alloc_flex_bg_array(sb, sbi->s_groups_count);
	if (err)
		goto failed;
		cpu_to_le32(percpu_counter_sum_positive(
				&EXT4_SB(sb)->s_freeinodes_counter));
	BUFFER_TRACE(sbh, "marking dirty");
	ext4_superblock_csum_set(sb);
	mark_buffer_dirty(sbh);
	if (sync) {
		error = sync_dirty_buffer(sbh);
		if (error)
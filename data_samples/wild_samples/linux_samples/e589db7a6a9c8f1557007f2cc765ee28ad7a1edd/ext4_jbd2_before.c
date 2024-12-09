{
	struct buffer_head *bh = EXT4_SB(sb)->s_sbh;
	int err = 0;

	if (ext4_handle_valid(handle)) {
		ext4_superblock_csum_set(sb,
				(struct ext4_super_block *)bh->b_data);
		err = jbd2_journal_dirty_metadata(handle, bh);
		if (err)
			ext4_journal_abort_handle(where, line, __func__,
						  bh, handle, err);
	} else {
		ext4_superblock_csum_set(sb,
				(struct ext4_super_block *)bh->b_data);
		mark_buffer_dirty(bh);
	}
	return err;
}
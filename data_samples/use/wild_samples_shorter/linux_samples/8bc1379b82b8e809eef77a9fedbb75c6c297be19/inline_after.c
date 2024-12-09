	flags |= AOP_FLAG_NOFS;

	if (ret == -ENOSPC) {
		ext4_journal_stop(handle);
		ret = ext4_da_convert_inline_data_to_extent(mapping,
							    inode,
							    flags,
							    fsdata);
		if (ret == -ENOSPC &&
		    ext4_should_retry_alloc(inode->i_sb, &retries))
			goto retry_journal;
		goto out;
	return (error < 0 ? error : 0);
}

int ext4_inline_data_truncate(struct inode *inode, int *has_inline)
{
	handle_t *handle;
	int inline_size, value_len, needed_blocks, no_expand, err = 0;
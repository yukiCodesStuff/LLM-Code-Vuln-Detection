	if (inline_size >= pos + len) {
		ret = ext4_prepare_inline_data(handle, inode, pos + len);
		if (ret && ret != -ENOSPC)
			goto out_journal;
	}

	/*
	 * We cannot recurse into the filesystem as the transaction
	 * is already started.
	 */
	flags |= AOP_FLAG_NOFS;

	if (ret == -ENOSPC) {
		ret = ext4_da_convert_inline_data_to_extent(mapping,
							    inode,
							    flags,
							    fsdata);
		ext4_journal_stop(handle);
		if (ret == -ENOSPC &&
		    ext4_should_retry_alloc(inode->i_sb, &retries))
			goto retry_journal;
		goto out;
	}
	if (!ext4_has_inline_data(inode)) {
		*has_inline = 0;
		goto out;
	}
	inline_len = min_t(size_t, ext4_get_inline_size(inode),
			   i_size_read(inode));
	if (start >= inline_len)
		goto out;
	if (start + len < inline_len)
		inline_len = start + len;
	inline_len -= start;

	error = ext4_get_inode_loc(inode, &iloc);
	if (error)
		goto out;

	physical = (__u64)iloc.bh->b_blocknr << inode->i_sb->s_blocksize_bits;
	physical += (char *)ext4_raw_inode(&iloc) - iloc.bh->b_data;
	physical += offsetof(struct ext4_inode, i_block);

	if (physical)
		error = fiemap_fill_next_extent(fieinfo, start, physical,
						inline_len, flags);
	brelse(iloc.bh);
out:
	up_read(&EXT4_I(inode)->xattr_sem);
	return (error < 0 ? error : 0);
}

/*
 * Called during xattr set, and if we can sparse space 'needed',
 * just create the extent tree evict the data to the outer block.
 *
 * We use jbd2 instead of page cache to move data to the 1st block
 * so that the whole transaction can be committed as a whole and
 * the data isn't lost because of the delayed page cache write.
 */
int ext4_try_to_evict_inline_data(handle_t *handle,
				  struct inode *inode,
				  int needed)
{
	int error;
	struct ext4_xattr_entry *entry;
	struct ext4_inode *raw_inode;
	struct ext4_iloc iloc;

	error = ext4_get_inode_loc(inode, &iloc);
	if (error)
		return error;

	raw_inode = ext4_raw_inode(&iloc);
	entry = (struct ext4_xattr_entry *)((void *)raw_inode +
					    EXT4_I(inode)->i_inline_off);
	if (EXT4_XATTR_LEN(entry->e_name_len) +
	    EXT4_XATTR_SIZE(le32_to_cpu(entry->e_value_size)) < needed) {
		error = -ENOSPC;
		goto out;
	}
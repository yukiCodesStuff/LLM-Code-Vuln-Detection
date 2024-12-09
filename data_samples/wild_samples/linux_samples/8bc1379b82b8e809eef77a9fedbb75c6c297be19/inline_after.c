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
		ext4_journal_stop(handle);
		ret = ext4_da_convert_inline_data_to_extent(mapping,
							    inode,
							    flags,
							    fsdata);
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

int ext4_inline_data_truncate(struct inode *inode, int *has_inline)
{
	handle_t *handle;
	int inline_size, value_len, needed_blocks, no_expand, err = 0;
	size_t i_size;
	void *value = NULL;
	struct ext4_xattr_ibody_find is = {
		.s = { .not_found = -ENODATA, },
	};
	struct ext4_xattr_info i = {
		.name_index = EXT4_XATTR_INDEX_SYSTEM,
		.name = EXT4_XATTR_SYSTEM_DATA,
	};


	needed_blocks = ext4_writepage_trans_blocks(inode);
	handle = ext4_journal_start(inode, EXT4_HT_INODE, needed_blocks);
	if (IS_ERR(handle))
		return PTR_ERR(handle);

	ext4_write_lock_xattr(inode, &no_expand);
	if (!ext4_has_inline_data(inode)) {
		*has_inline = 0;
		ext4_journal_stop(handle);
		return 0;
	}

	if ((err = ext4_orphan_add(handle, inode)) != 0)
		goto out;

	if ((err = ext4_get_inode_loc(inode, &is.iloc)) != 0)
		goto out;

	down_write(&EXT4_I(inode)->i_data_sem);
	i_size = inode->i_size;
	inline_size = ext4_get_inline_size(inode);
	EXT4_I(inode)->i_disksize = i_size;

	if (i_size < inline_size) {
		/* Clear the content in the xattr space. */
		if (inline_size > EXT4_MIN_INLINE_DATA_SIZE) {
			if ((err = ext4_xattr_ibody_find(inode, &i, &is)) != 0)
				goto out_error;

			BUG_ON(is.s.not_found);

			value_len = le32_to_cpu(is.s.here->e_value_size);
			value = kmalloc(value_len, GFP_NOFS);
			if (!value) {
				err = -ENOMEM;
				goto out_error;
			}

			err = ext4_xattr_ibody_get(inode, i.name_index,
						   i.name, value, value_len);
			if (err <= 0)
				goto out_error;

			i.value = value;
			i.value_len = i_size > EXT4_MIN_INLINE_DATA_SIZE ?
					i_size - EXT4_MIN_INLINE_DATA_SIZE : 0;
			err = ext4_xattr_ibody_inline_set(handle, inode,
							  &i, &is);
			if (err)
				goto out_error;
		}

		/* Clear the content within i_blocks. */
		if (i_size < EXT4_MIN_INLINE_DATA_SIZE) {
			void *p = (void *) ext4_raw_inode(&is.iloc)->i_block;
			memset(p + i_size, 0,
			       EXT4_MIN_INLINE_DATA_SIZE - i_size);
		}

		EXT4_I(inode)->i_inline_size = i_size <
					EXT4_MIN_INLINE_DATA_SIZE ?
					EXT4_MIN_INLINE_DATA_SIZE : i_size;
	}

out_error:
	up_write(&EXT4_I(inode)->i_data_sem);
out:
	brelse(is.iloc.bh);
	ext4_write_unlock_xattr(inode, &no_expand);
	kfree(value);
	if (inode->i_nlink)
		ext4_orphan_del(handle, inode);

	if (err == 0) {
		inode->i_mtime = inode->i_ctime = current_time(inode);
		err = ext4_mark_inode_dirty(handle, inode);
		if (IS_SYNC(inode))
			ext4_handle_sync(handle);
	}
	ext4_journal_stop(handle);
	return err;
}

int ext4_convert_inline_data(struct inode *inode)
{
	int error, needed_blocks, no_expand;
	handle_t *handle;
	struct ext4_iloc iloc;

	if (!ext4_has_inline_data(inode)) {
		ext4_clear_inode_state(inode, EXT4_STATE_MAY_INLINE_DATA);
		return 0;
	}

	needed_blocks = ext4_writepage_trans_blocks(inode);

	iloc.bh = NULL;
	error = ext4_get_inode_loc(inode, &iloc);
	if (error)
		return error;

	handle = ext4_journal_start(inode, EXT4_HT_WRITE_PAGE, needed_blocks);
	if (IS_ERR(handle)) {
		error = PTR_ERR(handle);
		goto out_free;
	}

	ext4_write_lock_xattr(inode, &no_expand);
	if (ext4_has_inline_data(inode))
		error = ext4_convert_inline_data_nolock(handle, inode, &iloc);
	ext4_write_unlock_xattr(inode, &no_expand);
	ext4_journal_stop(handle);
out_free:
	brelse(iloc.bh);
	return error;
}
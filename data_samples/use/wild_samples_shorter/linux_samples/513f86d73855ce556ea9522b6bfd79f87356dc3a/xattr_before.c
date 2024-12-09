{
	int error = -EFSCORRUPTED;

	if (buffer_verified(bh))
		return 0;

	if (BHDR(bh)->h_magic != cpu_to_le32(EXT4_XATTR_MAGIC) ||
	    BHDR(bh)->h_blocks != cpu_to_le32(1))
		goto errout;
	error = -EFSBADCRC;
	if (!ext4_xattr_block_csum_verify(inode, bh))
		goto errout;
	error = ext4_xattr_check_entries(BFIRST(bh), bh->b_data + bh->b_size,
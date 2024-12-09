	struct ext4_dir_entry *dirent;
	int is_dx_block = 0;

	if (block >= inode->i_size) {
		ext4_error_inode(inode, func, line, block,
		       "Attempting to read directory block (%u) that is past i_size (%llu)",
		       block, inode->i_size);
		return ERR_PTR(-EFSCORRUPTED);
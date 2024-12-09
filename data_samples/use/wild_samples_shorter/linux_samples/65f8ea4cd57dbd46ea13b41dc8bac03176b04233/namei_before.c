	struct ext4_dir_entry *dirent;
	int is_dx_block = 0;

	if (ext4_simulate_fail(inode->i_sb, EXT4_SIM_DIRBLOCK_EIO))
		bh = ERR_PTR(-EIO);
	else
		bh = ext4_bread(NULL, inode, block, 0);
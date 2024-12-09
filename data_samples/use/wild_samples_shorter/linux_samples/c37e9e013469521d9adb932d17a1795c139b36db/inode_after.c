	int			inodes_per_block, inode_offset;

	iloc->bh = NULL;
	if (inode->i_ino < EXT4_ROOT_INO ||
	    inode->i_ino > le32_to_cpu(EXT4_SB(sb)->s_es->s_inodes_count))
		return -EFSCORRUPTED;

	iloc->block_group = (inode->i_ino - 1) / EXT4_INODES_PER_GROUP(sb);
	gdp = ext4_get_group_desc(sb, iloc->block_group, NULL);
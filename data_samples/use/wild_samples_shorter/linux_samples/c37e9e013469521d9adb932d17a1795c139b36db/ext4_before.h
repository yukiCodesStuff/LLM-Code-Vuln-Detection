static inline int ext4_valid_inum(struct super_block *sb, unsigned long ino)
{
	return ino == EXT4_ROOT_INO ||
		ino == EXT4_USR_QUOTA_INO ||
		ino == EXT4_GRP_QUOTA_INO ||
		ino == EXT4_BOOT_LOADER_INO ||
		ino == EXT4_JOURNAL_INO ||
		ino == EXT4_RESIZE_INO ||
		(ino >= EXT4_FIRST_INO(sb) &&
		 ino <= le32_to_cpu(EXT4_SB(sb)->s_es->s_inodes_count));
}

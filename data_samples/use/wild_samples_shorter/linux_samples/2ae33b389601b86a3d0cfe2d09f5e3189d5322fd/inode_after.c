	.mount		= hypfs_mount,
	.kill_sb	= hypfs_kill_super
};
MODULE_ALIAS_FS("s390_hypfs");

static const struct super_operations hypfs_s_ops = {
	.statfs		= simple_statfs,
	.evict_inode	= hypfs_evict_inode,
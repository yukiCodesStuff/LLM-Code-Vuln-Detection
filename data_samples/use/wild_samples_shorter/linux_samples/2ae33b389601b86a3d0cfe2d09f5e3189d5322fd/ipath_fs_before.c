	.mount =	ipathfs_mount,
	.kill_sb =	ipathfs_kill_super,
};

int __init ipath_init_ipathfs(void)
{
	return register_filesystem(&ipathfs_fs_type);
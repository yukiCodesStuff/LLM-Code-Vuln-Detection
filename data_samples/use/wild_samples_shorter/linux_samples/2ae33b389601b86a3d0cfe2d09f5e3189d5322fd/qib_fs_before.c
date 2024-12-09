	.mount =        qibfs_mount,
	.kill_sb =      qibfs_kill_super,
};

int __init qib_init_qibfs(void)
{
	return register_filesystem(&qibfs_fs_type);
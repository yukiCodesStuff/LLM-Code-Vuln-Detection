static struct file_system_type ipathfs_fs_type = {
	.owner =	THIS_MODULE,
	.name =		"ipathfs",
	.mount =	ipathfs_mount,
	.kill_sb =	ipathfs_kill_super,
};

int __init ipath_init_ipathfs(void)
{
	return register_filesystem(&ipathfs_fs_type);
}
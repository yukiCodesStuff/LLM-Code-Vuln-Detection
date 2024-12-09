static struct file_system_type qibfs_fs_type = {
	.owner =        THIS_MODULE,
	.name =         "ipathfs",
	.mount =        qibfs_mount,
	.kill_sb =      qibfs_kill_super,
};
MODULE_ALIAS_FS("ipathfs");

int __init qib_init_qibfs(void)
{
	return register_filesystem(&qibfs_fs_type);
}
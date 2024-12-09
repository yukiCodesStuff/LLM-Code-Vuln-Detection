	struct inode *inode = NULL;
	struct dentry *root = sb->s_root;
	struct binderfs_info *info = sb->s_fs_info;

	device = kzalloc(sizeof(*device), GFP_KERNEL);
	if (!device)
		return -ENOMEM;

	/* Reserve a new minor number for the new device. */
	mutex_lock(&binderfs_minors_mutex);
	minor = ida_alloc_max(&binderfs_minors, BINDERFS_MAX_MINOR, GFP_KERNEL);
	mutex_unlock(&binderfs_minors_mutex);
	if (minor < 0) {
		ret = minor;
		goto out;
	.fs_flags	= FS_USERNS_MOUNT,
};

static int __init init_binderfs(void)
{
	int ret;

	/* Allocate new major number for binderfs. */

	return ret;
}

device_initcall(init_binderfs);
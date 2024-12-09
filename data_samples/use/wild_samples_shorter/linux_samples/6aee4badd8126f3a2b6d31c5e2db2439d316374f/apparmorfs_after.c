{
	struct aa_ns *ns;
	struct path path;
	int error;

	if (!dentry)
		return ERR_PTR(-ECHILD);

	ns = aa_get_current_ns();
	path.mnt = mntget(aafs_mnt);
	path.dentry = dget(ns_dir(ns));
	error = nd_jump_link(&path);
	aa_put_ns(ns);

	return ERR_PTR(error);
}

static int policy_readlink(struct dentry *dentry, char __user *buffer,
			   int buflen)
{
	struct aa_ns *ns;
	struct path path;

	if (!dentry)
		return ERR_PTR(-ECHILD);
	ns = aa_get_current_ns();
	path.mnt = mntget(aafs_mnt);
	path.dentry = dget(ns_dir(ns));
	nd_jump_link(&path);
	aa_put_ns(ns);

	return NULL;
}

static int policy_readlink(struct dentry *dentry, char __user *buffer,
			   int buflen)
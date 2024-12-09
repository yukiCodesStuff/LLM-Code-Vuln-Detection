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
{
	char name[32];
	int res;

	res = snprintf(name, sizeof(name), "%s:[%lu]", AAFS_NAME,
		       d_inode(dentry)->i_ino);
	if (res > 0 && res < sizeof(name))
		res = readlink_copy(buffer, buflen, name);
	else
		res = -ENOENT;

	return res;
}

static const struct inode_operations policy_link_iops = {
	.readlink	= policy_readlink,
	.get_link	= policy_get_link,
};


/**
 * aa_create_aafs - create the apparmor security filesystem
 *
 * dentries created here are released by aa_destroy_aafs
 *
 * Returns: error on failure
 */
static int __init aa_create_aafs(void)
{
	struct dentry *dent;
	int error;

	if (!apparmor_initialized)
		return 0;

	if (aa_sfs_entry.dentry) {
		AA_ERROR("%s: AppArmor securityfs already exists\n", __func__);
		return -EEXIST;
	}

	/* setup apparmorfs used to virtualize policy/ */
	aafs_mnt = kern_mount(&aafs_ops);
	if (IS_ERR(aafs_mnt))
		panic("can't set apparmorfs up\n");
	aafs_mnt->mnt_sb->s_flags &= ~SB_NOUSER;

	/* Populate fs tree. */
	error = entry_create_dir(&aa_sfs_entry, NULL);
	if (error)
		goto error;

	dent = securityfs_create_file(".load", 0666, aa_sfs_entry.dentry,
				      NULL, &aa_fs_profile_load);
	if (IS_ERR(dent))
		goto dent_error;
	ns_subload(root_ns) = dent;

	dent = securityfs_create_file(".replace", 0666, aa_sfs_entry.dentry,
				      NULL, &aa_fs_profile_replace);
	if (IS_ERR(dent))
		goto dent_error;
	ns_subreplace(root_ns) = dent;

	dent = securityfs_create_file(".remove", 0666, aa_sfs_entry.dentry,
				      NULL, &aa_fs_profile_remove);
	if (IS_ERR(dent))
		goto dent_error;
	ns_subremove(root_ns) = dent;

	dent = securityfs_create_file("revision", 0444, aa_sfs_entry.dentry,
				      NULL, &aa_fs_ns_revision_fops);
	if (IS_ERR(dent))
		goto dent_error;
	ns_subrevision(root_ns) = dent;

	/* policy tree referenced by magic policy symlink */
	mutex_lock_nested(&root_ns->lock, root_ns->level);
	error = __aafs_ns_mkdir(root_ns, aafs_mnt->mnt_root, ".policy",
				aafs_mnt->mnt_root);
	mutex_unlock(&root_ns->lock);
	if (error)
		goto error;

	/* magic symlink similar to nsfs redirects based on task policy */
	dent = securityfs_create_symlink("policy", aa_sfs_entry.dentry,
					 NULL, &policy_link_iops);
	if (IS_ERR(dent))
		goto dent_error;

	error = aa_mk_null_file(aa_sfs_entry.dentry);
	if (error)
		goto error;

	/* TODO: add default profile to apparmorfs */

	/* Report that AppArmor fs is enabled */
	aa_info_message("AppArmor Filesystem Enabled");
	return 0;

dent_error:
	error = PTR_ERR(dent);
error:
	aa_destroy_aafs();
	AA_ERROR("Error creating AppArmor securityfs\n");
	return error;
}
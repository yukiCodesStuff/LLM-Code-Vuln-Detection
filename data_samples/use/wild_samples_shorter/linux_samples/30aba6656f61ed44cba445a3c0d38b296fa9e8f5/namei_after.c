
int sysctl_protected_symlinks __read_mostly = 0;
int sysctl_protected_hardlinks __read_mostly = 0;
int sysctl_protected_fifos __read_mostly;
int sysctl_protected_regular __read_mostly;

/**
 * may_follow_link - Check symlink following for unsafe situations
 * @nd: nameidata pathwalk data
	return -EPERM;
}

/**
 * may_create_in_sticky - Check whether an O_CREAT open in a sticky directory
 *			  should be allowed, or not, on files that already
 *			  exist.
 * @dir: the sticky parent directory
 * @inode: the inode of the file to open
 *
 * Block an O_CREAT open of a FIFO (or a regular file) when:
 *   - sysctl_protected_fifos (or sysctl_protected_regular) is enabled
 *   - the file already exists
 *   - we are in a sticky directory
 *   - we don't own the file
 *   - the owner of the directory doesn't own the file
 *   - the directory is world writable
 * If the sysctl_protected_fifos (or sysctl_protected_regular) is set to 2
 * the directory doesn't have to be world writable: being group writable will
 * be enough.
 *
 * Returns 0 if the open is allowed, -ve on error.
 */
static int may_create_in_sticky(struct dentry * const dir,
				struct inode * const inode)
{
	if ((!sysctl_protected_fifos && S_ISFIFO(inode->i_mode)) ||
	    (!sysctl_protected_regular && S_ISREG(inode->i_mode)) ||
	    likely(!(dir->d_inode->i_mode & S_ISVTX)) ||
	    uid_eq(inode->i_uid, dir->d_inode->i_uid) ||
	    uid_eq(current_fsuid(), inode->i_uid))
		return 0;

	if (likely(dir->d_inode->i_mode & 0002) ||
	    (dir->d_inode->i_mode & 0020 &&
	     ((sysctl_protected_fifos >= 2 && S_ISFIFO(inode->i_mode)) ||
	      (sysctl_protected_regular >= 2 && S_ISREG(inode->i_mode))))) {
		return -EACCES;
	}
	return 0;
}

static __always_inline
const char *get_link(struct nameidata *nd)
{
	struct saved *last = nd->stack + nd->depth - 1;
	if (error)
		return error;
	audit_inode(nd->name, nd->path.dentry, 0);
	if (open_flag & O_CREAT) {
		error = -EISDIR;
		if (d_is_dir(nd->path.dentry))
			goto out;
		error = may_create_in_sticky(dir,
					     d_backing_inode(nd->path.dentry));
		if (unlikely(error))
			goto out;
	}
	error = -ENOTDIR;
	if ((nd->flags & LOOKUP_DIRECTORY) && !d_can_lookup(nd->path.dentry))
		goto out;
	if (!d_is_reg(nd->path.dentry))
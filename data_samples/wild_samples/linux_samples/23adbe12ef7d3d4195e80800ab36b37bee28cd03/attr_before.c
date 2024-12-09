	if (ia_valid & ATTR_SIZE) {
		int error = inode_newsize_ok(inode, attr->ia_size);
		if (error)
			return error;
	}

	/* If force is set do it anyway. */
	if (ia_valid & ATTR_FORCE)
		return 0;

	/* Make sure a caller can chown. */
	if ((ia_valid & ATTR_UID) &&
	    (!uid_eq(current_fsuid(), inode->i_uid) ||
	     !uid_eq(attr->ia_uid, inode->i_uid)) &&
	    !inode_capable(inode, CAP_CHOWN))
		return -EPERM;

	/* Make sure caller can chgrp. */
	if ((ia_valid & ATTR_GID) &&
	    (!uid_eq(current_fsuid(), inode->i_uid) ||
	    (!in_group_p(attr->ia_gid) && !gid_eq(attr->ia_gid, inode->i_gid))) &&
	    !inode_capable(inode, CAP_CHOWN))
		return -EPERM;

	/* Make sure a caller can chmod. */
	if (ia_valid & ATTR_MODE) {
		if (!inode_owner_or_capable(inode))
			return -EPERM;
		/* Also check the setgid bit! */
		if (!in_group_p((ia_valid & ATTR_GID) ? attr->ia_gid :
				inode->i_gid) &&
		    !inode_capable(inode, CAP_FSETID))
			attr->ia_mode &= ~S_ISGID;
	}
	if (ia_valid & ATTR_MODE) {
		if (!inode_owner_or_capable(inode))
			return -EPERM;
		/* Also check the setgid bit! */
		if (!in_group_p((ia_valid & ATTR_GID) ? attr->ia_gid :
				inode->i_gid) &&
		    !inode_capable(inode, CAP_FSETID))
			attr->ia_mode &= ~S_ISGID;
	}

	/* Check for setting the inode time. */
	if (ia_valid & (ATTR_MTIME_SET | ATTR_ATIME_SET | ATTR_TIMES_SET)) {
		if (!inode_owner_or_capable(inode))
			return -EPERM;
	}

	return 0;
}
EXPORT_SYMBOL(inode_change_ok);

/**
 * inode_newsize_ok - may this inode be truncated to a given size
 * @inode:	the inode to be truncated
 * @offset:	the new size to assign to the inode
 * @Returns:	0 on success, -ve errno on failure
 *
 * inode_newsize_ok must be called with i_mutex held.
 *
 * inode_newsize_ok will check filesystem limits and ulimits to check that the
 * new inode size is within limits. inode_newsize_ok will also send SIGXFSZ
 * when necessary. Caller must not proceed with inode size change if failure is
 * returned. @inode must be a file (not directory), with appropriate
 * permissions to allow truncate (inode_newsize_ok does NOT check these
 * conditions).
 */
int inode_newsize_ok(const struct inode *inode, loff_t offset)
{
	if (inode->i_size < offset) {
		unsigned long limit;

		limit = rlimit(RLIMIT_FSIZE);
		if (limit != RLIM_INFINITY && offset > limit)
			goto out_sig;
		if (offset > inode->i_sb->s_maxbytes)
			goto out_big;
	} else {
		/*
		 * truncation of in-use swapfiles is disallowed - it would
		 * cause subsequent swapout to scribble on the now-freed
		 * blocks.
		 */
		if (IS_SWAPFILE(inode))
			return -ETXTBSY;
	}
	if (ia_valid & ATTR_MODE) {
		umode_t mode = attr->ia_mode;

		if (!in_group_p(inode->i_gid) &&
		    !inode_capable(inode, CAP_FSETID))
			mode &= ~S_ISGID;
		inode->i_mode = mode;
	}
}
EXPORT_SYMBOL(setattr_copy);

/**
 * notify_change - modify attributes of a filesytem object
 * @dentry:	object affected
 * @iattr:	new attributes
 * @delegated_inode: returns inode, if the inode is delegated
 *
 * The caller must hold the i_mutex on the affected object.
 *
 * If notify_change discovers a delegation in need of breaking,
 * it will return -EWOULDBLOCK and return a reference to the inode in
 * delegated_inode.  The caller should then break the delegation and
 * retry.  Because breaking a delegation may take a long time, the
 * caller should drop the i_mutex before doing so.
 *
 * Alternatively, a caller may pass NULL for delegated_inode.  This may
 * be appropriate for callers that expect the underlying filesystem not
 * to be NFS exported.  Also, passing NULL is fine for callers holding
 * the file open for write, as there can be no conflicting delegation in
 * that case.
 */
int notify_change(struct dentry * dentry, struct iattr * attr, struct inode **delegated_inode)
{
	struct inode *inode = dentry->d_inode;
	umode_t mode = inode->i_mode;
	int error;
	struct timespec now;
	unsigned int ia_valid = attr->ia_valid;

	WARN_ON_ONCE(!mutex_is_locked(&inode->i_mutex));

	if (ia_valid & (ATTR_MODE | ATTR_UID | ATTR_GID | ATTR_TIMES_SET)) {
		if (IS_IMMUTABLE(inode) || IS_APPEND(inode))
			return -EPERM;
	}

	if ((ia_valid & ATTR_MODE)) {
		umode_t amode = attr->ia_mode;
		/* Flag setting protected by i_mutex */
		if (is_sxid(amode))
			inode->i_flags &= ~S_NOSEC;
	}

	now = current_fs_time(inode->i_sb);

	attr->ia_ctime = now;
	if (!(ia_valid & ATTR_ATIME_SET))
		attr->ia_atime = now;
	if (!(ia_valid & ATTR_MTIME_SET))
		attr->ia_mtime = now;
	if (ia_valid & ATTR_KILL_PRIV) {
		attr->ia_valid &= ~ATTR_KILL_PRIV;
		ia_valid &= ~ATTR_KILL_PRIV;
		error = security_inode_need_killpriv(dentry);
		if (error > 0)
			error = security_inode_killpriv(dentry);
		if (error)
			return error;
	}

	/*
	 * We now pass ATTR_KILL_S*ID to the lower level setattr function so
	 * that the function has the ability to reinterpret a mode change
	 * that's due to these bits. This adds an implicit restriction that
	 * no function will ever call notify_change with both ATTR_MODE and
	 * ATTR_KILL_S*ID set.
	 */
	if ((ia_valid & (ATTR_KILL_SUID|ATTR_KILL_SGID)) &&
	    (ia_valid & ATTR_MODE))
		BUG();

	if (ia_valid & ATTR_KILL_SUID) {
		if (mode & S_ISUID) {
			ia_valid = attr->ia_valid |= ATTR_MODE;
			attr->ia_mode = (inode->i_mode & ~S_ISUID);
		}
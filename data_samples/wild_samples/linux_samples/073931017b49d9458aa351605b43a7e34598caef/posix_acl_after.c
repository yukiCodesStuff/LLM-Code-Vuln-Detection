	if (!p || p == ERR_PTR(-EOPNOTSUPP)) {
		*mode &= ~current_umask();
		return 0;
	}
	if (IS_ERR(p))
		return PTR_ERR(p);

	clone = posix_acl_clone(p, GFP_NOFS);
	if (!clone)
		goto no_mem;

	ret = posix_acl_create_masq(clone, mode);
	if (ret < 0)
		goto no_mem_clone;

	if (ret == 0)
		posix_acl_release(clone);
	else
		*acl = clone;

	if (!S_ISDIR(*mode))
		posix_acl_release(p);
	else
		*default_acl = p;

	return 0;

no_mem_clone:
	posix_acl_release(clone);
no_mem:
	posix_acl_release(p);
	return -ENOMEM;
}
EXPORT_SYMBOL_GPL(posix_acl_create);

/**
 * posix_acl_update_mode  -  update mode in set_acl
 *
 * Update the file mode when setting an ACL: compute the new file permission
 * bits based on the ACL.  In addition, if the ACL is equivalent to the new
 * file mode, set *acl to NULL to indicate that no ACL should be set.
 *
 * As with chmod, clear the setgit bit if the caller is not in the owning group
 * or capable of CAP_FSETID (see inode_change_ok).
 *
 * Called from set_acl inode operations.
 */
int posix_acl_update_mode(struct inode *inode, umode_t *mode_p,
			  struct posix_acl **acl)
{
	umode_t mode = inode->i_mode;
	int error;

	error = posix_acl_equiv_mode(*acl, &mode);
	if (error < 0)
		return error;
	if (error == 0)
		*acl = NULL;
	if (!in_group_p(inode->i_gid) &&
	    !capable_wrt_inode_uidgid(inode, CAP_FSETID))
		mode &= ~S_ISGID;
	*mode_p = mode;
	return 0;
}
	switch (type) {
	case ACL_TYPE_ACCESS:
		name = XATTR_NAME_POSIX_ACL_ACCESS;
		if (acl) {
			error = posix_acl_update_mode(inode, &inode->i_mode, &acl);
			if (error)
				return error;
		}
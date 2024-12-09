		return error;

	if (type == ACL_TYPE_ACCESS) {
		umode_t mode;

		error = posix_acl_update_mode(inode, &mode, &acl);
		if (error)
			return error;
		error = xfs_set_mode(inode, mode);
		if (error)
			return error;
	}
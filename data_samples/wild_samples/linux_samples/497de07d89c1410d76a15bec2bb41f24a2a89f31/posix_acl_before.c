{
	int error;

	if (type == ACL_TYPE_ACCESS) {
		error = posix_acl_equiv_mode(acl, &inode->i_mode);
		if (error < 0)
			return 0;
		if (error == 0)
			acl = NULL;
	}
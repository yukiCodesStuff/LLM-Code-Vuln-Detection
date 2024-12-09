	switch (type) {
	case ACL_TYPE_ACCESS:
		name = XATTR_NAME_POSIX_ACL_ACCESS;
		if (acl) {
			umode_t mode;

			error = posix_acl_update_mode(inode, &mode, &acl);
			if (error) {
				gossip_err("%s: posix_acl_update_mode err: %d\n",
					   __func__,
					   error);
				return error;
			}

			if (inode->i_mode != mode)
				SetModeFlag(orangefs_inode);
			inode->i_mode = mode;
			mark_inode_dirty_sync(inode);
		}
			if (error) {
				gossip_err("%s: posix_acl_update_mode err: %d\n",
					   __func__,
					   error);
				return error;
			}

			if (inode->i_mode != mode)
				SetModeFlag(orangefs_inode);
			inode->i_mode = mode;
			mark_inode_dirty_sync(inode);
		}
		break;
	case ACL_TYPE_DEFAULT:
		name = XATTR_NAME_POSIX_ACL_DEFAULT;
		break;
	default:
		gossip_err("%s: invalid type %d!\n", __func__, type);
		return -EINVAL;
	}

	gossip_debug(GOSSIP_ACL_DEBUG,
		     "%s: inode %pU, key %s type %d\n",
		     __func__, get_khandle_from_ino(inode),
		     name,
		     type);

	if (acl) {
		size = posix_acl_xattr_size(acl->a_count);
		value = kmalloc(size, GFP_KERNEL);
		if (!value)
			return -ENOMEM;

		error = posix_acl_to_xattr(&init_user_ns, acl, value, size);
		if (error < 0)
			goto out;
	}

	gossip_debug(GOSSIP_ACL_DEBUG,
		     "%s: name %s, value %p, size %zd, acl %p\n",
		     __func__, name, value, size, acl);
	/*
	 * Go ahead and set the extended attribute now. NOTE: Suppose acl
	 * was NULL, then value will be NULL and size will be 0 and that
	 * will xlate to a removexattr. However, we don't want removexattr
	 * complain if attributes does not exist.
	 */
	error = orangefs_inode_setxattr(inode, name, value, size, 0);

out:
	kfree(value);
	if (!error)
		set_cached_acl(inode, type, acl);
	return error;
}

int orangefs_init_acl(struct inode *inode, struct inode *dir)
{
	struct orangefs_inode_s *orangefs_inode = ORANGEFS_I(inode);
	struct posix_acl *default_acl, *acl;
	umode_t mode = inode->i_mode;
	int error = 0;

	ClearModeFlag(orangefs_inode);

	error = posix_acl_create(dir, &mode, &default_acl, &acl);
	if (error)
		return error;

	if (default_acl) {
		error = orangefs_set_acl(inode, default_acl, ACL_TYPE_DEFAULT);
		posix_acl_release(default_acl);
	}

	if (acl) {
		if (!error)
			error = orangefs_set_acl(inode, acl, ACL_TYPE_ACCESS);
		posix_acl_release(acl);
	}

	/* If mode of the inode was changed, then do a forcible ->setattr */
	if (mode != inode->i_mode) {
		SetModeFlag(orangefs_inode);
		inode->i_mode = mode;
		orangefs_flush_inode(inode);
	}

	return error;
}
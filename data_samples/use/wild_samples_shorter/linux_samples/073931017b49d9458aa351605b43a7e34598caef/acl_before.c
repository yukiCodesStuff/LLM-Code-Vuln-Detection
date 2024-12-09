	case ACL_TYPE_ACCESS:
		name = XATTR_NAME_POSIX_ACL_ACCESS;
		if (acl) {
			umode_t mode = inode->i_mode;
			/*
			 * can we represent this with the traditional file
			 * mode permission bits?
			 */
			error = posix_acl_equiv_mode(acl, &mode);
			if (error < 0) {
				gossip_err("%s: posix_acl_equiv_mode err: %d\n",
					   __func__,
					   error);
				return error;
			}
				SetModeFlag(orangefs_inode);
			inode->i_mode = mode;
			mark_inode_dirty_sync(inode);
			if (error == 0)
				acl = NULL;
		}
		break;
	case ACL_TYPE_DEFAULT:
		name = XATTR_NAME_POSIX_ACL_DEFAULT;
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
				SetModeFlag(orangefs_inode);
			inode->i_mode = mode;
			mark_inode_dirty_sync(inode);
		}
		break;
	case ACL_TYPE_DEFAULT:
		name = XATTR_NAME_POSIX_ACL_DEFAULT;
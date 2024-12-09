	if ((ia_valid & ATTR_UID) &&
	    (!uid_eq(current_fsuid(), inode->i_uid) ||
	     !uid_eq(attr->ia_uid, inode->i_uid)) &&
	    !capable_wrt_inode_uidgid(inode, CAP_CHOWN))
		return -EPERM;

	/* Make sure caller can chgrp. */
	if ((ia_valid & ATTR_GID) &&
	    (!uid_eq(current_fsuid(), inode->i_uid) ||
	    (!in_group_p(attr->ia_gid) && !gid_eq(attr->ia_gid, inode->i_gid))) &&
	    !capable_wrt_inode_uidgid(inode, CAP_CHOWN))
		return -EPERM;

	/* Make sure a caller can chmod. */
	if (ia_valid & ATTR_MODE) {
		/* Also check the setgid bit! */
		if (!in_group_p((ia_valid & ATTR_GID) ? attr->ia_gid :
				inode->i_gid) &&
		    !capable_wrt_inode_uidgid(inode, CAP_FSETID))
			attr->ia_mode &= ~S_ISGID;
	}

	/* Check for setting the inode time. */
		umode_t mode = attr->ia_mode;

		if (!in_group_p(inode->i_gid) &&
		    !capable_wrt_inode_uidgid(inode, CAP_FSETID))
			mode &= ~S_ISGID;
		inode->i_mode = mode;
	}
}
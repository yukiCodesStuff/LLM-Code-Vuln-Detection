	else {
		rc = cifs_get_inode_info(&newinode, full_path, buf, inode->i_sb,
					 xid, fid);
		if (newinode) {
			if (server->ops->set_lease_key)
				server->ops->set_lease_key(newinode, fid);
			if (cifs_sb->mnt_cifs_flags & CIFS_MOUNT_DYNPERM)
				newinode->i_mode = mode;
			if ((*oplock & CIFS_CREATE_ACTION) &&
			    (cifs_sb->mnt_cifs_flags & CIFS_MOUNT_SET_UID)) {
				newinode->i_uid = current_fsuid();
				if (inode->i_mode & S_ISGID)
					newinode->i_gid = inode->i_gid;
				else
					newinode->i_gid = current_fsgid();
			}
		}
	}
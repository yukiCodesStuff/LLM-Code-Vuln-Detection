					      xid);
	else {
		rc = cifs_get_inode_info(&newinode, full_path, buf, inode->i_sb,
					 xid, &fid->netfid);
		if (newinode) {
			if (server->ops->set_lease_key)
				server->ops->set_lease_key(newinode, fid);
			if (cifs_sb->mnt_cifs_flags & CIFS_MOUNT_DYNPERM)
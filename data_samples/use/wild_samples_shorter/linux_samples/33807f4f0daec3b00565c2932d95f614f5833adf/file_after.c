	struct cifsInodeInfo *cinode = CIFS_I(inode);
	struct TCP_Server_Info *server = tlink_tcon(cfile->tlink)->ses->server;
	ssize_t rc = -EACCES;
	loff_t lock_pos = pos;

	if (file->f_flags & O_APPEND)
		lock_pos = i_size_read(inode);
	/*
	 * We need to hold the sem to be sure nobody modifies lock list
	 * with a brlock that prevents writing.
	 */
	down_read(&cinode->lock_sem);
	if (!cifs_find_lock_conflict(cfile, lock_pos, iov_length(iov, nr_segs),
				     server->vals->exclusive_lock_type, NULL,
				     CIFS_WRITE_OP))
		rc = generic_file_aio_write(iocb, iov, nr_segs, pos);
	up_read(&cinode->lock_sem);
	return rc;
}

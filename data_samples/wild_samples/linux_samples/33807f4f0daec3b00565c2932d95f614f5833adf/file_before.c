{
	struct file *file = iocb->ki_filp;
	struct cifsFileInfo *cfile = (struct cifsFileInfo *)file->private_data;
	struct inode *inode = file->f_mapping->host;
	struct cifsInodeInfo *cinode = CIFS_I(inode);
	struct TCP_Server_Info *server = tlink_tcon(cfile->tlink)->ses->server;
	ssize_t rc = -EACCES;

	BUG_ON(iocb->ki_pos != pos);

	/*
	 * We need to hold the sem to be sure nobody modifies lock list
	 * with a brlock that prevents writing.
	 */
	down_read(&cinode->lock_sem);
	if (!cifs_find_lock_conflict(cfile, pos, iov_length(iov, nr_segs),
				     server->vals->exclusive_lock_type, NULL,
				     CIFS_WRITE_OP)) {
		mutex_lock(&inode->i_mutex);
		rc = __generic_file_aio_write(iocb, iov, nr_segs,
					       &iocb->ki_pos);
		mutex_unlock(&inode->i_mutex);
	}
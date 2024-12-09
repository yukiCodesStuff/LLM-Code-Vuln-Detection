{
	struct file *file = iocb->ki_filp;
	struct cifsFileInfo *cfile = (struct cifsFileInfo *)file->private_data;
	struct inode *inode = file->f_mapping->host;
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

ssize_t
cifs_strict_writev(struct kiocb *iocb, const struct iovec *iov,
		   unsigned long nr_segs, loff_t pos)
{
	struct inode *inode = file_inode(iocb->ki_filp);
	struct cifsInodeInfo *cinode = CIFS_I(inode);
	struct cifs_sb_info *cifs_sb = CIFS_SB(inode->i_sb);
	struct cifsFileInfo *cfile = (struct cifsFileInfo *)
						iocb->ki_filp->private_data;
	struct cifs_tcon *tcon = tlink_tcon(cfile->tlink);
	ssize_t written;

	if (CIFS_CACHE_WRITE(cinode)) {
		if (cap_unix(tcon->ses) &&
		(CIFS_UNIX_FCNTL_CAP & le64_to_cpu(tcon->fsUnixInfo.Capability))
		    && ((cifs_sb->mnt_cifs_flags & CIFS_MOUNT_NOPOSIXBRL) == 0))
			return generic_file_aio_write(iocb, iov, nr_segs, pos);
		return cifs_writev(iocb, iov, nr_segs, pos);
	}
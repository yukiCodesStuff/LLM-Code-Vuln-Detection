{
	unsigned long nr_pages, i;
	size_t copied, len, cur_len;
	ssize_t total_written = 0;
	loff_t offset;
	struct iov_iter it;
	struct cifsFileInfo *open_file;
	struct cifs_tcon *tcon;
	struct cifs_sb_info *cifs_sb;
	struct cifs_writedata *wdata, *tmp;
	struct list_head wdata_list;
	int rc;
	pid_t pid;

	len = iov_length(iov, nr_segs);
	if (!len)
		return 0;

	rc = generic_write_checks(file, poffset, &len, 0);
	if (rc)
		return rc;

	INIT_LIST_HEAD(&wdata_list);
	cifs_sb = CIFS_SB(file->f_path.dentry->d_sb);
	open_file = file->private_data;
	tcon = tlink_tcon(open_file->tlink);

	if (!tcon->ses->server->ops->async_writev)
		return -ENOSYS;

	offset = *poffset;

	if (cifs_sb->mnt_cifs_flags & CIFS_MOUNT_RWPIDFORWARD)
		pid = open_file->pid;
	else
		pid = current->tgid;

	iov_iter_init(&it, iov, nr_segs, len, 0);
	do {
		size_t save_len;

		nr_pages = get_numpages(cifs_sb->wsize, len, &cur_len);
		wdata = cifs_writedata_alloc(nr_pages,
					     cifs_uncached_writev_complete);
		if (!wdata) {
			rc = -ENOMEM;
			break;
		}

		rc = cifs_write_allocate_pages(wdata->pages, nr_pages);
		if (rc) {
			kfree(wdata);
			break;
		}

		save_len = cur_len;
		for (i = 0; i < nr_pages; i++) {
			copied = min_t(const size_t, cur_len, PAGE_SIZE);
			copied = iov_iter_copy_from_user(wdata->pages[i], &it,
							 0, copied);
			cur_len -= copied;
			iov_iter_advance(&it, copied);
		}
		cur_len = save_len - cur_len;

		wdata->sync_mode = WB_SYNC_ALL;
		wdata->nr_pages = nr_pages;
		wdata->offset = (__u64)offset;
		wdata->cfile = cifsFileInfo_get(open_file);
		wdata->pid = pid;
		wdata->bytes = cur_len;
		wdata->pagesz = PAGE_SIZE;
		wdata->tailsz = cur_len - ((nr_pages - 1) * PAGE_SIZE);
		rc = cifs_uncached_retry_writev(wdata);
		if (rc) {
			kref_put(&wdata->refcount,
				 cifs_uncached_writedata_release);
			break;
		}

		list_add_tail(&wdata->list, &wdata_list);
		offset += cur_len;
		len -= cur_len;
	} while (len > 0);

	/*
	 * If at least one write was successfully sent, then discard any rc
	 * value from the later writes. If the other write succeeds, then
	 * we'll end up returning whatever was written. If it fails, then
	 * we'll get a new rc value from that.
	 */
	if (!list_empty(&wdata_list))
		rc = 0;

	/*
	 * Wait for and collect replies for any successful sends in order of
	 * increasing offset. Once an error is hit or we get a fatal signal
	 * while waiting, then return without waiting for any more replies.
	 */
restart_loop:
	list_for_each_entry_safe(wdata, tmp, &wdata_list, list) {
		if (!rc) {
			/* FIXME: freezable too? */
			rc = wait_for_completion_killable(&wdata->done);
			if (rc)
				rc = -EINTR;
			else if (wdata->result)
				rc = wdata->result;
			else
				total_written += wdata->bytes;

			/* resend call if it's a retryable error */
			if (rc == -EAGAIN) {
				rc = cifs_uncached_retry_writev(wdata);
				goto restart_loop;
			}
		}
		list_del_init(&wdata->list);
		kref_put(&wdata->refcount, cifs_uncached_writedata_release);
	}

	if (total_written > 0)
		*poffset += total_written;

	cifs_stats_bytes_written(tcon, total_written);
	return total_written ? total_written : (ssize_t)rc;
}

ssize_t cifs_user_writev(struct kiocb *iocb, const struct iovec *iov,
				unsigned long nr_segs, loff_t pos)
{
	ssize_t written;
	struct inode *inode;

	inode = file_inode(iocb->ki_filp);

	/*
	 * BB - optimize the way when signing is disabled. We can drop this
	 * extra memory-to-memory copying and use iovec buffers for constructing
	 * write request.
	 */

	written = cifs_iovec_write(iocb->ki_filp, iov, nr_segs, &pos);
	if (written > 0) {
		CIFS_I(inode)->invalid_mapping = true;
		iocb->ki_pos = pos;
	}

	return written;
}

static ssize_t
cifs_writev(struct kiocb *iocb, const struct iovec *iov,
	    unsigned long nr_segs, loff_t pos)
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

	if (rc > 0) {
		ssize_t err;

		err = generic_write_sync(file, pos, rc);
		if (err < 0 && rc > 0)
			rc = err;
	}

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
	/*
	 * For non-oplocked files in strict cache mode we need to write the data
	 * to the server exactly from the pos to pos+len-1 rather than flush all
	 * affected pages because it may cause a error with mandatory locks on
	 * these pages but not on the region from pos to ppos+len-1.
	 */
	written = cifs_user_writev(iocb, iov, nr_segs, pos);
	if (written > 0 && CIFS_CACHE_READ(cinode)) {
		/*
		 * Windows 7 server can delay breaking level2 oplock if a write
		 * request comes - break it on the client to prevent reading
		 * an old data.
		 */
		cifs_invalidate_mapping(inode);
		cifs_dbg(FYI, "Set no oplock for inode=%p after a write operation\n",
			 inode);
		cinode->oplock = 0;
	}
	return written;
}

static struct cifs_readdata *
cifs_readdata_alloc(unsigned int nr_pages, work_func_t complete)
{
	struct cifs_readdata *rdata;

	rdata = kzalloc(sizeof(*rdata) + (sizeof(struct page *) * nr_pages),
			GFP_KERNEL);
	if (rdata != NULL) {
		kref_init(&rdata->refcount);
		INIT_LIST_HEAD(&rdata->list);
		init_completion(&rdata->done);
		INIT_WORK(&rdata->work, complete);
	}

	return rdata;
}

void
cifs_readdata_release(struct kref *refcount)
{
	struct cifs_readdata *rdata = container_of(refcount,
					struct cifs_readdata, refcount);

	if (rdata->cfile)
		cifsFileInfo_put(rdata->cfile);

	kfree(rdata);
}

static int
cifs_read_allocate_pages(struct cifs_readdata *rdata, unsigned int nr_pages)
{
	int rc = 0;
	struct page *page;
	unsigned int i;

	for (i = 0; i < nr_pages; i++) {
		page = alloc_page(GFP_KERNEL|__GFP_HIGHMEM);
		if (!page) {
			rc = -ENOMEM;
			break;
		}
		rdata->pages[i] = page;
	}

	if (rc) {
		for (i = 0; i < nr_pages; i++) {
			put_page(rdata->pages[i]);
			rdata->pages[i] = NULL;
		}
	}
	return rc;
}

static void
cifs_uncached_readdata_release(struct kref *refcount)
{
	struct cifs_readdata *rdata = container_of(refcount,
					struct cifs_readdata, refcount);
	unsigned int i;

	for (i = 0; i < rdata->nr_pages; i++) {
		put_page(rdata->pages[i]);
		rdata->pages[i] = NULL;
	}
	cifs_readdata_release(refcount);
}

static int
cifs_retry_async_readv(struct cifs_readdata *rdata)
{
	int rc;
	struct TCP_Server_Info *server;

	server = tlink_tcon(rdata->cfile->tlink)->ses->server;

	do {
		if (rdata->cfile->invalidHandle) {
			rc = cifs_reopen_file(rdata->cfile, true);
			if (rc != 0)
				continue;
		}
		rc = server->ops->async_readv(rdata);
	} while (rc == -EAGAIN);

	return rc;
}

/**
 * cifs_readdata_to_iov - copy data from pages in response to an iovec
 * @rdata:	the readdata response with list of pages holding data
 * @iov:	vector in which we should copy the data
 * @nr_segs:	number of segments in vector
 * @offset:	offset into file of the first iovec
 * @copied:	used to return the amount of data copied to the iov
 *
 * This function copies data from a list of pages in a readdata response into
 * an array of iovecs. It will first calculate where the data should go
 * based on the info in the readdata and then copy the data into that spot.
 */
static ssize_t
cifs_readdata_to_iov(struct cifs_readdata *rdata, const struct iovec *iov,
			unsigned long nr_segs, loff_t offset, ssize_t *copied)
{
	int rc = 0;
	struct iov_iter ii;
	size_t pos = rdata->offset - offset;
	ssize_t remaining = rdata->bytes;
	unsigned char *pdata;
	unsigned int i;

	/* set up iov_iter and advance to the correct offset */
	iov_iter_init(&ii, iov, nr_segs, iov_length(iov, nr_segs), 0);
	iov_iter_advance(&ii, pos);

	*copied = 0;
	for (i = 0; i < rdata->nr_pages; i++) {
		ssize_t copy;
		struct page *page = rdata->pages[i];

		/* copy a whole page or whatever's left */
		copy = min_t(ssize_t, remaining, PAGE_SIZE);

		/* ...but limit it to whatever space is left in the iov */
		copy = min_t(ssize_t, copy, iov_iter_count(&ii));

		/* go while there's data to be copied and no errors */
		if (copy && !rc) {
			pdata = kmap(page);
			rc = memcpy_toiovecend(ii.iov, pdata, ii.iov_offset,
						(int)copy);
			kunmap(page);
			if (!rc) {
				*copied += copy;
				remaining -= copy;
				iov_iter_advance(&ii, copy);
			}
		}
	}

	return rc;
}

static void
cifs_uncached_readv_complete(struct work_struct *work)
{
	struct cifs_readdata *rdata = container_of(work,
						struct cifs_readdata, work);

	complete(&rdata->done);
	kref_put(&rdata->refcount, cifs_uncached_readdata_release);
}

static int
cifs_uncached_read_into_pages(struct TCP_Server_Info *server,
			struct cifs_readdata *rdata, unsigned int len)
{
	int total_read = 0, result = 0;
	unsigned int i;
	unsigned int nr_pages = rdata->nr_pages;
	struct kvec iov;

	rdata->tailsz = PAGE_SIZE;
	for (i = 0; i < nr_pages; i++) {
		struct page *page = rdata->pages[i];

		if (len >= PAGE_SIZE) {
			/* enough data to fill the page */
			iov.iov_base = kmap(page);
			iov.iov_len = PAGE_SIZE;
			cifs_dbg(FYI, "%u: iov_base=%p iov_len=%zu\n",
				 i, iov.iov_base, iov.iov_len);
			len -= PAGE_SIZE;
		} else if (len > 0) {
			/* enough for partial page, fill and zero the rest */
			iov.iov_base = kmap(page);
			iov.iov_len = len;
			cifs_dbg(FYI, "%u: iov_base=%p iov_len=%zu\n",
				 i, iov.iov_base, iov.iov_len);
			memset(iov.iov_base + len, '\0', PAGE_SIZE - len);
			rdata->tailsz = len;
			len = 0;
		} else {
			/* no need to hold page hostage */
			rdata->pages[i] = NULL;
			rdata->nr_pages--;
			put_page(page);
			continue;
		}

		result = cifs_readv_from_socket(server, &iov, 1, iov.iov_len);
		kunmap(page);
		if (result < 0)
			break;

		total_read += result;
	}

	return total_read > 0 ? total_read : result;
}

static ssize_t
cifs_iovec_read(struct file *file, const struct iovec *iov,
		 unsigned long nr_segs, loff_t *poffset)
{
	ssize_t rc;
	size_t len, cur_len;
	ssize_t total_read = 0;
	loff_t offset = *poffset;
	unsigned int npages;
	struct cifs_sb_info *cifs_sb;
	struct cifs_tcon *tcon;
	struct cifsFileInfo *open_file;
	struct cifs_readdata *rdata, *tmp;
	struct list_head rdata_list;
	pid_t pid;

	if (!nr_segs)
		return 0;

	len = iov_length(iov, nr_segs);
	if (!len)
		return 0;

	INIT_LIST_HEAD(&rdata_list);
	cifs_sb = CIFS_SB(file->f_path.dentry->d_sb);
	open_file = file->private_data;
	tcon = tlink_tcon(open_file->tlink);

	if (!tcon->ses->server->ops->async_readv)
		return -ENOSYS;

	if (cifs_sb->mnt_cifs_flags & CIFS_MOUNT_RWPIDFORWARD)
		pid = open_file->pid;
	else
		pid = current->tgid;

	if ((file->f_flags & O_ACCMODE) == O_WRONLY)
		cifs_dbg(FYI, "attempting read on write only file instance\n");

	do {
		cur_len = min_t(const size_t, len - total_read, cifs_sb->rsize);
		npages = DIV_ROUND_UP(cur_len, PAGE_SIZE);

		/* allocate a readdata struct */
		rdata = cifs_readdata_alloc(npages,
					    cifs_uncached_readv_complete);
		if (!rdata) {
			rc = -ENOMEM;
			goto error;
		}

		rc = cifs_read_allocate_pages(rdata, npages);
		if (rc)
			goto error;

		rdata->cfile = cifsFileInfo_get(open_file);
		rdata->nr_pages = npages;
		rdata->offset = offset;
		rdata->bytes = cur_len;
		rdata->pid = pid;
		rdata->pagesz = PAGE_SIZE;
		rdata->read_into_pages = cifs_uncached_read_into_pages;

		rc = cifs_retry_async_readv(rdata);
error:
		if (rc) {
			kref_put(&rdata->refcount,
				 cifs_uncached_readdata_release);
			break;
		}

		list_add_tail(&rdata->list, &rdata_list);
		offset += cur_len;
		len -= cur_len;
	} while (len > 0);

	/* if at least one read request send succeeded, then reset rc */
	if (!list_empty(&rdata_list))
		rc = 0;

	/* the loop below should proceed in the order of increasing offsets */
restart_loop:
	list_for_each_entry_safe(rdata, tmp, &rdata_list, list) {
		if (!rc) {
			ssize_t copied;

			/* FIXME: freezable sleep too? */
			rc = wait_for_completion_killable(&rdata->done);
			if (rc)
				rc = -EINTR;
			else if (rdata->result)
				rc = rdata->result;
			else {
				rc = cifs_readdata_to_iov(rdata, iov,
							nr_segs, *poffset,
							&copied);
				total_read += copied;
			}

			/* resend call if it's a retryable error */
			if (rc == -EAGAIN) {
				rc = cifs_retry_async_readv(rdata);
				goto restart_loop;
			}
		}
		list_del_init(&rdata->list);
		kref_put(&rdata->refcount, cifs_uncached_readdata_release);
	}

	cifs_stats_bytes_read(tcon, total_read);
	*poffset += total_read;

	/* mask nodata case */
	if (rc == -ENODATA)
		rc = 0;

	return total_read ? total_read : rc;
}

ssize_t cifs_user_readv(struct kiocb *iocb, const struct iovec *iov,
			       unsigned long nr_segs, loff_t pos)
{
	ssize_t read;

	read = cifs_iovec_read(iocb->ki_filp, iov, nr_segs, &pos);
	if (read > 0)
		iocb->ki_pos = pos;

	return read;
}

ssize_t
cifs_strict_readv(struct kiocb *iocb, const struct iovec *iov,
		  unsigned long nr_segs, loff_t pos)
{
	struct inode *inode = file_inode(iocb->ki_filp);
	struct cifsInodeInfo *cinode = CIFS_I(inode);
	struct cifs_sb_info *cifs_sb = CIFS_SB(inode->i_sb);
	struct cifsFileInfo *cfile = (struct cifsFileInfo *)
						iocb->ki_filp->private_data;
	struct cifs_tcon *tcon = tlink_tcon(cfile->tlink);
	int rc = -EACCES;

	/*
	 * In strict cache mode we need to read from the server all the time
	 * if we don't have level II oplock because the server can delay mtime
	 * change - so we can't make a decision about inode invalidating.
	 * And we can also fail with pagereading if there are mandatory locks
	 * on pages affected by this read but not on the region from pos to
	 * pos+len-1.
	 */
	if (!CIFS_CACHE_READ(cinode))
		return cifs_user_readv(iocb, iov, nr_segs, pos);

	if (cap_unix(tcon->ses) &&
	    (CIFS_UNIX_FCNTL_CAP & le64_to_cpu(tcon->fsUnixInfo.Capability)) &&
	    ((cifs_sb->mnt_cifs_flags & CIFS_MOUNT_NOPOSIXBRL) == 0))
		return generic_file_aio_read(iocb, iov, nr_segs, pos);

	/*
	 * We need to hold the sem to be sure nobody modifies lock list
	 * with a brlock that prevents reading.
	 */
	down_read(&cinode->lock_sem);
	if (!cifs_find_lock_conflict(cfile, pos, iov_length(iov, nr_segs),
				     tcon->ses->server->vals->shared_lock_type,
				     NULL, CIFS_READ_OP))
		rc = generic_file_aio_read(iocb, iov, nr_segs, pos);
	up_read(&cinode->lock_sem);
	return rc;
}

static ssize_t
cifs_read(struct file *file, char *read_data, size_t read_size, loff_t *offset)
{
	int rc = -EACCES;
	unsigned int bytes_read = 0;
	unsigned int total_read;
	unsigned int current_read_size;
	unsigned int rsize;
	struct cifs_sb_info *cifs_sb;
	struct cifs_tcon *tcon;
	struct TCP_Server_Info *server;
	unsigned int xid;
	char *cur_offset;
	struct cifsFileInfo *open_file;
	struct cifs_io_parms io_parms;
	int buf_type = CIFS_NO_BUFFER;
	__u32 pid;

	xid = get_xid();
	cifs_sb = CIFS_SB(file->f_path.dentry->d_sb);

	/* FIXME: set up handlers for larger reads and/or convert to async */
	rsize = min_t(unsigned int, cifs_sb->rsize, CIFSMaxBufSize);

	if (file->private_data == NULL) {
		rc = -EBADF;
		free_xid(xid);
		return rc;
	}
	open_file = file->private_data;
	tcon = tlink_tcon(open_file->tlink);
	server = tcon->ses->server;

	if (!server->ops->sync_read) {
		free_xid(xid);
		return -ENOSYS;
	}

	if (cifs_sb->mnt_cifs_flags & CIFS_MOUNT_RWPIDFORWARD)
		pid = open_file->pid;
	else
		pid = current->tgid;

	if ((file->f_flags & O_ACCMODE) == O_WRONLY)
		cifs_dbg(FYI, "attempting read on write only file instance\n");

	for (total_read = 0, cur_offset = read_data; read_size > total_read;
	     total_read += bytes_read, cur_offset += bytes_read) {
		current_read_size = min_t(uint, read_size - total_read, rsize);
		/*
		 * For windows me and 9x we do not want to request more than it
		 * negotiated since it will refuse the read then.
		 */
		if ((tcon->ses) && !(tcon->ses->capabilities &
				tcon->ses->server->vals->cap_large_files)) {
			current_read_size = min_t(uint, current_read_size,
					CIFSMaxBufSize);
		}
		rc = -EAGAIN;
		while (rc == -EAGAIN) {
			if (open_file->invalidHandle) {
				rc = cifs_reopen_file(open_file, true);
				if (rc != 0)
					break;
			}
			io_parms.pid = pid;
			io_parms.tcon = tcon;
			io_parms.offset = *offset;
			io_parms.length = current_read_size;
			rc = server->ops->sync_read(xid, open_file, &io_parms,
						    &bytes_read, &cur_offset,
						    &buf_type);
		}
		if (rc || (bytes_read == 0)) {
			if (total_read) {
				break;
			} else {
				free_xid(xid);
				return rc;
			}
		} else {
			cifs_stats_bytes_read(tcon, total_read);
			*offset += bytes_read;
		}
	}
	free_xid(xid);
	return total_read;
}

/*
 * If the page is mmap'ed into a process' page tables, then we need to make
 * sure that it doesn't change while being written back.
 */
static int
cifs_page_mkwrite(struct vm_area_struct *vma, struct vm_fault *vmf)
{
	struct page *page = vmf->page;

	lock_page(page);
	return VM_FAULT_LOCKED;
}

static struct vm_operations_struct cifs_file_vm_ops = {
	.fault = filemap_fault,
	.page_mkwrite = cifs_page_mkwrite,
	.remap_pages = generic_file_remap_pages,
};

int cifs_file_strict_mmap(struct file *file, struct vm_area_struct *vma)
{
	int rc, xid;
	struct inode *inode = file_inode(file);

	xid = get_xid();

	if (!CIFS_CACHE_READ(CIFS_I(inode))) {
		rc = cifs_invalidate_mapping(inode);
		if (rc)
			return rc;
	}

	rc = generic_file_mmap(file, vma);
	if (rc == 0)
		vma->vm_ops = &cifs_file_vm_ops;
	free_xid(xid);
	return rc;
}

int cifs_file_mmap(struct file *file, struct vm_area_struct *vma)
{
	int rc, xid;

	xid = get_xid();
	rc = cifs_revalidate_file(file);
	if (rc) {
		cifs_dbg(FYI, "Validation prior to mmap failed, error=%d\n",
			 rc);
		free_xid(xid);
		return rc;
	}
	rc = generic_file_mmap(file, vma);
	if (rc == 0)
		vma->vm_ops = &cifs_file_vm_ops;
	free_xid(xid);
	return rc;
}

static void
cifs_readv_complete(struct work_struct *work)
{
	unsigned int i;
	struct cifs_readdata *rdata = container_of(work,
						struct cifs_readdata, work);

	for (i = 0; i < rdata->nr_pages; i++) {
		struct page *page = rdata->pages[i];

		lru_cache_add_file(page);

		if (rdata->result == 0) {
			flush_dcache_page(page);
			SetPageUptodate(page);
		}

		unlock_page(page);

		if (rdata->result == 0)
			cifs_readpage_to_fscache(rdata->mapping->host, page);

		page_cache_release(page);
		rdata->pages[i] = NULL;
	}
	kref_put(&rdata->refcount, cifs_readdata_release);
}

static int
cifs_readpages_read_into_pages(struct TCP_Server_Info *server,
			struct cifs_readdata *rdata, unsigned int len)
{
	int total_read = 0, result = 0;
	unsigned int i;
	u64 eof;
	pgoff_t eof_index;
	unsigned int nr_pages = rdata->nr_pages;
	struct kvec iov;

	/* determine the eof that the server (probably) has */
	eof = CIFS_I(rdata->mapping->host)->server_eof;
	eof_index = eof ? (eof - 1) >> PAGE_CACHE_SHIFT : 0;
	cifs_dbg(FYI, "eof=%llu eof_index=%lu\n", eof, eof_index);

	rdata->tailsz = PAGE_CACHE_SIZE;
	for (i = 0; i < nr_pages; i++) {
		struct page *page = rdata->pages[i];

		if (len >= PAGE_CACHE_SIZE) {
			/* enough data to fill the page */
			iov.iov_base = kmap(page);
			iov.iov_len = PAGE_CACHE_SIZE;
			cifs_dbg(FYI, "%u: idx=%lu iov_base=%p iov_len=%zu\n",
				 i, page->index, iov.iov_base, iov.iov_len);
			len -= PAGE_CACHE_SIZE;
		} else if (len > 0) {
			/* enough for partial page, fill and zero the rest */
			iov.iov_base = kmap(page);
			iov.iov_len = len;
			cifs_dbg(FYI, "%u: idx=%lu iov_base=%p iov_len=%zu\n",
				 i, page->index, iov.iov_base, iov.iov_len);
			memset(iov.iov_base + len,
				'\0', PAGE_CACHE_SIZE - len);
			rdata->tailsz = len;
			len = 0;
		} else if (page->index > eof_index) {
			/*
			 * The VFS will not try to do readahead past the
			 * i_size, but it's possible that we have outstanding
			 * writes with gaps in the middle and the i_size hasn't
			 * caught up yet. Populate those with zeroed out pages
			 * to prevent the VFS from repeatedly attempting to
			 * fill them until the writes are flushed.
			 */
			zero_user(page, 0, PAGE_CACHE_SIZE);
			lru_cache_add_file(page);
			flush_dcache_page(page);
			SetPageUptodate(page);
			unlock_page(page);
			page_cache_release(page);
			rdata->pages[i] = NULL;
			rdata->nr_pages--;
			continue;
		} else {
			/* no need to hold page hostage */
			lru_cache_add_file(page);
			unlock_page(page);
			page_cache_release(page);
			rdata->pages[i] = NULL;
			rdata->nr_pages--;
			continue;
		}

		result = cifs_readv_from_socket(server, &iov, 1, iov.iov_len);
		kunmap(page);
		if (result < 0)
			break;

		total_read += result;
	}

	return total_read > 0 ? total_read : result;
}

static int cifs_readpages(struct file *file, struct address_space *mapping,
	struct list_head *page_list, unsigned num_pages)
{
	int rc;
	struct list_head tmplist;
	struct cifsFileInfo *open_file = file->private_data;
	struct cifs_sb_info *cifs_sb = CIFS_SB(file->f_path.dentry->d_sb);
	unsigned int rsize = cifs_sb->rsize;
	pid_t pid;

	/*
	 * Give up immediately if rsize is too small to read an entire page.
	 * The VFS will fall back to readpage. We should never reach this
	 * point however since we set ra_pages to 0 when the rsize is smaller
	 * than a cache page.
	 */
	if (unlikely(rsize < PAGE_CACHE_SIZE))
		return 0;

	/*
	 * Reads as many pages as possible from fscache. Returns -ENOBUFS
	 * immediately if the cookie is negative
	 *
	 * After this point, every page in the list might have PG_fscache set,
	 * so we will need to clean that up off of every page we don't use.
	 */
	rc = cifs_readpages_from_fscache(mapping->host, mapping, page_list,
					 &num_pages);
	if (rc == 0)
		return rc;

	if (cifs_sb->mnt_cifs_flags & CIFS_MOUNT_RWPIDFORWARD)
		pid = open_file->pid;
	else
		pid = current->tgid;

	rc = 0;
	INIT_LIST_HEAD(&tmplist);

	cifs_dbg(FYI, "%s: file=%p mapping=%p num_pages=%u\n",
		 __func__, file, mapping, num_pages);

	/*
	 * Start with the page at end of list and move it to private
	 * list. Do the same with any following pages until we hit
	 * the rsize limit, hit an index discontinuity, or run out of
	 * pages. Issue the async read and then start the loop again
	 * until the list is empty.
	 *
	 * Note that list order is important. The page_list is in
	 * the order of declining indexes. When we put the pages in
	 * the rdata->pages, then we want them in increasing order.
	 */
	while (!list_empty(page_list)) {
		unsigned int i;
		unsigned int bytes = PAGE_CACHE_SIZE;
		unsigned int expected_index;
		unsigned int nr_pages = 1;
		loff_t offset;
		struct page *page, *tpage;
		struct cifs_readdata *rdata;

		page = list_entry(page_list->prev, struct page, lru);

		/*
		 * Lock the page and put it in the cache. Since no one else
		 * should have access to this page, we're safe to simply set
		 * PG_locked without checking it first.
		 */
		__set_page_locked(page);
		rc = add_to_page_cache_locked(page, mapping,
					      page->index, GFP_KERNEL);

		/* give up if we can't stick it in the cache */
		if (rc) {
			__clear_page_locked(page);
			break;
		}

		/* move first page to the tmplist */
		offset = (loff_t)page->index << PAGE_CACHE_SHIFT;
		list_move_tail(&page->lru, &tmplist);

		/* now try and add more pages onto the request */
		expected_index = page->index + 1;
		list_for_each_entry_safe_reverse(page, tpage, page_list, lru) {
			/* discontinuity ? */
			if (page->index != expected_index)
				break;

			/* would this page push the read over the rsize? */
			if (bytes + PAGE_CACHE_SIZE > rsize)
				break;

			__set_page_locked(page);
			if (add_to_page_cache_locked(page, mapping,
						page->index, GFP_KERNEL)) {
				__clear_page_locked(page);
				break;
			}
			list_move_tail(&page->lru, &tmplist);
			bytes += PAGE_CACHE_SIZE;
			expected_index++;
			nr_pages++;
		}

		rdata = cifs_readdata_alloc(nr_pages, cifs_readv_complete);
		if (!rdata) {
			/* best to give up if we're out of mem */
			list_for_each_entry_safe(page, tpage, &tmplist, lru) {
				list_del(&page->lru);
				lru_cache_add_file(page);
				unlock_page(page);
				page_cache_release(page);
			}
			rc = -ENOMEM;
			break;
		}

		rdata->cfile = cifsFileInfo_get(open_file);
		rdata->mapping = mapping;
		rdata->offset = offset;
		rdata->bytes = bytes;
		rdata->pid = pid;
		rdata->pagesz = PAGE_CACHE_SIZE;
		rdata->read_into_pages = cifs_readpages_read_into_pages;

		list_for_each_entry_safe(page, tpage, &tmplist, lru) {
			list_del(&page->lru);
			rdata->pages[rdata->nr_pages++] = page;
		}

		rc = cifs_retry_async_readv(rdata);
		if (rc != 0) {
			for (i = 0; i < rdata->nr_pages; i++) {
				page = rdata->pages[i];
				lru_cache_add_file(page);
				unlock_page(page);
				page_cache_release(page);
			}
			kref_put(&rdata->refcount, cifs_readdata_release);
			break;
		}

		kref_put(&rdata->refcount, cifs_readdata_release);
	}

	/* Any pages that have been shown to fscache but didn't get added to
	 * the pagecache must be uncached before they get returned to the
	 * allocator.
	 */
	cifs_fscache_readpages_cancel(mapping->host, page_list);
	return rc;
}

/*
 * cifs_readpage_worker must be called with the page pinned
 */
static int cifs_readpage_worker(struct file *file, struct page *page,
	loff_t *poffset)
{
	char *read_data;
	int rc;

	/* Is the page cached? */
	rc = cifs_readpage_from_fscache(file_inode(file), page);
	if (rc == 0)
		goto read_complete;

	read_data = kmap(page);
	/* for reads over a certain size could initiate async read ahead */

	rc = cifs_read(file, read_data, PAGE_CACHE_SIZE, poffset);

	if (rc < 0)
		goto io_error;
	else
		cifs_dbg(FYI, "Bytes read %d\n", rc);

	file_inode(file)->i_atime =
		current_fs_time(file_inode(file)->i_sb);

	if (PAGE_CACHE_SIZE > rc)
		memset(read_data + rc, 0, PAGE_CACHE_SIZE - rc);

	flush_dcache_page(page);
	SetPageUptodate(page);

	/* send this page to the cache */
	cifs_readpage_to_fscache(file_inode(file), page);

	rc = 0;

io_error:
	kunmap(page);
	unlock_page(page);

read_complete:
	return rc;
}

static int cifs_readpage(struct file *file, struct page *page)
{
	loff_t offset = (loff_t)page->index << PAGE_CACHE_SHIFT;
	int rc = -EACCES;
	unsigned int xid;

	xid = get_xid();

	if (file->private_data == NULL) {
		rc = -EBADF;
		free_xid(xid);
		return rc;
	}

	cifs_dbg(FYI, "readpage %p at offset %d 0x%x\n",
		 page, (int)offset, (int)offset);

	rc = cifs_readpage_worker(file, page, &offset);

	free_xid(xid);
	return rc;
}

static int is_inode_writable(struct cifsInodeInfo *cifs_inode)
{
	struct cifsFileInfo *open_file;

	spin_lock(&cifs_file_list_lock);
	list_for_each_entry(open_file, &cifs_inode->openFileList, flist) {
		if (OPEN_FMODE(open_file->f_flags) & FMODE_WRITE) {
			spin_unlock(&cifs_file_list_lock);
			return 1;
		}
	}
	spin_unlock(&cifs_file_list_lock);
	return 0;
}

/* We do not want to update the file size from server for inodes
   open for write - to avoid races with writepage extending
   the file - in the future we could consider allowing
   refreshing the inode only on increases in the file size
   but this is tricky to do without racing with writebehind
   page caching in the current Linux kernel design */
bool is_size_safe_to_change(struct cifsInodeInfo *cifsInode, __u64 end_of_file)
{
	if (!cifsInode)
		return true;

	if (is_inode_writable(cifsInode)) {
		/* This inode is open for write at least once */
		struct cifs_sb_info *cifs_sb;

		cifs_sb = CIFS_SB(cifsInode->vfs_inode.i_sb);
		if (cifs_sb->mnt_cifs_flags & CIFS_MOUNT_DIRECT_IO) {
			/* since no page cache to corrupt on directio
			we can change size safely */
			return true;
		}

		if (i_size_read(&cifsInode->vfs_inode) < end_of_file)
			return true;

		return false;
	} else
		return true;
}

static int cifs_write_begin(struct file *file, struct address_space *mapping,
			loff_t pos, unsigned len, unsigned flags,
			struct page **pagep, void **fsdata)
{
	int oncethru = 0;
	pgoff_t index = pos >> PAGE_CACHE_SHIFT;
	loff_t offset = pos & (PAGE_CACHE_SIZE - 1);
	loff_t page_start = pos & PAGE_MASK;
	loff_t i_size;
	struct page *page;
	int rc = 0;

	cifs_dbg(FYI, "write_begin from %lld len %d\n", (long long)pos, len);

start:
	page = grab_cache_page_write_begin(mapping, index, flags);
	if (!page) {
		rc = -ENOMEM;
		goto out;
	}

	if (PageUptodate(page))
		goto out;

	/*
	 * If we write a full page it will be up to date, no need to read from
	 * the server. If the write is short, we'll end up doing a sync write
	 * instead.
	 */
	if (len == PAGE_CACHE_SIZE)
		goto out;

	/*
	 * optimize away the read when we have an oplock, and we're not
	 * expecting to use any of the data we'd be reading in. That
	 * is, when the page lies beyond the EOF, or straddles the EOF
	 * and the write will cover all of the existing data.
	 */
	if (CIFS_CACHE_READ(CIFS_I(mapping->host))) {
		i_size = i_size_read(mapping->host);
		if (page_start >= i_size ||
		    (offset == 0 && (pos + len) >= i_size)) {
			zero_user_segments(page, 0, offset,
					   offset + len,
					   PAGE_CACHE_SIZE);
			/*
			 * PageChecked means that the parts of the page
			 * to which we're not writing are considered up
			 * to date. Once the data is copied to the
			 * page, it can be set uptodate.
			 */
			SetPageChecked(page);
			goto out;
		}
	}

	if ((file->f_flags & O_ACCMODE) != O_WRONLY && !oncethru) {
		/*
		 * might as well read a page, it is fast enough. If we get
		 * an error, we don't need to return it. cifs_write_end will
		 * do a sync write instead since PG_uptodate isn't set.
		 */
		cifs_readpage_worker(file, page, &page_start);
		page_cache_release(page);
		oncethru = 1;
		goto start;
	} else {
		/* we could try using another file handle if there is one -
		   but how would we lock it to prevent close of that handle
		   racing with this read? In any case
		   this will be written out by write_end so is fine */
	}
out:
	*pagep = page;
	return rc;
}

static int cifs_release_page(struct page *page, gfp_t gfp)
{
	if (PagePrivate(page))
		return 0;

	return cifs_fscache_release_page(page, gfp);
}

static void cifs_invalidate_page(struct page *page, unsigned int offset,
				 unsigned int length)
{
	struct cifsInodeInfo *cifsi = CIFS_I(page->mapping->host);

	if (offset == 0 && length == PAGE_CACHE_SIZE)
		cifs_fscache_invalidate_page(page, &cifsi->vfs_inode);
}

static int cifs_launder_page(struct page *page)
{
	int rc = 0;
	loff_t range_start = page_offset(page);
	loff_t range_end = range_start + (loff_t)(PAGE_CACHE_SIZE - 1);
	struct writeback_control wbc = {
		.sync_mode = WB_SYNC_ALL,
		.nr_to_write = 0,
		.range_start = range_start,
		.range_end = range_end,
	};

	cifs_dbg(FYI, "Launder page: %p\n", page);

	if (clear_page_dirty_for_io(page))
		rc = cifs_writepage_locked(page, &wbc);

	cifs_fscache_invalidate_page(page, page->mapping->host);
	return rc;
}

void cifs_oplock_break(struct work_struct *work)
{
	struct cifsFileInfo *cfile = container_of(work, struct cifsFileInfo,
						  oplock_break);
	struct inode *inode = cfile->dentry->d_inode;
	struct cifsInodeInfo *cinode = CIFS_I(inode);
	struct cifs_tcon *tcon = tlink_tcon(cfile->tlink);
	int rc = 0;

	if (!CIFS_CACHE_WRITE(cinode) && CIFS_CACHE_READ(cinode) &&
						cifs_has_mand_locks(cinode)) {
		cifs_dbg(FYI, "Reset oplock to None for inode=%p due to mand locks\n",
			 inode);
		cinode->oplock = 0;
	}

	if (inode && S_ISREG(inode->i_mode)) {
		if (CIFS_CACHE_READ(cinode))
			break_lease(inode, O_RDONLY);
		else
			break_lease(inode, O_WRONLY);
		rc = filemap_fdatawrite(inode->i_mapping);
		if (!CIFS_CACHE_READ(cinode)) {
			rc = filemap_fdatawait(inode->i_mapping);
			mapping_set_error(inode->i_mapping, rc);
			cifs_invalidate_mapping(inode);
		}
		cifs_dbg(FYI, "Oplock flush inode %p rc %d\n", inode, rc);
	}

	rc = cifs_push_locks(cfile);
	if (rc)
		cifs_dbg(VFS, "Push locks rc = %d\n", rc);

	/*
	 * releasing stale oplock after recent reconnect of smb session using
	 * a now incorrect file handle is not a data integrity issue but do
	 * not bother sending an oplock release if session to server still is
	 * disconnected since oplock already released by the server
	 */
	if (!cfile->oplock_break_cancelled) {
		rc = tcon->ses->server->ops->oplock_response(tcon, &cfile->fid,
							     cinode);
		cifs_dbg(FYI, "Oplock release rc = %d\n", rc);
	}
}

/*
 * The presence of cifs_direct_io() in the address space ops vector
 * allowes open() O_DIRECT flags which would have failed otherwise.
 *
 * In the non-cached mode (mount with cache=none), we shunt off direct read and write requests
 * so this method should never be called.
 *
 * Direct IO is not yet supported in the cached mode. 
 */
static ssize_t
cifs_direct_io(int rw, struct kiocb *iocb, const struct iovec *iov,
               loff_t pos, unsigned long nr_segs)
{
        /*
         * FIXME
         * Eventually need to support direct IO for non forcedirectio mounts
         */
        return -EINVAL;
}


const struct address_space_operations cifs_addr_ops = {
	.readpage = cifs_readpage,
	.readpages = cifs_readpages,
	.writepage = cifs_writepage,
	.writepages = cifs_writepages,
	.write_begin = cifs_write_begin,
	.write_end = cifs_write_end,
	.set_page_dirty = __set_page_dirty_nobuffers,
	.releasepage = cifs_release_page,
	.direct_IO = cifs_direct_io,
	.invalidatepage = cifs_invalidate_page,
	.launder_page = cifs_launder_page,
};

/*
 * cifs_readpages requires the server to support a buffer large enough to
 * contain the header plus one complete page of data.  Otherwise, we need
 * to leave cifs_readpages out of the address space operations.
 */
const struct address_space_operations cifs_addr_ops_smallbuf = {
	.readpage = cifs_readpage,
	.writepage = cifs_writepage,
	.writepages = cifs_writepages,
	.write_begin = cifs_write_begin,
	.write_end = cifs_write_end,
	.set_page_dirty = __set_page_dirty_nobuffers,
	.releasepage = cifs_release_page,
	.invalidatepage = cifs_invalidate_page,
	.launder_page = cifs_launder_page,
};
		if (rc) {
			kfree(wdata);
			break;
		}

		save_len = cur_len;
		for (i = 0; i < nr_pages; i++) {
			copied = min_t(const size_t, cur_len, PAGE_SIZE);
			copied = iov_iter_copy_from_user(wdata->pages[i], &it,
							 0, copied);
			cur_len -= copied;
			iov_iter_advance(&it, copied);
		}
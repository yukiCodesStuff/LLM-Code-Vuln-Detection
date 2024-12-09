 * a time): we would prefer not to enlarge the shmem inode just for that.
 */
struct shmem_falloc {
	int	mode;		/* FALLOC_FL mode currently operating */
	pgoff_t start;		/* start of range currently being fallocated */
	pgoff_t next;		/* the next page offset to be fallocated */
	pgoff_t nr_falloced;	/* how many new pages have been fallocated */
	pgoff_t nr_unswapped;	/* how often writepage refused to swap out */
			spin_lock(&inode->i_lock);
			shmem_falloc = inode->i_private;
			if (shmem_falloc &&
			    !shmem_falloc->mode &&
			    index >= shmem_falloc->start &&
			    index < shmem_falloc->next)
				shmem_falloc->nr_unswapped++;
			else
	 * Trinity finds that probing a hole which tmpfs is punching can
	 * prevent the hole-punch from ever completing: which in turn
	 * locks writers out with its hold on i_mutex.  So refrain from
	 * faulting pages into the hole while it's being punched, and
	 * wait on i_mutex to be released if vmf->flags permits.
	 */
	if (unlikely(inode->i_private)) {
		struct shmem_falloc *shmem_falloc;

		spin_lock(&inode->i_lock);
		shmem_falloc = inode->i_private;
		if (!shmem_falloc ||
		    shmem_falloc->mode != FALLOC_FL_PUNCH_HOLE ||
		    vmf->pgoff < shmem_falloc->start ||
		    vmf->pgoff >= shmem_falloc->next)
			shmem_falloc = NULL;
		spin_unlock(&inode->i_lock);
		/*
		 * i_lock has protected us from taking shmem_falloc seriously
		 * once return from shmem_fallocate() went back up that stack.
		 * i_lock does not serialize with i_mutex at all, but it does
		 * not matter if sometimes we wait unnecessarily, or sometimes
		 * miss out on waiting: we just need to make those cases rare.
		 */
		if (shmem_falloc) {
			if ((vmf->flags & FAULT_FLAG_ALLOW_RETRY) &&
			   !(vmf->flags & FAULT_FLAG_RETRY_NOWAIT)) {
				up_read(&vma->vm_mm->mmap_sem);
				mutex_lock(&inode->i_mutex);
				mutex_unlock(&inode->i_mutex);
				return VM_FAULT_RETRY;
			}
			/* cond_resched? Leave that to GUP or return to user */
			return VM_FAULT_NOPAGE;
		}
	}

	error = shmem_getpage(inode, vmf->pgoff, &vmf->page, SGP_CACHE, &ret);
	if (error)

	mutex_lock(&inode->i_mutex);

	shmem_falloc.mode = mode & ~FALLOC_FL_KEEP_SIZE;

	if (mode & FALLOC_FL_PUNCH_HOLE) {
		struct address_space *mapping = file->f_mapping;
		loff_t unmap_start = round_up(offset, PAGE_SIZE);
		loff_t unmap_end = round_down(offset + len, PAGE_SIZE) - 1;

		shmem_falloc.start = unmap_start >> PAGE_SHIFT;
		shmem_falloc.next = (unmap_end + 1) >> PAGE_SHIFT;
		spin_lock(&inode->i_lock);
		inode->i_private = &shmem_falloc;
					    1 + unmap_end - unmap_start, 0);
		shmem_truncate_range(inode, offset, offset + len - 1);
		/* No need to unmap again: hole-punching leaves COWed pages */
		error = 0;
		goto undone;
	}

	/* We need to check rlimit even when FALLOC_FL_KEEP_SIZE */
	error = inode_newsize_ok(inode, offset + len);
		goto out;
	}

	shmem_falloc.start = start;
	shmem_falloc.next  = start;
	shmem_falloc.nr_falloced = 0;
	shmem_falloc.nr_unswapped = 0;
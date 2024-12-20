		 unsigned long nr_segs, loff_t *poffset)
{
	unsigned long nr_pages, i;
	size_t bytes, copied, len, cur_len;
	ssize_t total_written = 0;
	loff_t offset;
	struct iov_iter it;
	struct cifsFileInfo *open_file;

		save_len = cur_len;
		for (i = 0; i < nr_pages; i++) {
			bytes = min_t(const size_t, cur_len, PAGE_SIZE);
			copied = iov_iter_copy_from_user(wdata->pages[i], &it,
							 0, bytes);
			cur_len -= copied;
			iov_iter_advance(&it, copied);
			/*
			 * If we didn't copy as much as we expected, then that
			 * may mean we trod into an unmapped area. Stop copying
			 * at that point. On the next pass through the big
			 * loop, we'll likely end up getting a zero-length
			 * write and bailing out of it.
			 */
			if (copied < bytes)
				break;
		}
		cur_len = save_len - cur_len;

		/*
		 * If we have no data to send, then that probably means that
		 * the copy above failed altogether. That's most likely because
		 * the address in the iovec was bogus. Set the rc to -EFAULT,
		 * free anything we allocated and bail out.
		 */
		if (!cur_len) {
			for (i = 0; i < nr_pages; i++)
				put_page(wdata->pages[i]);
			kfree(wdata);
			rc = -EFAULT;
			break;
		}

		/*
		 * i + 1 now represents the number of pages we actually used in
		 * the copy phase above. Bring nr_pages down to that, and free
		 * any pages that we didn't use.
		 */
		for ( ; nr_pages > i + 1; nr_pages--)
			put_page(wdata->pages[nr_pages - 1]);

		wdata->sync_mode = WB_SYNC_ALL;
		wdata->nr_pages = nr_pages;
		wdata->offset = (__u64)offset;
		wdata->cfile = cifsFileInfo_get(open_file);
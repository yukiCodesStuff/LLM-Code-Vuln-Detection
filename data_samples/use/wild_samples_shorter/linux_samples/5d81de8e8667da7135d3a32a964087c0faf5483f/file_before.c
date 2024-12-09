		 unsigned long nr_segs, loff_t *poffset)
{
	unsigned long nr_pages, i;
	size_t copied, len, cur_len;
	ssize_t total_written = 0;
	loff_t offset;
	struct iov_iter it;
	struct cifsFileInfo *open_file;

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
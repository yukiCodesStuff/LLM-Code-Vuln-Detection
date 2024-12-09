	 * Inherently racy -- command line shares address space
	 * with code and data.
	 */
	rv = access_remote_vm(mm, arg_end - 1, &c, 1, FOLL_ANON);
	if (rv <= 0)
		goto out_free_page;

	rv = 0;
			int nr_read;

			_count = min3(count, len, PAGE_SIZE);
			nr_read = access_remote_vm(mm, p, page, _count, FOLL_ANON);
			if (nr_read < 0)
				rv = nr_read;
			if (nr_read <= 0)
				goto out_free_page;
				bool final;

				_count = min3(count, len, PAGE_SIZE);
				nr_read = access_remote_vm(mm, p, page, _count, FOLL_ANON);
				if (nr_read < 0)
					rv = nr_read;
				if (nr_read <= 0)
					goto out_free_page;
		max_len = min_t(size_t, PAGE_SIZE, count);
		this_len = min(max_len, this_len);

		retval = access_remote_vm(mm, (env_start + src), page, this_len, FOLL_ANON);

		if (retval <= 0) {
			ret = retval;
			break;
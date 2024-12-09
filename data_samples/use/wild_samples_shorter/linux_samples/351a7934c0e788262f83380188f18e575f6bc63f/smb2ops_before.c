	/* start with specified wsize, or default */
	wsize = volume_info->wsize ? volume_info->wsize : CIFS_DEFAULT_IOSIZE;
	wsize = min_t(unsigned int, wsize, server->max_write);
	/*
	 * limit write size to 2 ** 16, because we don't support multicredit
	 * requests now.
	 */
	wsize = min_t(unsigned int, wsize, 2 << 15);

	return wsize;
}

	/* start with specified rsize, or default */
	rsize = volume_info->rsize ? volume_info->rsize : CIFS_DEFAULT_IOSIZE;
	rsize = min_t(unsigned int, rsize, server->max_read);
	/*
	 * limit write size to 2 ** 16, because we don't support multicredit
	 * requests now.
	 */
	rsize = min_t(unsigned int, rsize, 2 << 15);

	return rsize;
}

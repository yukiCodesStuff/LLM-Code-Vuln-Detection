
	if (num <= 0) return NULL;

	/* We don't support shrinking the buffer. Note the memcpy that copies
	 * |old_len| bytes to the new buffer, below. */
	if (num < old_len) return NULL;

	if (realloc_debug_func != NULL)
		realloc_debug_func(str, NULL, num, file, line, 0);
	ret=malloc_ex_func(num,file,line);
	if(ret)
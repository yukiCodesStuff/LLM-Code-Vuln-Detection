	char *ptr;
	int err;

	file = kmalloc(sizeof(*file), GFP_NOIO);
	if (!file)
		return -ENOMEM;

	err = 0;
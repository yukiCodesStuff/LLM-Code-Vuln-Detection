	unsigned long state;
	int retval;

	retval = kstrtoul(buf, 10, &state);
	if (retval)
		return retval;

	temp_buf.command = ARVO_COMMAND_MODE_KEY;
	unsigned long key_mask;
	int retval;

	retval = kstrtoul(buf, 10, &key_mask);
	if (retval)
		return retval;

	temp_buf.command = ARVO_COMMAND_KEY_MASK;
	unsigned long profile;
	int retval;

	retval = kstrtoul(buf, 10, &profile);
	if (retval)
		return retval;

	if (profile < 1 || profile > 5)
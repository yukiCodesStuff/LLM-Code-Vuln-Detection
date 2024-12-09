		return -ENOSYS;

	if (arg != CDSL_CURRENT && arg != CDSL_NONE) {
		if (arg >= cdi->capacity)
			return -EINVAL;
	}

	/*
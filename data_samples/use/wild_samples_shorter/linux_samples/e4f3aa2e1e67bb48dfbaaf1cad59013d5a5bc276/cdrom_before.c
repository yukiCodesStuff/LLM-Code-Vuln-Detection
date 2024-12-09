		return -ENOSYS;

	if (arg != CDSL_CURRENT && arg != CDSL_NONE) {
		if ((int)arg >= cdi->capacity)
			return -EINVAL;
	}

	/*
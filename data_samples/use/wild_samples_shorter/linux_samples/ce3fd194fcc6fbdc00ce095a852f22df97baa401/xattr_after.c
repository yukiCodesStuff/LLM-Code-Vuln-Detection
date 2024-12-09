
	/* Check the values */
	while (!IS_LAST_ENTRY(entry)) {
		u32 size = le32_to_cpu(entry->e_value_size);

		if (size > INT_MAX)
			return -EFSCORRUPTED;

		if (size != 0 && entry->e_value_inum == 0) {
			u16 offs = le16_to_cpu(entry->e_value_offs);
			void *value;

			/*
			 * The value cannot overlap the names, and the value
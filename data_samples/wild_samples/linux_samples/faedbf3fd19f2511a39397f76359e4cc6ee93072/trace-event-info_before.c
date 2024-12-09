		if (!mkstemp(tdata->temp_file)) {
			pr_debug("Can't make temp file");
			return NULL;
		}
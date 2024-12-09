		if (!mkstemp(tdata->temp_file)) {
			pr_debug("Can't make temp file");
			free(tdata);
			return NULL;
		}
			 "/tmp/perf-XXXXXX");
		if (!mkstemp(tdata->temp_file)) {
			pr_debug("Can't make temp file");
			free(tdata);
			return NULL;
		}

		temp_fd = open(tdata->temp_file, O_RDWR);
		if (temp_fd < 0) {
			pr_debug("Can't read '%s'", tdata->temp_file);
			free(tdata);
			return NULL;
		}

		/*
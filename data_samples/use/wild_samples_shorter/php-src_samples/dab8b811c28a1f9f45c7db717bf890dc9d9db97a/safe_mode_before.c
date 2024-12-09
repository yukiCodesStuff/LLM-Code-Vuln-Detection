			mode = CHECKUID_CHECK_FILE_AND_DIR;
		}
	}

	/* 
	 * If given filepath is a URL, allow - safe mode stuff
	 * related to URL's is checked in individual functions
	 */
	wrapper = php_stream_locate_url_wrapper(filename, NULL, STREAM_LOCATE_WRAPPERS_ONLY TSRMLS_CC);
	if (wrapper != NULL)
		return 1;
		
	/* First we see if the file is owned by the same user...
	 * If that fails, passthrough and check directory...
	 */
			mode = CHECKUID_CHECK_FILE_AND_DIR;
		}
	}
		
	/* First we see if the file is owned by the same user...
	 * If that fails, passthrough and check directory...
	 */
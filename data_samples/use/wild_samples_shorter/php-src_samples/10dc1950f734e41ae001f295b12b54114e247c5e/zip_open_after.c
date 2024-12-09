        zip_error_set(error, ZIP_ER_SEEK, EFBIG);
        return NULL;
    }
    if (offset+size > buf_offset + eocd_offset) {
	/* cdir spans past EOCD record */
	zip_error_set(error, ZIP_ER_INCONS, 0);
	return NULL;
    }
    if ((flags & ZIP_CHECKCONS) && offset+size != buf_offset + eocd_offset) {
	zip_error_set(error, ZIP_ER_INCONS, 0);
	return NULL;
    }

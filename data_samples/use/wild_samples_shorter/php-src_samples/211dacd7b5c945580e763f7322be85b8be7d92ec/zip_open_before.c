        zip_error_set(error, ZIP_ER_SEEK, EFBIG);
        return NULL;
    }
    if ((flags & ZIP_CHECKCONS) && offset+size != eocd_offset) {
	zip_error_set(error, ZIP_ER_INCONS, 0);
	return NULL;
    }

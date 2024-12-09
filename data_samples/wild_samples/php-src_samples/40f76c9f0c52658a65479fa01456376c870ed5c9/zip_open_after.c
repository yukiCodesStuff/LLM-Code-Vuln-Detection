    if (offset > ZIP_INT64_MAX || offset+size < offset) {
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

    if ((cd=_zip_cdir_new(nentry, error)) == NULL)
	return NULL;

    
    cd->size = size;
    cd->offset = offset;

    return cd;
}
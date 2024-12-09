    if ((flags & ZIP_CHECKCONS) && size+eocd_offset+12 != (zip_uint64_t)(buf_offset+(eocd64loc-buf))) {
	_zip_error_set(error, ZIP_ER_INCONS, 0);
	return NULL;
    }

    cdp += 4; /* skip version made by/needed */
    cdp += 8; /* skip num disks */
    
    nentry = _zip_read8(&cdp);
    i = _zip_read8(&cdp);

    if (nentry != i) {
	_zip_error_set(error, ZIP_ER_MULTIDISK, 0);
	return NULL;
    }
    cdp += 8; /* skip num disks */
    
    nentry = _zip_read8(&cdp);
    i = _zip_read8(&cdp);

    if (nentry != i) {
	_zip_error_set(error, ZIP_ER_MULTIDISK, 0);
	return NULL;
    }

    size = _zip_read8(&cdp);
    offset = _zip_read8(&cdp);

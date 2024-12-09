        (BIO_write(bp, "-----\n", 6) != 6))
        goto err;

    i = strlen(header);
    if (i > 0) {
        if ((BIO_write(bp, header, i) != i) || (BIO_write(bp, "\n", 1) != 1))
            goto err;
    }
{
    BIO *tmp = *header;
    char *linebuf, *p;
    int len, line, ret = 0, end = 0, prev_partial_line_read = 0, partial_line_read = 0;
    /* 0 if not seen (yet), 1 if reading header, 2 if finished header */
    enum header_status got_header = MAYBE_HEADER;
    unsigned int flags_mask;
    size_t namelen;
        return 0;
    }

    for (line = 0; ; line++) {
        flags_mask = ~0u;
        len = BIO_gets(bp, linebuf, LINESIZE);
        if (len <= 0) {
            ERR_raise(ERR_LIB_PEM, PEM_R_BAD_END_LINE);
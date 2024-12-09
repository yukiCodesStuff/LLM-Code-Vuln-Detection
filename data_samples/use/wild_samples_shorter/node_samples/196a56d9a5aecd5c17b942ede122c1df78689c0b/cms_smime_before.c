         * Don't use SMIME_TEXT for verify: it adds headers and we want to
         * remove them.
         */
        SMIME_crlf_copy(dcont, cmsbio, flags & ~SMIME_TEXT);

        if (flags & CMS_TEXT) {
            if (!SMIME_text(tmpout, out)) {
                ERR_raise(ERR_LIB_CMS, CMS_R_SMIME_TEXT_ERROR);
        return 0;
    }

    ret = SMIME_crlf_copy(data, cmsbio, flags);

    (void)BIO_flush(cmsbio);

    if (!CMS_dataFinal(cms, cmsbio)) {
        ERR_raise(ERR_LIB_CMS, CMS_R_CMS_DATAFINAL_ERROR);
        goto err;
    }
err:
    do_free_upto(cmsbio, dcont);

    return ret;
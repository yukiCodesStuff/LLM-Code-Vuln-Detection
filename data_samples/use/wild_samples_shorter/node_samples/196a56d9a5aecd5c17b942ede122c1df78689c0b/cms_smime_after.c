         * Don't use SMIME_TEXT for verify: it adds headers and we want to
         * remove them.
         */
        if (!SMIME_crlf_copy(dcont, cmsbio, flags & ~SMIME_TEXT))
            goto err;

        if (flags & CMS_TEXT) {
            if (!SMIME_text(tmpout, out)) {
                ERR_raise(ERR_LIB_CMS, CMS_R_SMIME_TEXT_ERROR);
        return 0;
    }

    if (!SMIME_crlf_copy(data, cmsbio, flags)) {
        goto err;
    }

    (void)BIO_flush(cmsbio);

    if (!CMS_dataFinal(cms, cmsbio)) {
        ERR_raise(ERR_LIB_CMS, CMS_R_CMS_DATAFINAL_ERROR);
        goto err;
    }

    ret = 1;

err:
    do_free_upto(cmsbio, dcont);

    return ret;
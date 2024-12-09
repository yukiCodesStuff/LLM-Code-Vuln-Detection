        if (tmpout == NULL) {
            ERR_raise(ERR_LIB_CMS, ERR_R_MALLOC_FAILURE);
            goto err;
        }
        cmsbio = CMS_dataInit(cms, tmpout);
        if (cmsbio == NULL)
            goto err;
        /*
         * Don't use SMIME_TEXT for verify: it adds headers and we want to
         * remove them.
         */
        SMIME_crlf_copy(dcont, cmsbio, flags & ~SMIME_TEXT);

        if (flags & CMS_TEXT) {
            if (!SMIME_text(tmpout, out)) {
                ERR_raise(ERR_LIB_CMS, CMS_R_SMIME_TEXT_ERROR);
                goto err;
            }
        }
    if ((cmsbio = CMS_dataInit(cms, dcont)) == NULL) {
        ERR_raise(ERR_LIB_CMS, CMS_R_CMS_LIB);
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

}

#ifdef ZLIB

int CMS_uncompress(CMS_ContentInfo *cms, BIO *dcont, BIO *out,
                   unsigned int flags)
{
    BIO *cont;
    int r;

    if (OBJ_obj2nid(CMS_get0_type(cms)) != NID_id_smime_ct_compressedData) {
        ERR_raise(ERR_LIB_CMS, CMS_R_TYPE_NOT_COMPRESSED_DATA);
        return 0;
    }

    if (dcont == NULL && !check_content(cms))
        return 0;

    cont = CMS_dataInit(cms, dcont);
    if (cont == NULL)
        return 0;
    r = cms_copy_content(out, cont, flags);
    do_free_upto(cont, dcont);
    return r;
}

CMS_ContentInfo *CMS_compress(BIO *in, int comp_nid, unsigned int flags)
{
    CMS_ContentInfo *cms;

    if (comp_nid <= 0)
        comp_nid = NID_zlib_compression;
    cms = ossl_cms_CompressedData_create(comp_nid, NULL, NULL);
    if (cms == NULL)
        return NULL;

    if (!(flags & CMS_DETACHED))
        CMS_set_detached(cms, 0);

    if ((flags & CMS_STREAM) || CMS_final(cms, in, NULL, flags))
        return cms;

    CMS_ContentInfo_free(cms);
    return NULL;
}

#else

int CMS_uncompress(CMS_ContentInfo *cms, BIO *dcont, BIO *out,
                   unsigned int flags)
{
    ERR_raise(ERR_LIB_CMS, CMS_R_UNSUPPORTED_COMPRESSION_ALGORITHM);
    return 0;
}

CMS_ContentInfo *CMS_compress(BIO *in, int comp_nid, unsigned int flags)
{
    ERR_raise(ERR_LIB_CMS, CMS_R_UNSUPPORTED_COMPRESSION_ALGORITHM);
    return NULL;
}

#endif
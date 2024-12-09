    return 1;
}

int SSL_CTX_use_serverinfo_ex(SSL_CTX *ctx, unsigned int version,
                              const unsigned char *serverinfo,
                              size_t serverinfo_length)
{
    unsigned char *new_serverinfo;

    if (ctx == NULL || serverinfo == NULL || serverinfo_length == 0) {
        ERR_raise(ERR_LIB_SSL, ERR_R_PASSED_NULL_PARAMETER);
        return 0;
    }
    if (!serverinfo_process_buffer(version, serverinfo, serverinfo_length,
                                   NULL)) {
        ERR_raise(ERR_LIB_SSL, SSL_R_INVALID_SERVERINFO_DATA);
        return 0;
    unsigned int name_len;
    int ret = 0;
    BIO *bin = NULL;
    size_t num_extensions = 0, contextoff = 0;

    if (ctx == NULL || file == NULL) {
        ERR_raise(ERR_LIB_SSL, ERR_R_PASSED_NULL_PARAMETER);
        goto end;

    for (num_extensions = 0;; num_extensions++) {
        unsigned int version;

        if (PEM_read_bio(bin, &name, &header, &extension, &extension_length)
            == 0) {
            /*
                ERR_raise(ERR_LIB_SSL, SSL_R_BAD_DATA);
                goto end;
            }
            /*
             * File does not have a context value so we must take account of
             * this later.
             */
            contextoff = 4;
        } else {
            /* 8 byte header: 4 bytes context, 2 bytes type, 2 bytes len */
            if (extension_length < 8
                    || (extension[6] << 8) + extension[7]
            }
        }
        /* Append the decoded extension to the serverinfo buffer */
        tmp = OPENSSL_realloc(serverinfo, serverinfo_length + extension_length
                                          + contextoff);
        if (tmp == NULL) {
            ERR_raise(ERR_LIB_SSL, ERR_R_MALLOC_FAILURE);
            goto end;
        }
        serverinfo = tmp;
        if (contextoff > 0) {
            unsigned char *sinfo = serverinfo + serverinfo_length;

            /* We know this only uses the last 2 bytes */
            sinfo[0] = 0;
            sinfo[1] = 0;
            sinfo[2] = (SYNTHV1CONTEXT >> 8) & 0xff;
            sinfo[3] = SYNTHV1CONTEXT & 0xff;
        }
        memcpy(serverinfo + serverinfo_length + contextoff,
               extension, extension_length);
        serverinfo_length += extension_length + contextoff;

        OPENSSL_free(name);
        name = NULL;
        OPENSSL_free(header);
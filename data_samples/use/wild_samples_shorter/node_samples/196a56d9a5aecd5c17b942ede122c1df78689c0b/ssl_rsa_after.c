    return 1;
}

static size_t extension_contextoff(unsigned int version)
{
    return version == SSL_SERVERINFOV1 ? 4 : 0;
}

static size_t extension_append_length(unsigned int version, size_t extension_length)
{
    return extension_length + extension_contextoff(version);
}

static void extension_append(unsigned int version,
                             const unsigned char *extension,
                             const size_t extension_length,
                             unsigned char *serverinfo)
{
    const size_t contextoff = extension_contextoff(version);

    if (contextoff > 0) {
        /* We know this only uses the last 2 bytes */
        serverinfo[0] = 0;
        serverinfo[1] = 0;
        serverinfo[2] = (SYNTHV1CONTEXT >> 8) & 0xff;
        serverinfo[3] = SYNTHV1CONTEXT & 0xff;
    }

    memcpy(serverinfo + contextoff, extension, extension_length);
}

int SSL_CTX_use_serverinfo_ex(SSL_CTX *ctx, unsigned int version,
                              const unsigned char *serverinfo,
                              size_t serverinfo_length)
{
    unsigned char *new_serverinfo = NULL;

    if (ctx == NULL || serverinfo == NULL || serverinfo_length == 0) {
        ERR_raise(ERR_LIB_SSL, ERR_R_PASSED_NULL_PARAMETER);
        return 0;
    }
    if (version == SSL_SERVERINFOV1) {
        /*
         * Convert serverinfo version v1 to v2 and call yourself recursively
         * over the converted serverinfo.
         */
        const size_t sinfo_length = extension_append_length(SSL_SERVERINFOV1,
                                                            serverinfo_length);
        unsigned char *sinfo;
        int ret;

        sinfo = OPENSSL_malloc(sinfo_length);
        if (sinfo == NULL) {
            ERR_raise(ERR_LIB_SSL, ERR_R_MALLOC_FAILURE);
            return 0;
        }

        extension_append(SSL_SERVERINFOV1, serverinfo, serverinfo_length, sinfo);

        ret = SSL_CTX_use_serverinfo_ex(ctx, SSL_SERVERINFOV2, sinfo,
                                        sinfo_length);

        OPENSSL_free(sinfo);
        return ret;
    }
    if (!serverinfo_process_buffer(version, serverinfo, serverinfo_length,
                                   NULL)) {
        ERR_raise(ERR_LIB_SSL, SSL_R_INVALID_SERVERINFO_DATA);
        return 0;
    unsigned int name_len;
    int ret = 0;
    BIO *bin = NULL;
    size_t num_extensions = 0;

    if (ctx == NULL || file == NULL) {
        ERR_raise(ERR_LIB_SSL, ERR_R_PASSED_NULL_PARAMETER);
        goto end;

    for (num_extensions = 0;; num_extensions++) {
        unsigned int version;
        size_t append_length;

        if (PEM_read_bio(bin, &name, &header, &extension, &extension_length)
            == 0) {
            /*
                ERR_raise(ERR_LIB_SSL, SSL_R_BAD_DATA);
                goto end;
            }
        } else {
            /* 8 byte header: 4 bytes context, 2 bytes type, 2 bytes len */
            if (extension_length < 8
                    || (extension[6] << 8) + extension[7]
            }
        }
        /* Append the decoded extension to the serverinfo buffer */
        append_length = extension_append_length(version, extension_length);
        tmp = OPENSSL_realloc(serverinfo, serverinfo_length + append_length);
        if (tmp == NULL) {
            ERR_raise(ERR_LIB_SSL, ERR_R_MALLOC_FAILURE);
            goto end;
        }
        serverinfo = tmp;
        extension_append(version, extension, extension_length,
                         serverinfo + serverinfo_length);
        serverinfo_length += append_length;

        OPENSSL_free(name);
        name = NULL;
        OPENSSL_free(header);
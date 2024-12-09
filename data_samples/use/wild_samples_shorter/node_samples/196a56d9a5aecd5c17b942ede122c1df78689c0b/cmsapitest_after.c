    return ret;
}

static unsigned char *read_all(BIO *bio, long *p_len)
{
    const int step = 256;
    unsigned char *buf = NULL;
    unsigned char *tmp = NULL;
    int ret;

    *p_len = 0;
    for (;;) {
        tmp = OPENSSL_realloc(buf, *p_len + step);
        if (tmp == NULL)
            break;
        buf = tmp;
        ret = BIO_read(bio, buf + *p_len, step);
        if (ret < 0)
            break;

        *p_len += ret;

        if (ret < step)
            return buf;
    }

    /* Error */
    OPENSSL_free(buf);
    *p_len = 0;
    return NULL;
}

static int test_d2i_CMS_decode(const int idx)
{
    BIO *bio = NULL;
    CMS_ContentInfo *cms = NULL;
    unsigned char *buf = NULL;
    const unsigned char *tmp = NULL;
    long buf_len = 0;
    int ret = 0;

    if (!TEST_ptr(bio = BIO_new_file(derin, "r")))
      goto end;

    switch (idx) {
    case 0:
        if (!TEST_ptr(cms = d2i_CMS_bio(bio, NULL)))
            goto end;
        break;
    case 1:
        if (!TEST_ptr(buf = read_all(bio, &buf_len)))
            goto end;
        tmp = buf;
        if (!TEST_ptr(cms = d2i_CMS_ContentInfo(NULL, &tmp, buf_len)))
            goto end;
        break;
    }

    if (!TEST_int_eq(ERR_peek_error(), 0))
        goto end;

    ret = 1;
end:
    CMS_ContentInfo_free(cms);
    BIO_free(bio);
    OPENSSL_free(buf);

    return ret;
}

    ADD_TEST(test_encrypt_decrypt_aes_192_gcm);
    ADD_TEST(test_encrypt_decrypt_aes_256_gcm);
    ADD_TEST(test_d2i_CMS_bio_NULL);
    ADD_ALL_TESTS(test_d2i_CMS_decode, 2);
    return 1;
}

void cleanup_tests(void)
    return ret;
}

static int test_d2i_CMS_bio_file_encrypted_data(void)
{
    BIO *bio = NULL;
    CMS_ContentInfo *cms = NULL;
    int ret = 0;

    ERR_clear_error();

    if (!TEST_ptr(bio = BIO_new_file(derin, "r"))
      || !TEST_ptr(cms = d2i_CMS_bio(bio, NULL)))
      goto end;

    if (!TEST_int_eq(ERR_peek_error(), 0))
        goto end;

    ret = 1;
end:
    CMS_ContentInfo_free(cms);
    BIO_free(bio);

    return ret;
}

    ADD_TEST(test_encrypt_decrypt_aes_192_gcm);
    ADD_TEST(test_encrypt_decrypt_aes_256_gcm);
    ADD_TEST(test_d2i_CMS_bio_NULL);
    ADD_TEST(test_d2i_CMS_bio_file_encrypted_data);
    return 1;
}

void cleanup_tests(void)
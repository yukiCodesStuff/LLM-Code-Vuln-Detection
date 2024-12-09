    return ret;
}

static int ivgen_test(void)
{
    unsigned char iv_gen[16];
    unsigned char tag[32];
    return do_encrypt(iv_gen, ct, &ctlen, tag, &taglen)
           && do_decrypt(iv_gen, ct, ctlen, tag, taglen);
}

int setup_tests(void)
{
    ADD_TEST(kat_test);
    ADD_TEST(badkeylen_test);
    ADD_TEST(ivgen_test);
    return 1;
}
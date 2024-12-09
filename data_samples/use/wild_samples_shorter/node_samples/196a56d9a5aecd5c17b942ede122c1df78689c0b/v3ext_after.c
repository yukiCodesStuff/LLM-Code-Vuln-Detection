 */

#include <stdio.h>
#include <string.h>
#include <openssl/x509.h>
#include <openssl/x509v3.h>
#include <openssl/pem.h>
#include <openssl/err.h>
    ASN1_OCTET_STRING_free(ip2);
    return testresult;
}

static struct extvalues_st {
    const char *value;
    int pass;
} extvalues[] = {
    /* No prefix is ok */
    { "sbgp-ipAddrBlock = IPv4:192.0.0.1\n", 1 },
    { "sbgp-ipAddrBlock = IPv4:192.0.0.0/0\n", 1 },
    { "sbgp-ipAddrBlock = IPv4:192.0.0.0/1\n", 1 },
    { "sbgp-ipAddrBlock = IPv4:192.0.0.0/32\n", 1 },
    /* Prefix is too long */
    { "sbgp-ipAddrBlock = IPv4:192.0.0.0/33\n", 0 },
    /* Unreasonably large prefix */
    { "sbgp-ipAddrBlock = IPv4:192.0.0.0/12341234\n", 0 },
    /* Invalid IP addresses */
    { "sbgp-ipAddrBlock = IPv4:192.0.0\n", 0 },
    { "sbgp-ipAddrBlock = IPv4:256.0.0.0\n", 0 },
    { "sbgp-ipAddrBlock = IPv4:-1.0.0.0\n", 0 },
    { "sbgp-ipAddrBlock = IPv4:192.0.0.0.0\n", 0 },
    { "sbgp-ipAddrBlock = IPv3:192.0.0.0\n", 0 },

    /* IPv6 */
    /* No prefix is ok */
    { "sbgp-ipAddrBlock = IPv6:2001:db8::\n", 1 },
    { "sbgp-ipAddrBlock = IPv6:2001::db8\n", 1 },
    { "sbgp-ipAddrBlock = IPv6:2001:0db8:0000:0000:0000:0000:0000:0000\n", 1 },
    { "sbgp-ipAddrBlock = IPv6:2001:db8::/0\n", 1 },
    { "sbgp-ipAddrBlock = IPv6:2001:db8::/1\n", 1 },
    { "sbgp-ipAddrBlock = IPv6:2001:db8::/32\n", 1 },
    { "sbgp-ipAddrBlock = IPv6:2001:0db8:0000:0000:0000:0000:0000:0000/32\n", 1 },
    { "sbgp-ipAddrBlock = IPv6:2001:db8::/128\n", 1 },
    /* Prefix is too long */
    { "sbgp-ipAddrBlock = IPv6:2001:db8::/129\n", 0 },
    /* Unreasonably large prefix */
    { "sbgp-ipAddrBlock = IPv6:2001:db8::/12341234\n", 0 },
    /* Invalid IP addresses */
    /* Not enough blocks of numbers */
    { "sbgp-ipAddrBlock = IPv6:2001:0db8:0000:0000:0000:0000:0000\n", 0 },
    /* Too many blocks of numbers */
    { "sbgp-ipAddrBlock = IPv6:2001:0db8:0000:0000:0000:0000:0000:0000:0000\n", 0 },
    /* First value too large */
    { "sbgp-ipAddrBlock = IPv6:1ffff:0db8:0000:0000:0000:0000:0000:0000\n", 0 },
    /* First value with invalid characters */
    { "sbgp-ipAddrBlock = IPv6:fffg:0db8:0000:0000:0000:0000:0000:0000\n", 0 },
    /* First value is negative */
    { "sbgp-ipAddrBlock = IPv6:-1:0db8:0000:0000:0000:0000:0000:0000\n", 0 }
};

static int test_ext_syntax(void)
{
    size_t i;
    int testresult = 1;

    for (i = 0; i < OSSL_NELEM(extvalues); i++) {
        X509V3_CTX ctx;
        BIO *extbio = BIO_new_mem_buf(extvalues[i].value,
                                      strlen(extvalues[i].value));
        CONF *conf;
        long eline;

        if (!TEST_ptr(extbio))
            return 0 ;

        conf = NCONF_new_ex(NULL, NULL);
        if (!TEST_ptr(conf)) {
            BIO_free(extbio);
            return 0;
        }
        if (!TEST_long_gt(NCONF_load_bio(conf, extbio, &eline), 0)) {
            testresult = 0;
        } else {
            X509V3_set_ctx_test(&ctx);
            X509V3_set_nconf(&ctx, conf);

            if (extvalues[i].pass) {
                if (!TEST_true(X509V3_EXT_add_nconf(conf, &ctx, "default",
                                                    NULL))) {
                    TEST_info("Value: %s", extvalues[i].value);
                    testresult = 0;
                }
            } else {
                ERR_set_mark();
                if (!TEST_false(X509V3_EXT_add_nconf(conf, &ctx, "default",
                                                     NULL))) {
                    testresult = 0;
                    TEST_info("Value: %s", extvalues[i].value);
                    ERR_clear_last_mark();
                } else {
                    ERR_pop_to_mark();
                }
            }
        }
        BIO_free(extbio);
        NCONF_free(conf);
    }

    return testresult;
}
#endif /* OPENSSL_NO_RFC3779 */

OPT_TEST_DECLARE_USAGE("cert.pem\n")

#ifndef OPENSSL_NO_RFC3779
    ADD_TEST(test_asid);
    ADD_TEST(test_addr_ranges);
    ADD_TEST(test_ext_syntax);
#endif /* OPENSSL_NO_RFC3779 */
    return 1;
}
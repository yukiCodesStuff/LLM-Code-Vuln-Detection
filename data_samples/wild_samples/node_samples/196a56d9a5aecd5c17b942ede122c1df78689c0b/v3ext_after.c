/*
 * Copyright 2016-2022 The OpenSSL Project Authors. All Rights Reserved.
 *
 * Licensed under the Apache License 2.0 (the "License").  You may not use
 * this file except in compliance with the License.  You can obtain a copy
 * in the file LICENSE in the source distribution or at
 * https://www.openssl.org/source/license.html
 */

#include <stdio.h>
#include <string.h>
#include <openssl/x509.h>
#include <openssl/x509v3.h>
#include <openssl/pem.h>
#include <openssl/err.h>
#include "internal/nelem.h"

#include "testutil.h"

static const char *infile;

static int test_pathlen(void)
{
    X509 *x = NULL;
    BIO *b = NULL;
    long pathlen;
    int ret = 0;

    if (!TEST_ptr(b = BIO_new_file(infile, "r"))
            || !TEST_ptr(x = PEM_read_bio_X509(b, NULL, NULL, NULL))
            || !TEST_int_eq(pathlen = X509_get_pathlen(x), 6))
        goto end;

    ret = 1;

end:
    BIO_free(b);
    X509_free(x);
    return ret;
}
    for (i = 0; i < OSSL_NELEM(ranges); i++) {
        addr = sk_IPAddressFamily_new_null();
        if (!TEST_ptr(addr))
            goto end;
        /*
         * Has the side effect of installing the comparison function onto the
         * stack.
         */
        if (!TEST_true(X509v3_addr_canonize(addr)))
            goto end;

        ip1 = a2i_IPADDRESS(ranges[i].ip1);
        if (!TEST_ptr(ip1))
            goto end;
        if (!TEST_true(ip1->length == 4 || ip1->length == 16))
            goto end;
        ip2 = a2i_IPADDRESS(ranges[i].ip2);
        if (!TEST_ptr(ip2))
            goto end;
        if (!TEST_int_eq(ip2->length, ip1->length))
            goto end;
        if (!TEST_true(memcmp(ip1->data, ip2->data, ip1->length) <= 0))
            goto end;

        if (!TEST_true(X509v3_addr_add_range(addr, ranges[i].afi, NULL, ip1->data, ip2->data)))
            goto end;

        if (!TEST_true(X509v3_addr_is_canonical(addr)))
            goto end;

        if (!check_addr(addr, ranges[i].rorp))
            goto end;

        sk_IPAddressFamily_pop_free(addr, IPAddressFamily_free);
        addr = NULL;
        ASN1_OCTET_STRING_free(ip1);
        ASN1_OCTET_STRING_free(ip2);
        ip1 = ip2 = NULL;
    }

    testresult = 1;
 end:
    sk_IPAddressFamily_pop_free(addr, IPAddressFamily_free);
    ASN1_OCTET_STRING_free(ip1);
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
    if (!test_skip_common_options()) {
        TEST_error("Error parsing test options\n");
        return 0;
    }

    if (!TEST_ptr(infile = test_get_argument(0)))
        return 0;

    ADD_TEST(test_pathlen);
#ifndef OPENSSL_NO_RFC3779
    ADD_TEST(test_asid);
    ADD_TEST(test_addr_ranges);
    ADD_TEST(test_ext_syntax);
#endif /* OPENSSL_NO_RFC3779 */
    return 1;
}
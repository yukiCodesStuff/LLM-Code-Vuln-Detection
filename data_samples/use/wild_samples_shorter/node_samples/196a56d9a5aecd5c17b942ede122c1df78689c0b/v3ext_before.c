 */

#include <stdio.h>
#include <openssl/x509.h>
#include <openssl/x509v3.h>
#include <openssl/pem.h>
#include <openssl/err.h>
    ASN1_OCTET_STRING_free(ip2);
    return testresult;
}
#endif /* OPENSSL_NO_RFC3779 */

OPT_TEST_DECLARE_USAGE("cert.pem\n")

#ifndef OPENSSL_NO_RFC3779
    ADD_TEST(test_asid);
    ADD_TEST(test_addr_ranges);
#endif /* OPENSSL_NO_RFC3779 */
    return 1;
}
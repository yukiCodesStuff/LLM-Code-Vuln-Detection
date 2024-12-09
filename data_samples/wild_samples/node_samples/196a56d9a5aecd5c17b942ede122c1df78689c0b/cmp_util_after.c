/*
 * Copyright 2007-2022 The OpenSSL Project Authors. All Rights Reserved.
 * Copyright Nokia 2007-2019
 * Copyright Siemens AG 2015-2019
 *
 * Licensed under the Apache License 2.0 (the "License").  You may not use
 * this file except in compliance with the License.  You can obtain a copy
 * in the file LICENSE in the source distribution or at
 * https://www.openssl.org/source/license.html
 */

#include <string.h>
#include <openssl/cmp_util.h>
#include "cmp_local.h" /* just for decls of internal functions defined here */
#include <openssl/cmperr.h>
#include <openssl/err.h> /* should be implied by cmperr.h */
#include <openssl/x509v3.h>

/*
 * use trace API for CMP-specific logging, prefixed by "CMP " and severity
 */

int OSSL_CMP_log_open(void) /* is designed to be idempotent */
{
#ifdef OPENSSL_NO_TRACE
    return 1;
#else
# ifndef OPENSSL_NO_STDIO
    BIO *bio = BIO_new_fp(stdout, BIO_NOCLOSE);

    if (bio != NULL && OSSL_trace_set_channel(OSSL_TRACE_CATEGORY_CMP, bio))
        return 1;
    BIO_free(bio);
# endif
    ERR_raise(ERR_LIB_CMP, CMP_R_NO_STDIO);
    return 0;
#endif
}
                if ((*level = parse_level(p_level)) >= 0) {
                    *func = OPENSSL_strndup(p_func, p_file - 1 - p_func);
                    *file = OPENSSL_strndup(p_file, p_line - 1 - p_file);
                    /* no real problem if OPENSSL_strndup() returns NULL */
                    *line = (int)line_number;
                    msg = strchr(p_level, ':');
                    if (msg != NULL && *++msg == ' ')
                        msg++;
                }
            }
        }
    }
    return msg;
}

#define UNKNOWN_FUNC "(unknown function)" /* the default for OPENSSL_FUNC */
/*
 * substitute fallback if component/function name is NULL or empty or contains
 * just pseudo-information "(unknown function)" due to -pedantic and macros.h
 */
static const char *improve_location_name(const char *func, const char *fallback)
{
    if (fallback == NULL)
        return func == NULL ? UNKNOWN_FUNC : func;

    return func == NULL || *func == '\0' || strcmp(func, UNKNOWN_FUNC) == 0
        ? fallback : func;
}

int OSSL_CMP_print_to_bio(BIO *bio, const char *component, const char *file,
                          int line, OSSL_CMP_severity level, const char *msg)
{
    const char *level_string =
        level == OSSL_CMP_LOG_EMERG ? "EMERG" :
        level == OSSL_CMP_LOG_ALERT ? "ALERT" :
        level == OSSL_CMP_LOG_CRIT ? "CRIT" :
        level == OSSL_CMP_LOG_ERR ? "error" :
        level == OSSL_CMP_LOG_WARNING ? "warning" :
        level == OSSL_CMP_LOG_NOTICE ? "NOTE" :
        level == OSSL_CMP_LOG_INFO ? "info" :
        level == OSSL_CMP_LOG_DEBUG ? "DEBUG" : "(unknown level)";

#ifndef NDEBUG
    if (BIO_printf(bio, "%s:%s:%d:", improve_location_name(component, "CMP"),
                   file, line) < 0)
        return 0;
#endif
    return BIO_printf(bio, OSSL_CMP_LOG_PREFIX"%s: %s\n",
                      level_string, msg) >= 0;
}

#define ERR_PRINT_BUF_SIZE 4096
/* this is similar to ERR_print_errors_cb, but uses the CMP-specific cb type */
void OSSL_CMP_print_errors_cb(OSSL_CMP_log_cb_t log_fn)
{
    unsigned long err;
    char msg[ERR_PRINT_BUF_SIZE];
    const char *file = NULL, *func = NULL, *data = NULL;
    int line, flags;

    while ((err = ERR_get_error_all(&file, &line, &func, &data, &flags)) != 0) {
        const char *component =
            improve_location_name(func, ERR_lib_error_string(err));
        unsigned long reason = ERR_GET_REASON(err);
        const char *rs = NULL;
        char rsbuf[256];

#ifndef OPENSSL_NO_ERR
        if (ERR_SYSTEM_ERROR(err)) {
            if (openssl_strerror_r(reason, rsbuf, sizeof(rsbuf)))
                rs = rsbuf;
        } else {
            rs = ERR_reason_error_string(err);
        }
#endif
        if (rs == NULL) {
            BIO_snprintf(rsbuf, sizeof(rsbuf), "reason(%lu)", reason);
            rs = rsbuf;
        }
        if (data != NULL && (flags & ERR_TXT_STRING) != 0)
            BIO_snprintf(msg, sizeof(msg), "%s:%s", rs, data);
        else
            BIO_snprintf(msg, sizeof(msg), "%s", rs);

        if (log_fn == NULL) {
#ifndef OPENSSL_NO_STDIO
            BIO *bio = BIO_new_fp(stderr, BIO_NOCLOSE);

            if (bio != NULL) {
                OSSL_CMP_print_to_bio(bio, component, file, line,
                                      OSSL_CMP_LOG_ERR, msg);
                BIO_free(bio);
            }
#else
            /* ERR_raise(ERR_LIB_CMP, CMP_R_NO_STDIO) makes no sense during error printing */
#endif
        } else {
            if (log_fn(component, file, line, OSSL_CMP_LOG_ERR, msg) <= 0)
                break; /* abort outputting the error report */
        }
    }
}

int ossl_cmp_X509_STORE_add1_certs(X509_STORE *store, STACK_OF(X509) *certs,
                                   int only_self_signed)
{
    int i;

    if (store == NULL) {
        ERR_raise(ERR_LIB_CMP, CMP_R_NULL_ARGUMENT);
        return 0;
    }
    if (certs == NULL)
        return 1;
    for (i = 0; i < sk_X509_num(certs); i++) {
        X509 *cert = sk_X509_value(certs, i);

        if (!only_self_signed || X509_self_signed(cert, 0) == 1)
            if (!X509_STORE_add_cert(store, cert)) /* ups cert ref counter */
                return 0;
    }
    return 1;
}

int ossl_cmp_sk_ASN1_UTF8STRING_push_str(STACK_OF(ASN1_UTF8STRING) *sk,
                                         const char *text, int len)
{
    ASN1_UTF8STRING *utf8string;

    if (!ossl_assert(sk != NULL && text != NULL))
        return 0;
    if ((utf8string = ASN1_UTF8STRING_new()) == NULL)
        return 0;
    if (!ASN1_STRING_set(utf8string, text, len))
        goto err;
    if (!sk_ASN1_UTF8STRING_push(sk, utf8string))
        goto err;
    return 1;

 err:
    ASN1_UTF8STRING_free(utf8string);
    return 0;
}

int ossl_cmp_asn1_octet_string_set1(ASN1_OCTET_STRING **tgt,
                                    const ASN1_OCTET_STRING *src)
{
    ASN1_OCTET_STRING *new;
    if (tgt == NULL) {
        ERR_raise(ERR_LIB_CMP, CMP_R_NULL_ARGUMENT);
        return 0;
    }
    if (*tgt == src) /* self-assignment */
        return 1;

    if (src != NULL) {
        if ((new = ASN1_OCTET_STRING_dup(src)) == NULL)
            return 0;
    } else {
        new = NULL;
    }

    ASN1_OCTET_STRING_free(*tgt);
    *tgt = new;
    return 1;
}

int ossl_cmp_asn1_octet_string_set1_bytes(ASN1_OCTET_STRING **tgt,
                                          const unsigned char *bytes, int len)
{
    ASN1_OCTET_STRING *new = NULL;

    if (tgt == NULL) {
        ERR_raise(ERR_LIB_CMP, CMP_R_NULL_ARGUMENT);
        return 0;
    }
    if (bytes != NULL) {
        if ((new = ASN1_OCTET_STRING_new()) == NULL
                || !(ASN1_OCTET_STRING_set(new, bytes, len))) {
            ASN1_OCTET_STRING_free(new);
            return 0;
        }
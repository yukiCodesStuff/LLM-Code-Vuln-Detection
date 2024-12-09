/*
 * Copyright 2004-2022 The OpenSSL Project Authors. All Rights Reserved.
 *
 * Licensed under the Apache License 2.0 (the "License").  You may not use
 * this file except in compliance with the License.  You can obtain a copy
 * in the file LICENSE in the source distribution or at
     */
    if (namelen == 0 || name == NULL)
        namelen = name ? strlen(name) : 0;
    else if (name != NULL
             && memchr(name, '\0', namelen > 1 ? namelen - 1 : namelen) != NULL)
        return 0;
    if (namelen > 0 && name[namelen - 1] == '\0')
        --namelen;

    return 1;
}

X509_VERIFY_PARAM *X509_VERIFY_PARAM_new(void)
{
    X509_VERIFY_PARAM *param;

/* Macro to test if a field should be copied from src to dest */

#define test_x509_verify_param_copy(field, def) \
    (to_overwrite || (src->field != def && (to_default || dest->field == def)))

/* Macro to test and copy a field if necessary */

#define x509_verify_param_copy(field, def) \
{
    unsigned long inh_flags;
    int to_default, to_overwrite;

    if (src == NULL)
        return 1;
    inh_flags = dest->inh_flags | src->inh_flags;

    if ((inh_flags & X509_VP_FLAG_ONCE) != 0)
        dest->inh_flags = 0;

    if ((inh_flags & X509_VP_FLAG_LOCKED) != 0)
        return 1;

    to_default = (inh_flags & X509_VP_FLAG_DEFAULT) != 0;
    to_overwrite = (inh_flags & X509_VP_FLAG_OVERWRITE) != 0;

    x509_verify_param_copy(purpose, 0);
    x509_verify_param_copy(trust, X509_TRUST_DEFAULT);
    x509_verify_param_copy(depth, -1);

    /* If overwrite or check time not set, copy across */

    if (to_overwrite || (dest->flags & X509_V_FLAG_USE_CHECK_TIME) == 0) {
        dest->check_time = src->check_time;
        dest->flags &= ~X509_V_FLAG_USE_CHECK_TIME;
        /* Don't need to copy flag: that is done below */
    }

    if ((inh_flags & X509_VP_FLAG_RESET_FLAGS) != 0)
        dest->flags = 0;

    dest->flags |= src->flags;

    if (test_x509_verify_param_copy(hosts, NULL)) {
        sk_OPENSSL_STRING_pop_free(dest->hosts, str_free);
        dest->hosts = NULL;
        if (src->hosts != NULL) {
            dest->hosts =
                sk_OPENSSL_STRING_deep_copy(src->hosts, str_copy, str_free);
            if (dest->hosts == NULL)
                return 0;
int X509_VERIFY_PARAM_set1(X509_VERIFY_PARAM *to,
                           const X509_VERIFY_PARAM *from)
{
    unsigned long save_flags;
    int ret;

    if (to == NULL) {
        ERR_raise(ERR_LIB_X509, ERR_R_PASSED_NULL_PARAMETER);
        return 0;
    }
    save_flags = to->inh_flags;
    to->inh_flags |= X509_VP_FLAG_DEFAULT;
    ret = X509_VERIFY_PARAM_inherit(to, from);
    to->inh_flags = save_flags;
    return ret;
                               const char *src, size_t srclen)
{
    char *tmp;

    if (src != NULL) {
        if (srclen == 0)
            srclen = strlen(src);

        tmp = OPENSSL_malloc(srclen + 1);
{
    OPENSSL_free(param->name);
    param->name = OPENSSL_strdup(name);
    return param->name != NULL;
}

int X509_VERIFY_PARAM_set_flags(X509_VERIFY_PARAM *param, unsigned long flags)
{
    param->flags |= flags;
    if ((flags & X509_V_FLAG_POLICY_MASK) != 0)
        param->flags |= X509_V_FLAG_POLICY_CHECK;
    return 1;
}

        if (param->policies == NULL)
            return 0;
    }
    return sk_ASN1_OBJECT_push(param->policies, policy);
}

int X509_VERIFY_PARAM_set1_policies(X509_VERIFY_PARAM *param,
                                    STACK_OF(ASN1_OBJECT) *policies)
    int i;
    ASN1_OBJECT *oid, *doid;

    if (param == NULL) {
        ERR_raise(ERR_LIB_X509, ERR_R_PASSED_NULL_PARAMETER);
        return 0;
    }
    sk_ASN1_OBJECT_pop_free(param->policies, ASN1_OBJECT_free);

    if (policies == NULL) {
        param->policies = NULL;
    for (i = 0; i < sk_ASN1_OBJECT_num(policies); i++) {
        oid = sk_ASN1_OBJECT_value(policies, i);
        doid = OBJ_dup(oid);
        if (doid == NULL)
            return 0;
        if (!sk_ASN1_OBJECT_push(param->policies, doid)) {
            ASN1_OBJECT_free(doid);
            return 0;
        OPENSSL_free(to->peername);
        to->peername = peername;
    }
    if (from != NULL)
        from->peername = NULL;
}

char *X509_VERIFY_PARAM_get0_email(X509_VERIFY_PARAM *param)
static unsigned char
*int_X509_VERIFY_PARAM_get0_ip(X509_VERIFY_PARAM *param, size_t *plen)
{
    if (param == NULL || param->ip == NULL) {
        ERR_raise(ERR_LIB_X509, ERR_R_PASSED_NULL_PARAMETER);
        return NULL;
    }
    if (plen != NULL)
        *plen = param->iplen;
    return param->ip;
}
    size_t iplen;
    unsigned char *ip = int_X509_VERIFY_PARAM_get0_ip(param, &iplen);

    return ip == NULL ? NULL : ossl_ipaddr_to_asc(ip, iplen);
}

int X509_VERIFY_PARAM_set1_ip(X509_VERIFY_PARAM *param,
                              const unsigned char *ip, size_t iplen)
{
    if (iplen != 0 && iplen != 4 && iplen != 16) {
        ERR_raise(ERR_LIB_X509, ERR_R_PASSED_INVALID_ARGUMENT);
        return 0;
    }
    return int_x509_param_set1((char **)&param->ip, &param->iplen,
                               (char *)ip, iplen);
}

int X509_VERIFY_PARAM_set1_ip_asc(X509_VERIFY_PARAM *param, const char *ipasc)
{
    unsigned char ipout[16];
    size_t iplen = (size_t)ossl_a2i_ipadd(ipout, ipasc);

    if (iplen == 0)
        return 0;
    return X509_VERIFY_PARAM_set1_ip(param, ipout, iplen);
}
{
    int idx;
    X509_VERIFY_PARAM *ptmp;

    if (param_table == NULL) {
        param_table = sk_X509_VERIFY_PARAM_new(param_cmp);
        if (param_table == NULL)
            return 0;
            X509_VERIFY_PARAM_free(ptmp);
        }
    }
    return sk_X509_VERIFY_PARAM_push(param_table, param);
}

int X509_VERIFY_PARAM_get_count(void)
{
    int num = OSSL_NELEM(default_table);

    if (param_table != NULL)
        num += sk_X509_VERIFY_PARAM_num(param_table);
    return num;
}

const X509_VERIFY_PARAM *X509_VERIFY_PARAM_get0(int id)
{
    int num = OSSL_NELEM(default_table);

    if (id < num)
        return default_table + id;
    return sk_X509_VERIFY_PARAM_value(param_table, id - num);
}
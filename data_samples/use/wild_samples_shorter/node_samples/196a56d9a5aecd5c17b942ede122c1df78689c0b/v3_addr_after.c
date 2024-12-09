/*
 * Construct a prefix.
 */
static int make_addressPrefix(IPAddressOrRange **result, unsigned char *addr,
                              const int prefixlen, const int afilen)
{
    int bytelen = (prefixlen + 7) / 8, bitlen = prefixlen % 8;
    IPAddressOrRange *aor = IPAddressOrRange_new();

    if (prefixlen < 0 || prefixlen > (afilen * 8))
        return 0;
    if (aor == NULL)
        return 0;
    aor->type = IPAddressOrRange_addressPrefix;
    if (aor->u.addressPrefix == NULL &&
        return 0;

    if ((prefixlen = range_should_be_prefix(min, max, length)) >= 0)
        return make_addressPrefix(result, min, prefixlen, length);

    if ((aor = IPAddressOrRange_new()) == NULL)
        return 0;
    aor->type = IPAddressOrRange_addressRange;
{
    IPAddressOrRanges *aors = make_prefix_or_range(addr, afi, safi);
    IPAddressOrRange *aor;

    if (aors == NULL
            || !make_addressPrefix(&aor, a, prefixlen, length_from_afi(afi)))
        return 0;
    if (sk_IPAddressOrRange_push(aors, aor))
        return 1;
    IPAddressOrRange_free(aor);
        switch (delim) {
        case '/':
            prefixlen = (int)strtoul(s + i2, &t, 10);
            if (t == s + i2
                    || *t != '\0'
                    || prefixlen > (length * 8)
                    || prefixlen < 0) {
                ERR_raise(ERR_LIB_X509V3, X509V3_R_EXTENSION_VALUE_ERROR);
                X509V3_conf_add_error_name_value(val);
                goto err;
            }
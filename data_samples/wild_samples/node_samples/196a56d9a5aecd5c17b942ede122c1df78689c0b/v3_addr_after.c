    switch (mask) {
    case 0x01:
        j = 7;
        break;
    case 0x03:
        j = 6;
        break;
    case 0x07:
        j = 5;
        break;
    case 0x0F:
        j = 4;
        break;
    case 0x1F:
        j = 3;
        break;
    case 0x3F:
        j = 2;
        break;
    case 0x7F:
        j = 1;
        break;
    default:
        return -1;
    }
    if ((min[i] & mask) != 0 || (max[i] & mask) != mask)
        return -1;
    else
        return i * 8 + j;
}

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
        (aor->u.addressPrefix = ASN1_BIT_STRING_new()) == NULL)
        goto err;
    if (!ASN1_BIT_STRING_set(aor->u.addressPrefix, addr, bytelen))
        goto err;
    aor->u.addressPrefix->flags &= ~7;
    aor->u.addressPrefix->flags |= ASN1_STRING_FLAG_BITS_LEFT;
    if (bitlen > 0) {
        aor->u.addressPrefix->data[bytelen - 1] &= ~(0xFF >> bitlen);
        aor->u.addressPrefix->flags |= 8 - bitlen;
    }

    *result = aor;
    return 1;

 err:
    IPAddressOrRange_free(aor);
    return 0;
}
{
    IPAddressOrRange *aor;
    int i, prefixlen;

    if (memcmp(min, max, length) > 0)
        return 0;

    if ((prefixlen = range_should_be_prefix(min, max, length)) >= 0)
        return make_addressPrefix(result, min, prefixlen, length);

    if ((aor = IPAddressOrRange_new()) == NULL)
        return 0;
    aor->type = IPAddressOrRange_addressRange;
    if ((aor->u.addressRange = IPAddressRange_new()) == NULL)
        goto err;
    if (aor->u.addressRange->min == NULL &&
        (aor->u.addressRange->min = ASN1_BIT_STRING_new()) == NULL)
        goto err;
    if (aor->u.addressRange->max == NULL &&
        (aor->u.addressRange->max = ASN1_BIT_STRING_new()) == NULL)
        goto err;

    for (i = length; i > 0 && min[i - 1] == 0x00; --i) ;
    if (!ASN1_BIT_STRING_set(aor->u.addressRange->min, min, i))
        goto err;
    aor->u.addressRange->min->flags &= ~7;
    aor->u.addressRange->min->flags |= ASN1_STRING_FLAG_BITS_LEFT;
    if (i > 0) {
        unsigned char b = min[i - 1];
        int j = 1;
        while ((b & (0xFFU >> j)) != 0)
            ++j;
        aor->u.addressRange->min->flags |= 8 - j;
    }
{
    IPAddressOrRanges *aors = make_prefix_or_range(addr, afi, safi);
    IPAddressOrRange *aor;

    if (aors == NULL
            || !make_addressPrefix(&aor, a, prefixlen, length_from_afi(afi)))
        return 0;
    if (sk_IPAddressOrRange_push(aors, aor))
        return 1;
    IPAddressOrRange_free(aor);
    return 0;
}

/*
 * Add a range.
 */
int X509v3_addr_add_range(IPAddrBlocks *addr,
                          const unsigned afi,
                          const unsigned *safi,
                          unsigned char *min, unsigned char *max)
{
    IPAddressOrRanges *aors = make_prefix_or_range(addr, afi, safi);
    IPAddressOrRange *aor;
    int length = length_from_afi(afi);
    if (aors == NULL)
        return 0;
    if (!make_addressRange(&aor, min, max, length))
        return 0;
    if (sk_IPAddressOrRange_push(aors, aor))
        return 1;
    IPAddressOrRange_free(aor);
    return 0;
}

/*
 * Extract min and max values from an IPAddressOrRange.
 */
static int extract_min_max(IPAddressOrRange *aor,
                           unsigned char *min, unsigned char *max, int length)
{
    if (aor == NULL || min == NULL || max == NULL)
        return 0;
    switch (aor->type) {
    case IPAddressOrRange_addressPrefix:
        return (addr_expand(min, aor->u.addressPrefix, length, 0x00) &&
                addr_expand(max, aor->u.addressPrefix, length, 0xFF));
    case IPAddressOrRange_addressRange:
        return (addr_expand(min, aor->u.addressRange->min, length, 0x00) &&
                addr_expand(max, aor->u.addressRange->max, length, 0xFF));
    }
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
	return len + digits_len(ct, dptr, limit, shift);
}

static int sip_parse_addr(const struct nf_conn *ct, const char *cp,
			  const char **endp, union nf_inet_addr *addr,
			  const char *limit, bool delim)
{
	const char *end;
	int ret;

	if (!ct)
		return 0;

	switch (nf_ct_l3num(ct)) {
	case AF_INET:
		ret = in4_pton(cp, limit - cp, (u8 *)&addr->ip, -1, &end);
		if (ret == 0)
			return 0;
		break;
	case AF_INET6:
		if (cp < limit && *cp == '[')
			cp++;
		else if (delim)
			return 0;

		ret = in6_pton(cp, limit - cp, (u8 *)&addr->ip6, -1, &end);
		if (ret == 0)
			return 0;

		if (end < limit && *end == ']')
			end++;
		else if (delim)
			return 0;
		break;
	default:
		BUG();
	}

	if (endp)
		*endp = end;
	return 1;
}
	union nf_inet_addr addr;
	const char *aux = dptr;

	if (!sip_parse_addr(ct, dptr, &dptr, &addr, limit, true)) {
		pr_debug("ip: %s parse failed.!\n", dptr);
		return 0;
	}

		return 0;
	dptr += shift;

	if (!sip_parse_addr(ct, dptr, &end, addr, limit, true))
		return -1;
	if (end < limit && *end == ':') {
		end++;
		p = simple_strtoul(end, (char **)&end, 10);
	if (ret == 0)
		return ret;

	if (!sip_parse_addr(ct, dptr + *matchoff, &c, addr, limit, true))
		return -1;
	if (*c == ':') {
		c++;
		p = simple_strtoul(c, (char **)&c, 10);
			       unsigned int dataoff, unsigned int datalen,
			       const char *name,
			       unsigned int *matchoff, unsigned int *matchlen,
			       union nf_inet_addr *addr, bool delim)
{
	const char *limit = dptr + datalen;
	const char *start, *end;

		return 0;

	start += strlen(name);
	if (!sip_parse_addr(ct, start, &end, addr, limit, delim))
		return 0;
	*matchoff = start - dptr;
	*matchlen = end - start;
	return 1;
	return 1;
}

static int sdp_parse_addr(const struct nf_conn *ct, const char *cp,
			  const char **endp, union nf_inet_addr *addr,
			  const char *limit)
{
	const char *end;
	int ret;

	memset(addr, 0, sizeof(*addr));
	switch (nf_ct_l3num(ct)) {
	case AF_INET:
		ret = in4_pton(cp, limit - cp, (u8 *)&addr->ip, -1, &end);
		break;
	case AF_INET6:
		ret = in6_pton(cp, limit - cp, (u8 *)&addr->ip6, -1, &end);
		break;
	default:
		BUG();
	}

	if (ret == 0)
		return 0;
	if (endp)
		*endp = end;
	return 1;
}

/* skip ip address. returns its length. */
static int sdp_addr_len(const struct nf_conn *ct, const char *dptr,
			const char *limit, int *shift)
{
	union nf_inet_addr addr;
	const char *aux = dptr;

	if (!sdp_parse_addr(ct, dptr, &dptr, &addr, limit)) {
		pr_debug("ip: %s parse failed.!\n", dptr);
		return 0;
	}

	return dptr - aux;
}

/* SDP header parsing: a SDP session description contains an ordered set of
 * headers, starting with a section containing general session parameters,
 * optionally followed by multiple media descriptions.
 *
 */
static const struct sip_header ct_sdp_hdrs[] = {
	[SDP_HDR_VERSION]		= SDP_HDR("v=", NULL, digits_len),
	[SDP_HDR_OWNER_IP4]		= SDP_HDR("o=", "IN IP4 ", sdp_addr_len),
	[SDP_HDR_CONNECTION_IP4]	= SDP_HDR("c=", "IN IP4 ", sdp_addr_len),
	[SDP_HDR_OWNER_IP6]		= SDP_HDR("o=", "IN IP6 ", sdp_addr_len),
	[SDP_HDR_CONNECTION_IP6]	= SDP_HDR("c=", "IN IP6 ", sdp_addr_len),
	[SDP_HDR_MEDIA]			= SDP_HDR("m=", NULL, media_len),
};

/* Linear string search within SDP header values */
	if (ret <= 0)
		return ret;

	if (!sdp_parse_addr(ct, dptr + *matchoff, NULL, addr,
			    dptr + *matchoff + *matchlen))
		return -1;
	return 1;
}

}

static struct nf_conntrack_helper sip[MAX_PORTS][4] __read_mostly;

static const struct nf_conntrack_expect_policy sip_exp_policy[SIP_EXPECT_MAX + 1] = {
	[SIP_EXPECT_SIGNALLING] = {
		.name		= "signalling",
			sip[i][j].me = THIS_MODULE;

			if (ports[i] == SIP_PORT)
				sprintf(sip[i][j].name, "sip");
			else
				sprintf(sip[i][j].name, "sip-%u", i);

			pr_debug("port #%u: %u\n", i, ports[i]);

			ret = nf_conntrack_helper_register(&sip[i][j]);
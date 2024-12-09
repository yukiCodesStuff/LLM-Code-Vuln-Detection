{
	struct nlattr *cda[CTA_MAX+1];

	nla_parse_nested(cda, CTA_MAX, attr, ct_nla_policy);

	return ctnetlink_nfqueue_parse_ct((const struct nlattr **)cda, ct);
}
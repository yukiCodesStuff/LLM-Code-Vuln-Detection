ctnetlink_nfqueue_parse(const struct nlattr *attr, struct nf_conn *ct)
{
	struct nlattr *cda[CTA_MAX+1];

	nla_parse_nested(cda, CTA_MAX, attr, ct_nla_policy);

	return ctnetlink_nfqueue_parse_ct((const struct nlattr **)cda, ct);
}

static struct nfq_ct_hook ctnetlink_nfqueue_hook = {
	.build_size	= ctnetlink_nfqueue_build_size,
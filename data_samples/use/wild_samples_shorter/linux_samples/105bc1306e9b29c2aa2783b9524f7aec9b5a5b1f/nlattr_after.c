	[NLA_FLAG]	= 0,
};

static struct nlattr *nla_next(const struct nlattr *nla, int *remaining)
{
	int totlen = NLA_ALIGN(nla->nla_len);

	       nla->nla_len <= remaining;
}

static int nla_type(const struct nlattr *nla)
{
	return nla->nla_type & NLA_TYPE_MASK;
}
 * @see nla_validate
 * @return 0 on success or a negative error code.
 */
int nla_parse(struct nlattr *tb[], int maxtype, struct nlattr *head, int len,
	      struct nla_policy *policy)
{
	struct nlattr *nla;
	int rem, err;

	return err;
}

/**
 * Create attribute index based on nested attribute
 * @arg tb              Index array to be filled (maxtype+1 elements).
 * @arg maxtype         Maximum attribute type expected and accepted.
 * @arg nla             Nested Attribute.
 * @arg policy          Attribute validation policy.
 *
 * Feeds the stream of attributes nested into the specified attribute
 * to nla_parse().
 *
 * @see nla_parse
 * @return 0 on success or a negative error code.
 */
int nla_parse_nested(struct nlattr *tb[], int maxtype, struct nlattr *nla,
		     struct nla_policy *policy)
{
	return nla_parse(tb, maxtype, nla_data(nla), nla_len(nla), policy);
}

/* dump netlink extended ack error message */
int nla_dump_errormsg(struct nlmsghdr *nlh)
{
	struct nla_policy extack_policy[NLMSGERR_ATTR_MAX + 1] = {
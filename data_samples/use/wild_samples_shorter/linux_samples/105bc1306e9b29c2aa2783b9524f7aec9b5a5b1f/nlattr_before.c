	[NLA_FLAG]	= 0,
};

static int nla_len(const struct nlattr *nla)
{
	return nla->nla_len - NLA_HDRLEN;
}

static struct nlattr *nla_next(const struct nlattr *nla, int *remaining)
{
	int totlen = NLA_ALIGN(nla->nla_len);

	       nla->nla_len <= remaining;
}

static void *nla_data(const struct nlattr *nla)
{
	return (char *) nla + NLA_HDRLEN;
}

static int nla_type(const struct nlattr *nla)
{
	return nla->nla_type & NLA_TYPE_MASK;
}
 * @see nla_validate
 * @return 0 on success or a negative error code.
 */
static int nla_parse(struct nlattr *tb[], int maxtype, struct nlattr *head, int len,
		     struct nla_policy *policy)
{
	struct nlattr *nla;
	int rem, err;

	return err;
}

/* dump netlink extended ack error message */
int nla_dump_errormsg(struct nlmsghdr *nlh)
{
	struct nla_policy extack_policy[NLMSGERR_ATTR_MAX + 1] = {
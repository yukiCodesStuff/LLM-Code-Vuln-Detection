	     nla_ok(pos, rem); \
	     pos = nla_next(pos, &(rem)))

/**
 * nla_data - head of payload
 * @nla: netlink attribute
 */
static inline void *nla_data(const struct nlattr *nla)
{
	return (char *) nla + NLA_HDRLEN;
}

static inline uint8_t nla_getattr_u8(const struct nlattr *nla)
{
	return *(uint8_t *)nla_data(nla);
}

static inline uint32_t nla_getattr_u32(const struct nlattr *nla)
{
	return *(uint32_t *)nla_data(nla);
}

static inline const char *nla_getattr_str(const struct nlattr *nla)
{
	return (const char *)nla_data(nla);
}

/**
 * nla_len - length of payload
 * @nla: netlink attribute
 */
static inline int nla_len(const struct nlattr *nla)
{
	return nla->nla_len - NLA_HDRLEN;
}

int nla_parse(struct nlattr *tb[], int maxtype, struct nlattr *head, int len,
	      struct nla_policy *policy);
int nla_parse_nested(struct nlattr *tb[], int maxtype, struct nlattr *nla,
		     struct nla_policy *policy);

int nla_dump_errormsg(struct nlmsghdr *nlh);

#endif /* __NLATTR_H */
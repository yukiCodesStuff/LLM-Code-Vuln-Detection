	[XFRMA_SET_MARK]	= { .type = NLA_U32 },
	[XFRMA_SET_MARK_MASK]	= { .type = NLA_U32 },
	[XFRMA_IF_ID]		= { .type = NLA_U32 },
	[XFRMA_MTIMER_THRESH]   = { .type = NLA_U32 },
};
EXPORT_SYMBOL_GPL(xfrma_policy);

static const struct nla_policy xfrma_spd_policy[XFRMA_SPD_MAX+1] = {
	[VDPA_ATTR_DEV_NET_CFG_MACADDR] = NLA_POLICY_ETH_ADDR,
	/* virtio spec 1.1 section 5.1.4.1 for valid MTU range */
	[VDPA_ATTR_DEV_NET_CFG_MTU] = NLA_POLICY_MIN(NLA_U16, 68),
	[VDPA_ATTR_DEV_FEATURES] = { .type = NLA_U64 },
};

static const struct genl_ops vdpa_nl_ops[] = {
	{
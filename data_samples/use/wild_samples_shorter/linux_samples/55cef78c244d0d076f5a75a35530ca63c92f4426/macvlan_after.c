	[IFLA_MACVLAN_MACADDR_COUNT] = { .type = NLA_U32 },
	[IFLA_MACVLAN_BC_QUEUE_LEN] = { .type = NLA_U32 },
	[IFLA_MACVLAN_BC_QUEUE_LEN_USED] = { .type = NLA_REJECT },
	[IFLA_MACVLAN_BC_CUTOFF] = { .type = NLA_S32 },
};

int macvlan_link_register(struct rtnl_link_ops *ops)
{
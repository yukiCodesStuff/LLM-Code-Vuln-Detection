	"legacy-rx",
#define IXGBE_PRIV_FLAGS_VF_IPSEC_EN	BIT(1)
	"vf-ipsec",
#define IXGBE_PRIV_FLAGS_AUTO_DISABLE_VF	BIT(2)
	"mdd-disable-vf",
};

#define IXGBE_PRIV_FLAGS_STR_LEN ARRAY_SIZE(ixgbe_priv_flags_strings)

	if (adapter->flags2 & IXGBE_FLAG2_VF_IPSEC_ENABLED)
		priv_flags |= IXGBE_PRIV_FLAGS_VF_IPSEC_EN;

	if (adapter->flags2 & IXGBE_FLAG2_AUTO_DISABLE_VF)
		priv_flags |= IXGBE_PRIV_FLAGS_AUTO_DISABLE_VF;

	return priv_flags;
}

static int ixgbe_set_priv_flags(struct net_device *netdev, u32 priv_flags)
{
	struct ixgbe_adapter *adapter = netdev_priv(netdev);
	unsigned int flags2 = adapter->flags2;
	unsigned int i;

	flags2 &= ~IXGBE_FLAG2_RX_LEGACY;
	if (priv_flags & IXGBE_PRIV_FLAGS_LEGACY_RX)
		flags2 |= IXGBE_FLAG2_RX_LEGACY;
	if (priv_flags & IXGBE_PRIV_FLAGS_VF_IPSEC_EN)
		flags2 |= IXGBE_FLAG2_VF_IPSEC_ENABLED;

	flags2 &= ~IXGBE_FLAG2_AUTO_DISABLE_VF;
	if (priv_flags & IXGBE_PRIV_FLAGS_AUTO_DISABLE_VF) {
		if (adapter->hw.mac.type == ixgbe_mac_82599EB) {
			/* Reset primary abort counter */
			for (i = 0; i < adapter->num_vfs; i++)
				adapter->vfinfo[i].primary_abort_count = 0;

			flags2 |= IXGBE_FLAG2_AUTO_DISABLE_VF;
		} else {
			e_info(probe,
			       "Cannot set private flags: Operation not supported\n");
			return -EOPNOTSUPP;
		}
	}

	if (flags2 != adapter->flags2) {
		adapter->flags2 = flags2;

		/* reset interface to repopulate queues */
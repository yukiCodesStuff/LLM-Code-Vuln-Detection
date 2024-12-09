static const char ixgbe_priv_flags_strings[][ETH_GSTRING_LEN] = {
#define IXGBE_PRIV_FLAGS_LEGACY_RX	BIT(0)
	"legacy-rx",
#define IXGBE_PRIV_FLAGS_VF_IPSEC_EN	BIT(1)
	"vf-ipsec",
};

#define IXGBE_PRIV_FLAGS_STR_LEN ARRAY_SIZE(ixgbe_priv_flags_strings)

#define ixgbe_isbackplane(type) ((type) == ixgbe_media_type_backplane)

static void ixgbe_set_supported_10gtypes(struct ixgbe_hw *hw,
					 struct ethtool_link_ksettings *cmd)
{
	if (!ixgbe_isbackplane(hw->phy.media_type)) {
		ethtool_link_ksettings_add_link_mode(cmd, supported,
						     10000baseT_Full);
		return;
	}

	switch (hw->device_id) {
	case IXGBE_DEV_ID_82598:
	case IXGBE_DEV_ID_82599_KX4:
	case IXGBE_DEV_ID_82599_KX4_MEZZ:
	case IXGBE_DEV_ID_X550EM_X_KX4:
		ethtool_link_ksettings_add_link_mode
			(cmd, supported, 10000baseKX4_Full);
		break;
	case IXGBE_DEV_ID_82598_BX:
	case IXGBE_DEV_ID_82599_KR:
	case IXGBE_DEV_ID_X550EM_X_KR:
	case IXGBE_DEV_ID_X550EM_X_XFI:
		ethtool_link_ksettings_add_link_mode
			(cmd, supported, 10000baseKR_Full);
		break;
	default:
		ethtool_link_ksettings_add_link_mode
			(cmd, supported, 10000baseKX4_Full);
		ethtool_link_ksettings_add_link_mode
			(cmd, supported, 10000baseKR_Full);
		break;
	}
}
{
	struct ixgbe_adapter *adapter = netdev_priv(netdev);
	u32 priv_flags = 0;

	if (adapter->flags2 & IXGBE_FLAG2_RX_LEGACY)
		priv_flags |= IXGBE_PRIV_FLAGS_LEGACY_RX;

	if (adapter->flags2 & IXGBE_FLAG2_VF_IPSEC_ENABLED)
		priv_flags |= IXGBE_PRIV_FLAGS_VF_IPSEC_EN;

	return priv_flags;
}

static int ixgbe_set_priv_flags(struct net_device *netdev, u32 priv_flags)
{
	struct ixgbe_adapter *adapter = netdev_priv(netdev);
	unsigned int flags2 = adapter->flags2;

	flags2 &= ~IXGBE_FLAG2_RX_LEGACY;
	if (priv_flags & IXGBE_PRIV_FLAGS_LEGACY_RX)
		flags2 |= IXGBE_FLAG2_RX_LEGACY;

	flags2 &= ~IXGBE_FLAG2_VF_IPSEC_ENABLED;
	if (priv_flags & IXGBE_PRIV_FLAGS_VF_IPSEC_EN)
		flags2 |= IXGBE_FLAG2_VF_IPSEC_ENABLED;

	if (flags2 != adapter->flags2) {
		adapter->flags2 = flags2;

		/* reset interface to repopulate queues */
		if (netif_running(netdev))
			ixgbe_reinit_locked(adapter);
	}
{
	struct ixgbe_adapter *adapter = netdev_priv(netdev);
	unsigned int flags2 = adapter->flags2;

	flags2 &= ~IXGBE_FLAG2_RX_LEGACY;
	if (priv_flags & IXGBE_PRIV_FLAGS_LEGACY_RX)
		flags2 |= IXGBE_FLAG2_RX_LEGACY;

	flags2 &= ~IXGBE_FLAG2_VF_IPSEC_ENABLED;
	if (priv_flags & IXGBE_PRIV_FLAGS_VF_IPSEC_EN)
		flags2 |= IXGBE_FLAG2_VF_IPSEC_ENABLED;

	if (flags2 != adapter->flags2) {
		adapter->flags2 = flags2;

		/* reset interface to repopulate queues */
		if (netif_running(netdev))
			ixgbe_reinit_locked(adapter);
	}
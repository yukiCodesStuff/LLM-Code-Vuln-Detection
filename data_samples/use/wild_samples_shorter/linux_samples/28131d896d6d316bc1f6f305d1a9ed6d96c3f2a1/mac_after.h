 * @FLEXIBLE_TWT_SUPPORTED: AP supports flexible TWT schedule
 * @PROTECTED_TWT_SUPPORTED: AP supports protected TWT frames (with 11w)
 * @BROADCAST_TWT_SUPPORTED: AP and STA support broadcast TWT
 * @COEX_HIGH_PRIORITY_ENABLE: high priority mode for BT coex, to be used
 *	during 802.1X negotiation (and allowed during 4-way-HS)
 */
enum iwl_mac_data_policy {
	TWT_SUPPORTED = BIT(0),
	MORE_DATA_ACK_SUPPORTED = BIT(1),
	FLEXIBLE_TWT_SUPPORTED = BIT(2),
	PROTECTED_TWT_SUPPORTED = BIT(3),
	BROADCAST_TWT_SUPPORTED = BIT(4),
	COEX_HIGH_PRIORITY_ENABLE = BIT(5),
};

/**
 * struct iwl_mac_data_sta - configuration data for station MAC context
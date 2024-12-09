 * @FLEXIBLE_TWT_SUPPORTED: AP supports flexible TWT schedule
 * @PROTECTED_TWT_SUPPORTED: AP supports protected TWT frames (with 11w)
 * @BROADCAST_TWT_SUPPORTED: AP and STA support broadcast TWT
 */
enum iwl_mac_data_policy {
	TWT_SUPPORTED = BIT(0),
	MORE_DATA_ACK_SUPPORTED = BIT(1),
	FLEXIBLE_TWT_SUPPORTED = BIT(2),
	PROTECTED_TWT_SUPPORTED = BIT(3),
	BROADCAST_TWT_SUPPORTED = BIT(4),
};

/**
 * struct iwl_mac_data_sta - configuration data for station MAC context
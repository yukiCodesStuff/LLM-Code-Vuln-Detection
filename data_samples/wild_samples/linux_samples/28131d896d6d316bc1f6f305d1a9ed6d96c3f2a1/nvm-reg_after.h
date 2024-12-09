struct iwl_lari_config_change_cmd_v4 {
	__le32 config_bitmap;
	__le32 oem_uhb_allow_bitmap;
	__le32 oem_11ax_allow_bitmap;
	__le32 oem_unii4_allow_bitmap;
} __packed; /* LARI_CHANGE_CONF_CMD_S_VER_4 */

/**
 * struct iwl_lari_config_change_cmd_v5 - change LARI configuration
 * @config_bitmap: Bitmap of the config commands. Each bit will trigger a
 *     different predefined FW config operation.
 * @oem_uhb_allow_bitmap: Bitmap of UHB enabled MCC sets.
 * @oem_11ax_allow_bitmap: Bitmap of 11ax allowed MCCs. There are two bits
 *     per country, one to indicate whether to override and the other to
 *     indicate the value to use.
 * @oem_unii4_allow_bitmap: Bitmap of unii4 allowed MCCs.There are two bits
 *     per country, one to indicate whether to override and the other to
 *     indicate allow/disallow unii4 channels.
 * @chan_state_active_bitmap: Bitmap for overriding channel state to active.
 *     Each bit represents a country or region to activate, according to the BIOS
 *     definitions.
 */
struct iwl_lari_config_change_cmd_v5 {
	__le32 config_bitmap;
	__le32 oem_uhb_allow_bitmap;
	__le32 oem_11ax_allow_bitmap;
	__le32 oem_unii4_allow_bitmap;
	__le32 chan_state_active_bitmap;
} __packed; /* LARI_CHANGE_CONF_CMD_S_VER_5 */

/**
 * struct iwl_pnvm_init_complete_ntfy - PNVM initialization complete
 * @status: PNVM image loading status
 */
struct iwl_pnvm_init_complete_ntfy {
	__le32 status;
} __packed; /* PNVM_INIT_COMPLETE_NTFY_S_VER_1 */

#endif /* __iwl_fw_api_nvm_reg_h__ */
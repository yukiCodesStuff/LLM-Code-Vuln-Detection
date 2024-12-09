struct iwl_lari_config_change_cmd_v4 {
	__le32 config_bitmap;
	__le32 oem_uhb_allow_bitmap;
	__le32 oem_11ax_allow_bitmap;
	__le32 oem_unii4_allow_bitmap;
} __packed; /* LARI_CHANGE_CONF_CMD_S_VER_4 */

/**
 * struct iwl_pnvm_init_complete_ntfy - PNVM initialization complete
 * @status: PNVM image loading status
 */
struct iwl_pnvm_init_complete_ntfy {
	__le32 status;
} __packed; /* PNVM_INIT_COMPLETE_NTFY_S_VER_1 */

#endif /* __iwl_fw_api_nvm_reg_h__ */
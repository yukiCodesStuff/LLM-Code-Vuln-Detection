struct iwl_buf_alloc_cmd {
	__le32 alloc_id;
	__le32 buf_location;
	__le32 num_frags;
	struct iwl_buf_alloc_frag frags[BUF_ALLOC_MAX_NUM_FRAGS];
} __packed; /* BUFFER_ALLOCATION_CMD_API_S_VER_2 */

/**
 * struct iwl_dbg_host_event_cfg_cmd
 * @enabled_severities: enabled severities
 */
struct iwl_dbg_host_event_cfg_cmd {
	__le32 enabled_severities;
} __packed; /* DEBUG_HOST_EVENT_CFG_CMD_API_S_VER_1 */

#endif /* __iwl_fw_api_debug_h__ */
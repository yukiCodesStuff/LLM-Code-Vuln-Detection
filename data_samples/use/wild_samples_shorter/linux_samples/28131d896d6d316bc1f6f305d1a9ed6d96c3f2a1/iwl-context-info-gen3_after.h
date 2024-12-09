 * struct iwl_prph_scratch_hwm_cfg - hwm config
 * @hwm_base_addr: hwm start address
 * @hwm_size: hwm size in DWs
 * @debug_token_config: debug preset
 */
struct iwl_prph_scratch_hwm_cfg {
	__le64 hwm_base_addr;
	__le32 hwm_size;
	__le32 debug_token_config;
} __packed; /* PERIPH_SCRATCH_HWM_CFG_S */

/*
 * struct iwl_prph_scratch_rbd_cfg - RBDs configuration
struct iwl_prph_scratch_pnvm_cfg {
	__le64 pnvm_base_addr;
	__le32 pnvm_size;
	__le32 reserved;
} __packed; /* PERIPH_SCRATCH_PNVM_CFG_S */

/*
 * struct iwl_prph_scratch_hwm_cfg - hwm config
 * @hwm_base_addr: hwm start address
 * @hwm_size: hwm size in DWs
 * @reserved: reserved
 */
struct iwl_prph_scratch_hwm_cfg {
	__le64 hwm_base_addr;
	__le32 hwm_size;
	__le32 reserved;
} __packed; /* PERIPH_SCRATCH_HWM_CFG_S */

/*
 * struct iwl_prph_scratch_rbd_cfg - RBDs configuration
 * @free_rbd_addr: default queue free RB CB base address
 * @reserved: reserved
 */
struct iwl_prph_scratch_rbd_cfg {
	__le64 free_rbd_addr;
	__le32 reserved;
} __packed; /* PERIPH_SCRATCH_RBD_CFG_S */

/*
 * struct iwl_prph_scratch_uefi_cfg - prph scratch reduce power table
 * @base_addr: reduce power table address
 * @size: table size in dwords
 */
struct iwl_prph_scratch_uefi_cfg {
	__le64 base_addr;
	__le32 size;
	__le32 reserved;
} __packed; /* PERIPH_SCRATCH_UEFI_CFG_S */

/*
 * struct iwl_prph_scratch_ctrl_cfg - prph scratch ctrl and config
 * @version: version information of context info and HW
 * @control: control flags of FH configurations
 * @pnvm_cfg: ror configuration
 * @hwm_cfg: hwm configuration
 * @rbd_cfg: default RX queue configuration
 */
struct iwl_prph_scratch_ctrl_cfg {
	struct iwl_prph_scratch_version version;
	struct iwl_prph_scratch_control control;
	struct iwl_prph_scratch_pnvm_cfg pnvm_cfg;
	struct iwl_prph_scratch_hwm_cfg hwm_cfg;
	struct iwl_prph_scratch_rbd_cfg rbd_cfg;
	struct iwl_prph_scratch_uefi_cfg reduce_power_cfg;
} __packed; /* PERIPH_SCRATCH_CTRL_CFG_S */

/*
 * struct iwl_prph_scratch - peripheral scratch mapping
 * @ctrl_cfg: control and configuration of prph scratch
 * @dram: firmware images addresses in DRAM
 * @reserved: reserved
 */
struct iwl_prph_scratch {
	struct iwl_prph_scratch_ctrl_cfg ctrl_cfg;
	__le32 reserved[12];
	struct iwl_context_info_dram dram;
} __packed; /* PERIPH_SCRATCH_S */

/*
 * struct iwl_prph_info - peripheral information
 * @boot_stage_mirror: reflects the value in the Boot Stage CSR register
 * @ipc_status_mirror: reflects the value in the IPC Status CSR register
 * @sleep_notif: indicates the peripheral sleep status
 * @reserved: reserved
 */
struct iwl_prph_info {
	__le32 boot_stage_mirror;
	__le32 ipc_status_mirror;
	__le32 sleep_notif;
	__le32 reserved;
} __packed; /* PERIPH_INFO_S */

/*
 * struct iwl_context_info_gen3 - device INIT configuration
 * @version: version of the context information
 * @size: size of context information in DWs
 * @config: context in which the peripheral would execute - a subset of
 *	capability csr register published by the peripheral
 * @prph_info_base_addr: the peripheral information structure start address
 * @cr_head_idx_arr_base_addr: the completion ring head index array
 *	start address
 * @tr_tail_idx_arr_base_addr: the transfer ring tail index array
 *	start address
 * @cr_tail_idx_arr_base_addr: the completion ring tail index array
 *	start address
 * @tr_head_idx_arr_base_addr: the transfer ring head index array
 *	start address
 * @cr_idx_arr_size: number of entries in the completion ring index array
 * @tr_idx_arr_size: number of entries in the transfer ring index array
 * @mtr_base_addr: the message transfer ring start address
 * @mcr_base_addr: the message completion ring start address
 * @mtr_size: number of entries which the message transfer ring can hold
 * @mcr_size: number of entries which the message completion ring can hold
 * @mtr_doorbell_vec: the doorbell vector associated with the message
 *	transfer ring
 * @mcr_doorbell_vec: the doorbell vector associated with the message
 *	completion ring
 * @mtr_msi_vec: the MSI which shall be generated by the peripheral after
 *	completing a transfer descriptor in the message transfer ring
 * @mcr_msi_vec: the MSI which shall be generated by the peripheral after
 *	completing a completion descriptor in the message completion ring
 * @mtr_opt_header_size: the size of the optional header in the transfer
 *	descriptor associated with the message transfer ring in DWs
 * @mtr_opt_footer_size: the size of the optional footer in the transfer
 *	descriptor associated with the message transfer ring in DWs
 * @mcr_opt_header_size: the size of the optional header in the completion
 *	descriptor associated with the message completion ring in DWs
 * @mcr_opt_footer_size: the size of the optional footer in the completion
 *	descriptor associated with the message completion ring in DWs
 * @msg_rings_ctrl_flags: message rings control flags
 * @prph_info_msi_vec: the MSI which shall be generated by the peripheral
 *	after updating the Peripheral Information structure
 * @prph_scratch_base_addr: the peripheral scratch structure start address
 * @prph_scratch_size: the size of the peripheral scratch structure in DWs
 * @reserved: reserved
 */
struct iwl_context_info_gen3 {
	__le16 version;
	__le16 size;
	__le32 config;
	__le64 prph_info_base_addr;
	__le64 cr_head_idx_arr_base_addr;
	__le64 tr_tail_idx_arr_base_addr;
	__le64 cr_tail_idx_arr_base_addr;
	__le64 tr_head_idx_arr_base_addr;
	__le16 cr_idx_arr_size;
	__le16 tr_idx_arr_size;
	__le64 mtr_base_addr;
	__le64 mcr_base_addr;
	__le16 mtr_size;
	__le16 mcr_size;
	__le16 mtr_doorbell_vec;
	__le16 mcr_doorbell_vec;
	__le16 mtr_msi_vec;
	__le16 mcr_msi_vec;
	u8 mtr_opt_header_size;
	u8 mtr_opt_footer_size;
	u8 mcr_opt_header_size;
	u8 mcr_opt_footer_size;
	__le16 msg_rings_ctrl_flags;
	__le16 prph_info_msi_vec;
	__le64 prph_scratch_base_addr;
	__le32 prph_scratch_size;
	__le32 reserved;
} __packed; /* IPC_CONTEXT_INFO_S */

int iwl_pcie_ctxt_info_gen3_init(struct iwl_trans *trans,
				 const struct fw_img *fw);
void iwl_pcie_ctxt_info_gen3_free(struct iwl_trans *trans, bool alive);

int iwl_trans_pcie_ctx_info_gen3_set_pnvm(struct iwl_trans *trans,
					  const void *data, u32 len);
int iwl_trans_pcie_ctx_info_gen3_set_reduce_power(struct iwl_trans *trans,
						  const void *data, u32 len);

#endif /* __iwl_context_info_file_gen3_h__ */
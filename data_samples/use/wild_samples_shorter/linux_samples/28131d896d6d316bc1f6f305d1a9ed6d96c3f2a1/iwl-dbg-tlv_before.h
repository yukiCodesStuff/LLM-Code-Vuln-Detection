 * @trig_list: list of triggers
 * @active_trig_list: list of active triggers
 * @hcmd_list: list of host commands
 */
struct iwl_dbg_tlv_time_point_data {
	struct list_head trig_list;
	struct list_head active_trig_list;
	struct list_head hcmd_list;
};

struct iwl_trans;
struct iwl_fw_runtime;
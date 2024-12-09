struct iwl_fw_ini_dump_cfg_name {
	__le32 image_type;
	__le32 cfg_name_len;
	u8 cfg_name[IWL_FW_INI_MAX_CFG_NAME];
} __packed;

/* AX210's HW type */
#define IWL_AX210_HW_TYPE 0x42
/* How many bits to roll when adding to the HW type of AX210 HW */
#define IWL_AX210_HW_TYPE_ADDITION_SHIFT 12

/* struct iwl_fw_ini_dump_info - ini dump information
 * @version: dump version
 * @time_point: time point that caused the dump collection
 * @trigger_reason: reason of the trigger
 * @external_cfg_state: &enum iwl_ini_cfg_state
 * @ver_type: FW version type
 * @ver_subtype: FW version subype
 * @hw_step: HW step
 * @hw_type: HW type
 * @rf_id_flavor: HW RF id flavor
 * @rf_id_dash: HW RF id dash
 * @rf_id_step: HW RF id step
 * @rf_id_type: HW RF id type
 * @lmac_major: lmac major version
 * @lmac_minor: lmac minor version
 * @umac_major: umac major version
 * @umac_minor: umac minor version
 * @fw_mon_mode: FW monitor mode &enum iwl_fw_ini_buffer_location
 * @regions_mask: bitmap mask of regions ids in the dump
 * @build_tag_len: length of the build tag
 * @build_tag: build tag string
 * @num_of_cfg_names: number of configuration name structs
 * @cfg_names: configuration names
 */
struct iwl_fw_ini_dump_info {
	__le32 version;
	__le32 time_point;
	__le32 trigger_reason;
	__le32 external_cfg_state;
	__le32 ver_type;
	__le32 ver_subtype;
	__le32 hw_step;
	__le32 hw_type;
	__le32 rf_id_flavor;
	__le32 rf_id_dash;
	__le32 rf_id_step;
	__le32 rf_id_type;
	__le32 lmac_major;
	__le32 lmac_minor;
	__le32 umac_major;
	__le32 umac_minor;
	__le32 fw_mon_mode;
	__le64 regions_mask;
	__le32 build_tag_len;
	u8 build_tag[FW_VER_HUMAN_READABLE_SZ];
	__le32 num_of_cfg_names;
	struct iwl_fw_ini_dump_cfg_name cfg_names[];
} __packed;

/**
 * struct iwl_fw_ini_err_table_dump - ini error table dump
 * @header: header of the region
 * @version: error table version
 * @data: data of memory ranges in this region,
 *	see &struct iwl_fw_ini_error_dump_range
 */
struct iwl_fw_ini_err_table_dump {
	struct iwl_fw_ini_error_dump_header header;
	__le32 version;
	u8 data[];
} __packed;

/**
 * struct iwl_fw_error_dump_rb - content of an Receive Buffer
 * @index: the index of the Receive Buffer in the Rx queue
 * @rxq: the RB's Rx queue
 * @reserved:
 * @data: the content of the Receive Buffer
 */
struct iwl_fw_error_dump_rb {
	__le32 index;
	__le32 rxq;
	__le32 reserved;
	u8 data[];
};

/**
 * struct iwl_fw_ini_monitor_dump - ini monitor dump
 * @header: header of the region
 * @write_ptr: write pointer position in the buffer
 * @cycle_cnt: cycles count
 * @cur_frag: current fragment in use
 * @data: data of memory ranges in this region,
 *	see &struct iwl_fw_ini_error_dump_range
 */
struct iwl_fw_ini_monitor_dump {
	struct iwl_fw_ini_error_dump_header header;
	__le32 write_ptr;
	__le32 cycle_cnt;
	__le32 cur_frag;
	u8 data[];
} __packed;

/**
 * struct iwl_fw_ini_special_device_memory - special device memory
 * @header: header of the region
 * @type: type of special memory
 * @version: struct special memory version
 * @data: data of memory ranges in this region,
 *	see &struct iwl_fw_ini_error_dump_range
 */
struct iwl_fw_ini_special_device_memory {
	struct iwl_fw_ini_error_dump_header header;
	__le16 type;
	__le16 version;
	u8 data[];
} __packed;

/**
 * struct iwl_fw_error_dump_paging - content of the UMAC's image page
 *	block on DRAM
 * @index: the index of the page block
 * @reserved:
 * @data: the content of the page block
 */
struct iwl_fw_error_dump_paging {
	__le32 index;
	__le32 reserved;
	u8 data[];
};

/**
 * iwl_fw_error_next_data - advance fw error dump data pointer
 * @data: previous data block
 * Returns: next data block
 */
static inline struct iwl_fw_error_dump_data *
iwl_fw_error_next_data(struct iwl_fw_error_dump_data *data)
{
	return (void *)(data->data + le32_to_cpu(data->len));
}

/**
 * enum iwl_fw_dbg_trigger - triggers available
 *
 * @FW_DBG_TRIGGER_USER: trigger log collection by user
 *	This should not be defined as a trigger to the driver, but a value the
 *	driver should set to indicate that the trigger was initiated by the
 *	user.
 * @FW_DBG_TRIGGER_FW_ASSERT: trigger log collection when the firmware asserts
 * @FW_DBG_TRIGGER_MISSED_BEACONS: trigger log collection when beacons are
 *	missed.
 * @FW_DBG_TRIGGER_CHANNEL_SWITCH: trigger log collection upon channel switch.
 * @FW_DBG_TRIGGER_FW_NOTIF: trigger log collection when the firmware sends a
 *	command response or a notification.
 * @FW_DBG_TRIGGER_MLME: trigger log collection upon MLME event.
 * @FW_DBG_TRIGGER_STATS: trigger log collection upon statistics threshold.
 * @FW_DBG_TRIGGER_RSSI: trigger log collection when the rssi of the beacon
 *	goes below a threshold.
 * @FW_DBG_TRIGGER_TXQ_TIMERS: configures the timers for the Tx queue hang
 *	detection.
 * @FW_DBG_TRIGGER_TIME_EVENT: trigger log collection upon time events related
 *	events.
 * @FW_DBG_TRIGGER_BA: trigger log collection upon BlockAck related events.
 * @FW_DBG_TX_LATENCY: trigger log collection when the tx latency goes above a
 *	threshold.
 * @FW_DBG_TDLS: trigger log collection upon TDLS related events.
 * @FW_DBG_TRIGGER_TX_STATUS: trigger log collection upon tx status when
 *  the firmware sends a tx reply.
 * @FW_DBG_TRIGGER_ALIVE_TIMEOUT: trigger log collection if alive flow timeouts
 * @FW_DBG_TRIGGER_DRIVER: trigger log collection upon a flow failure
 *	in the driver.
 */
enum iwl_fw_dbg_trigger {
	FW_DBG_TRIGGER_INVALID = 0,
	FW_DBG_TRIGGER_USER,
	FW_DBG_TRIGGER_FW_ASSERT,
	FW_DBG_TRIGGER_MISSED_BEACONS,
	FW_DBG_TRIGGER_CHANNEL_SWITCH,
	FW_DBG_TRIGGER_FW_NOTIF,
	FW_DBG_TRIGGER_MLME,
	FW_DBG_TRIGGER_STATS,
	FW_DBG_TRIGGER_RSSI,
	FW_DBG_TRIGGER_TXQ_TIMERS,
	FW_DBG_TRIGGER_TIME_EVENT,
	FW_DBG_TRIGGER_BA,
	FW_DBG_TRIGGER_TX_LATENCY,
	FW_DBG_TRIGGER_TDLS,
	FW_DBG_TRIGGER_TX_STATUS,
	FW_DBG_TRIGGER_ALIVE_TIMEOUT,
	FW_DBG_TRIGGER_DRIVER,

	/* must be last */
	FW_DBG_TRIGGER_MAX,
};

/**
 * struct iwl_fw_error_dump_trigger_desc - describes the trigger condition
 * @type: &enum iwl_fw_dbg_trigger
 * @data: raw data about what happened
 */
struct iwl_fw_error_dump_trigger_desc {
	__le32 type;
	u8 data[];
};

#endif /* __fw_error_dump_h__ */
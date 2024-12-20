struct iwl_hcmd_arr {
	const struct iwl_hcmd_names *arr;
	int size;
};

#define HCMD_ARR(x)	\
	{ .arr = x, .size = ARRAY_SIZE(x) }

/**
 * struct iwl_trans_config - transport configuration
 *
 * @op_mode: pointer to the upper layer.
 * @cmd_queue: the index of the command queue.
 *	Must be set before start_fw.
 * @cmd_fifo: the fifo for host commands
 * @cmd_q_wdg_timeout: the timeout of the watchdog timer for the command queue.
 * @no_reclaim_cmds: Some devices erroneously don't set the
 *	SEQ_RX_FRAME bit on some notifications, this is the
 *	list of such notifications to filter. Max length is
 *	%MAX_NO_RECLAIM_CMDS.
 * @n_no_reclaim_cmds: # of commands in list
 * @rx_buf_size: RX buffer size needed for A-MSDUs
 *	if unset 4k will be the RX buffer size
 * @bc_table_dword: set to true if the BC table expects the byte count to be
 *	in DWORD (as opposed to bytes)
 * @scd_set_active: should the transport configure the SCD for HCMD queue
 * @command_groups: array of command groups, each member is an array of the
 *	commands in the group; for debugging only
 * @command_groups_size: number of command groups, to avoid illegal access
 * @cb_data_offs: offset inside skb->cb to store transport data at, must have
 *	space for at least two pointers
 * @fw_reset_handshake: firmware supports reset flow handshake
 */
struct iwl_trans_config {
	struct iwl_op_mode *op_mode;

	u8 cmd_queue;
	u8 cmd_fifo;
	unsigned int cmd_q_wdg_timeout;
	const u8 *no_reclaim_cmds;
	unsigned int n_no_reclaim_cmds;

	enum iwl_amsdu_size rx_buf_size;
	bool bc_table_dword;
	bool scd_set_active;
	const struct iwl_hcmd_arr *command_groups;
	int command_groups_size;

	u8 cb_data_offs;
	bool fw_reset_handshake;
};

struct iwl_trans_dump_data {
	u32 len;
	u8 data[];
};

struct iwl_trans;

struct iwl_trans_txq_scd_cfg {
	u8 fifo;
	u8 sta_id;
	u8 tid;
	bool aggregate;
	int frame_limit;
};

/**
 * struct iwl_trans_rxq_dma_data - RX queue DMA data
 * @fr_bd_cb: DMA address of free BD cyclic buffer
 * @fr_bd_wid: Initial write index of the free BD cyclic buffer
 * @urbd_stts_wrptr: DMA address of urbd_stts_wrptr
 * @ur_bd_cb: DMA address of used BD cyclic buffer
 */
struct iwl_trans_rxq_dma_data {
	u64 fr_bd_cb;
	u32 fr_bd_wid;
	u64 urbd_stts_wrptr;
	u64 ur_bd_cb;
};

/**
 * struct iwl_trans_ops - transport specific operations
 *
 * All the handlers MUST be implemented
 *
 * @start_hw: starts the HW. From that point on, the HW can send interrupts.
 *	May sleep.
 * @op_mode_leave: Turn off the HW RF kill indication if on
 *	May sleep
 * @start_fw: allocates and inits all the resources for the transport
 *	layer. Also kick a fw image.
 *	May sleep
 * @fw_alive: called when the fw sends alive notification. If the fw provides
 *	the SCD base address in SRAM, then provide it here, or 0 otherwise.
 *	May sleep
 * @stop_device: stops the whole device (embedded CPU put to reset) and stops
 *	the HW. From that point on, the HW will be stopped but will still issue
 *	an interrupt if the HW RF kill switch is triggered.
 *	This callback must do the right thing and not crash even if %start_hw()
 *	was called but not &start_fw(). May sleep.
 * @d3_suspend: put the device into the correct mode for WoWLAN during
 *	suspend. This is optional, if not implemented WoWLAN will not be
 *	supported. This callback may sleep.
 * @d3_resume: resume the device after WoWLAN, enabling the opmode to
 *	talk to the WoWLAN image to get its status. This is optional, if not
 *	implemented WoWLAN will not be supported. This callback may sleep.
 * @send_cmd:send a host command. Must return -ERFKILL if RFkill is asserted.
 *	If RFkill is asserted in the middle of a SYNC host command, it must
 *	return -ERFKILL straight away.
 *	May sleep only if CMD_ASYNC is not set
 * @tx: send an skb. The transport relies on the op_mode to zero the
 *	the ieee80211_tx_info->driver_data. If the MPDU is an A-MSDU, all
 *	the CSUM will be taken care of (TCP CSUM and IP header in case of
 *	IPv4). If the MPDU is a single MSDU, the op_mode must compute the IP
 *	header if it is IPv4.
 *	Must be atomic
 * @reclaim: free packet until ssn. Returns a list of freed packets.
 *	Must be atomic
 * @txq_enable: setup a queue. To setup an AC queue, use the
 *	iwl_trans_ac_txq_enable wrapper. fw_alive must have been called before
 *	this one. The op_mode must not configure the HCMD queue. The scheduler
 *	configuration may be %NULL, in which case the hardware will not be
 *	configured. If true is returned, the operation mode needs to increment
 *	the sequence number of the packets routed to this queue because of a
 *	hardware scheduler bug. May sleep.
 * @txq_disable: de-configure a Tx queue to send AMPDUs
 *	Must be atomic
 * @txq_set_shared_mode: change Tx queue shared/unshared marking
 * @wait_tx_queues_empty: wait until tx queues are empty. May sleep.
 * @wait_txq_empty: wait until specific tx queue is empty. May sleep.
 * @freeze_txq_timer: prevents the timer of the queue from firing until the
 *	queue is set to awake. Must be atomic.
 * @block_txq_ptrs: stop updating the write pointers of the Tx queues. Note
 *	that the transport needs to refcount the calls since this function
 *	will be called several times with block = true, and then the queues
 *	need to be unblocked only after the same number of calls with
 *	block = false.
 * @write8: write a u8 to a register at offset ofs from the BAR
 * @write32: write a u32 to a register at offset ofs from the BAR
 * @read32: read a u32 register at offset ofs from the BAR
 * @read_prph: read a DWORD from a periphery register
 * @write_prph: write a DWORD to a periphery register
 * @read_mem: read device's SRAM in DWORD
 * @write_mem: write device's SRAM in DWORD. If %buf is %NULL, then the memory
 *	will be zeroed.
 * @read_config32: read a u32 value from the device's config space at
 *	the given offset.
 * @configure: configure parameters required by the transport layer from
 *	the op_mode. May be called several times before start_fw, can't be
 *	called after that.
 * @set_pmi: set the power pmi state
 * @grab_nic_access: wake the NIC to be able to access non-HBUS regs.
 *	Sleeping is not allowed between grab_nic_access and
 *	release_nic_access.
 * @release_nic_access: let the NIC go to sleep. The "flags" parameter
 *	must be the same one that was sent before to the grab_nic_access.
 * @set_bits_mask - set SRAM register according to value and mask.
 * @dump_data: return a vmalloc'ed buffer with debug data, maybe containing last
 *	TX'ed commands and similar. The buffer will be vfree'd by the caller.
 *	Note that the transport must fill in the proper file headers.
 * @debugfs_cleanup: used in the driver unload flow to make a proper cleanup
 *	of the trans debugfs
 * @set_pnvm: set the pnvm data in the prph scratch buffer, inside the
 *	context info.
 * @interrupts: disable/enable interrupts to transport
 */
struct iwl_trans_ops {

	int (*start_hw)(struct iwl_trans *iwl_trans);
	void (*op_mode_leave)(struct iwl_trans *iwl_trans);
	int (*start_fw)(struct iwl_trans *trans, const struct fw_img *fw,
			bool run_in_rfkill);
	void (*fw_alive)(struct iwl_trans *trans, u32 scd_addr);
	void (*stop_device)(struct iwl_trans *trans);

	int (*d3_suspend)(struct iwl_trans *trans, bool test, bool reset);
	int (*d3_resume)(struct iwl_trans *trans, enum iwl_d3_status *status,
			 bool test, bool reset);

	int (*send_cmd)(struct iwl_trans *trans, struct iwl_host_cmd *cmd);

	int (*tx)(struct iwl_trans *trans, struct sk_buff *skb,
		  struct iwl_device_tx_cmd *dev_cmd, int queue);
	void (*reclaim)(struct iwl_trans *trans, int queue, int ssn,
			struct sk_buff_head *skbs);

	void (*set_q_ptrs)(struct iwl_trans *trans, int queue, int ptr);

	bool (*txq_enable)(struct iwl_trans *trans, int queue, u16 ssn,
			   const struct iwl_trans_txq_scd_cfg *cfg,
			   unsigned int queue_wdg_timeout);
	void (*txq_disable)(struct iwl_trans *trans, int queue,
			    bool configure_scd);
	/* 22000 functions */
	int (*txq_alloc)(struct iwl_trans *trans,
			 __le16 flags, u8 sta_id, u8 tid,
			 int cmd_id, int size,
			 unsigned int queue_wdg_timeout);
	void (*txq_free)(struct iwl_trans *trans, int queue);
	int (*rxq_dma_data)(struct iwl_trans *trans, int queue,
			    struct iwl_trans_rxq_dma_data *data);

	void (*txq_set_shared_mode)(struct iwl_trans *trans, u32 txq_id,
				    bool shared);

	int (*wait_tx_queues_empty)(struct iwl_trans *trans, u32 txq_bm);
	int (*wait_txq_empty)(struct iwl_trans *trans, int queue);
	void (*freeze_txq_timer)(struct iwl_trans *trans, unsigned long txqs,
				 bool freeze);
	void (*block_txq_ptrs)(struct iwl_trans *trans, bool block);

	void (*write8)(struct iwl_trans *trans, u32 ofs, u8 val);
	void (*write32)(struct iwl_trans *trans, u32 ofs, u32 val);
	u32 (*read32)(struct iwl_trans *trans, u32 ofs);
	u32 (*read_prph)(struct iwl_trans *trans, u32 ofs);
	void (*write_prph)(struct iwl_trans *trans, u32 ofs, u32 val);
	int (*read_mem)(struct iwl_trans *trans, u32 addr,
			void *buf, int dwords);
	int (*write_mem)(struct iwl_trans *trans, u32 addr,
			 const void *buf, int dwords);
	int (*read_config32)(struct iwl_trans *trans, u32 ofs, u32 *val);
	void (*configure)(struct iwl_trans *trans,
			  const struct iwl_trans_config *trans_cfg);
	void (*set_pmi)(struct iwl_trans *trans, bool state);
	void (*sw_reset)(struct iwl_trans *trans);
	bool (*grab_nic_access)(struct iwl_trans *trans);
	void (*release_nic_access)(struct iwl_trans *trans);
	void (*set_bits_mask)(struct iwl_trans *trans, u32 reg, u32 mask,
			      u32 value);

	struct iwl_trans_dump_data *(*dump_data)(struct iwl_trans *trans,
						 u32 dump_mask);
	void (*debugfs_cleanup)(struct iwl_trans *trans);
	void (*sync_nmi)(struct iwl_trans *trans);
	int (*set_pnvm)(struct iwl_trans *trans, const void *data, u32 len);
	int (*set_reduce_power)(struct iwl_trans *trans,
				const void *data, u32 len);
	void (*interrupts)(struct iwl_trans *trans, bool enable);
};

/**
 * enum iwl_trans_state - state of the transport layer
 *
 * @IWL_TRANS_NO_FW: firmware wasn't started yet, or crashed
 * @IWL_TRANS_FW_STARTED: FW was started, but not alive yet
 * @IWL_TRANS_FW_ALIVE: FW has sent an alive response
 */
enum iwl_trans_state {
	IWL_TRANS_NO_FW,
	IWL_TRANS_FW_STARTED,
	IWL_TRANS_FW_ALIVE,
};

/**
 * DOC: Platform power management
 *
 * In system-wide power management the entire platform goes into a low
 * power state (e.g. idle or suspend to RAM) at the same time and the
 * device is configured as a wakeup source for the entire platform.
 * This is usually triggered by userspace activity (e.g. the user
 * presses the suspend button or a power management daemon decides to
 * put the platform in low power mode).  The device's behavior in this
 * mode is dictated by the wake-on-WLAN configuration.
 *
 * The terms used for the device's behavior are as follows:
 *
 *	- D0: the device is fully powered and the host is awake;
 *	- D3: the device is in low power mode and only reacts to
 *		specific events (e.g. magic-packet received or scan
 *		results found);
 *
 * These terms reflect the power modes in the firmware and are not to
 * be confused with the physical device power state.
 */

/**
 * enum iwl_plat_pm_mode - platform power management mode
 *
 * This enumeration describes the device's platform power management
 * behavior when in system-wide suspend (i.e WoWLAN).
 *
 * @IWL_PLAT_PM_MODE_DISABLED: power management is disabled for this
 *	device.  In system-wide suspend mode, it means that the all
 *	connections will be closed automatically by mac80211 before
 *	the platform is suspended.
 * @IWL_PLAT_PM_MODE_D3: the device goes into D3 mode (i.e. WoWLAN).
 */
enum iwl_plat_pm_mode {
	IWL_PLAT_PM_MODE_DISABLED,
	IWL_PLAT_PM_MODE_D3,
};

/**
 * enum iwl_ini_cfg_state
 * @IWL_INI_CFG_STATE_NOT_LOADED: no debug cfg was given
 * @IWL_INI_CFG_STATE_LOADED: debug cfg was found and loaded
 * @IWL_INI_CFG_STATE_CORRUPTED: debug cfg was found and some of the TLVs
 *	are corrupted. The rest of the debug TLVs will still be used
 */
enum iwl_ini_cfg_state {
	IWL_INI_CFG_STATE_NOT_LOADED,
	IWL_INI_CFG_STATE_LOADED,
	IWL_INI_CFG_STATE_CORRUPTED,
};

/* Max time to wait for nmi interrupt */
#define IWL_TRANS_NMI_TIMEOUT (HZ / 4)

/**
 * struct iwl_dram_data
 * @physical: page phy pointer
 * @block: pointer to the allocated block/page
 * @size: size of the block/page
 */
struct iwl_dram_data {
	dma_addr_t physical;
	void *block;
	int size;
};

/**
 * struct iwl_fw_mon - fw monitor per allocation id
 * @num_frags: number of fragments
 * @frags: an array of DRAM buffer fragments
 */
struct iwl_fw_mon {
	u32 num_frags;
	struct iwl_dram_data *frags;
};

/**
 * struct iwl_self_init_dram - dram data used by self init process
 * @fw: lmac and umac dram data
 * @fw_cnt: total number of items in array
 * @paging: paging dram data
 * @paging_cnt: total number of items in array
 */
struct iwl_self_init_dram {
	struct iwl_dram_data *fw;
	int fw_cnt;
	struct iwl_dram_data *paging;
	int paging_cnt;
};

/**
 * struct iwl_trans_debug - transport debug related data
 *
 * @n_dest_reg: num of reg_ops in %dbg_dest_tlv
 * @rec_on: true iff there is a fw debug recording currently active
 * @dest_tlv: points to the destination TLV for debug
 * @conf_tlv: array of pointers to configuration TLVs for debug
 * @trigger_tlv: array of pointers to triggers TLVs for debug
 * @lmac_error_event_table: addrs of lmacs error tables
 * @umac_error_event_table: addr of umac error table
 * @tcm_error_event_table: address of TCM error table
 * @error_event_table_tlv_status: bitmap that indicates what error table
 *	pointers was recevied via TLV. uses enum &iwl_error_event_table_status
 * @internal_ini_cfg: internal debug cfg state. Uses &enum iwl_ini_cfg_state
 * @external_ini_cfg: external debug cfg state. Uses &enum iwl_ini_cfg_state
 * @fw_mon_cfg: debug buffer allocation configuration
 * @fw_mon_ini: DRAM buffer fragments per allocation id
 * @fw_mon: DRAM buffer for firmware monitor
 * @hw_error: equals true if hw error interrupt was received from the FW
 * @ini_dest: debug monitor destination uses &enum iwl_fw_ini_buffer_location
 * @active_regions: active regions
 * @debug_info_tlv_list: list of debug info TLVs
 * @time_point: array of debug time points
 * @periodic_trig_list: periodic triggers list
 * @domains_bitmap: bitmap of active domains other than
 *	&IWL_FW_INI_DOMAIN_ALWAYS_ON
 */
struct iwl_trans_debug {
	u8 n_dest_reg;
	bool rec_on;

	const struct iwl_fw_dbg_dest_tlv_v1 *dest_tlv;
	const struct iwl_fw_dbg_conf_tlv *conf_tlv[FW_DBG_CONF_MAX];
	struct iwl_fw_dbg_trigger_tlv * const *trigger_tlv;

	u32 lmac_error_event_table[2];
	u32 umac_error_event_table;
	u32 tcm_error_event_table;
	unsigned int error_event_table_tlv_status;

	enum iwl_ini_cfg_state internal_ini_cfg;
	enum iwl_ini_cfg_state external_ini_cfg;

	struct iwl_fw_ini_allocation_tlv fw_mon_cfg[IWL_FW_INI_ALLOCATION_NUM];
	struct iwl_fw_mon fw_mon_ini[IWL_FW_INI_ALLOCATION_NUM];

	struct iwl_dram_data fw_mon;

	bool hw_error;
	enum iwl_fw_ini_buffer_location ini_dest;

	u64 unsupported_region_msk;
	struct iwl_ucode_tlv *active_regions[IWL_FW_INI_MAX_REGION_ID];
	struct list_head debug_info_tlv_list;
	struct iwl_dbg_tlv_time_point_data
		time_point[IWL_FW_INI_TIME_POINT_NUM];
	struct list_head periodic_trig_list;

	u32 domains_bitmap;
};

struct iwl_dma_ptr {
	dma_addr_t dma;
	void *addr;
	size_t size;
};

struct iwl_cmd_meta {
	/* only for SYNC commands, iff the reply skb is wanted */
	struct iwl_host_cmd *source;
	u32 flags;
	u32 tbs;
};

/*
 * The FH will write back to the first TB only, so we need to copy some data
 * into the buffer regardless of whether it should be mapped or not.
 * This indicates how big the first TB must be to include the scratch buffer
 * and the assigned PN.
 * Since PN location is 8 bytes at offset 12, it's 20 now.
 * If we make it bigger then allocations will be bigger and copy slower, so
 * that's probably not useful.
 */
#define IWL_FIRST_TB_SIZE	20
#define IWL_FIRST_TB_SIZE_ALIGN ALIGN(IWL_FIRST_TB_SIZE, 64)

struct iwl_pcie_txq_entry {
	void *cmd;
	struct sk_buff *skb;
	/* buffer to free after command completes */
	const void *free_buf;
	struct iwl_cmd_meta meta;
};

struct iwl_pcie_first_tb_buf {
	u8 buf[IWL_FIRST_TB_SIZE_ALIGN];
};

/**
 * struct iwl_txq - Tx Queue for DMA
 * @q: generic Rx/Tx queue descriptor
 * @tfds: transmit frame descriptors (DMA memory)
 * @first_tb_bufs: start of command headers, including scratch buffers, for
 *	the writeback -- this is DMA memory and an array holding one buffer
 *	for each command on the queue
 * @first_tb_dma: DMA address for the first_tb_bufs start
 * @entries: transmit entries (driver state)
 * @lock: queue lock
 * @stuck_timer: timer that fires if queue gets stuck
 * @trans: pointer back to transport (for timer)
 * @need_update: indicates need to update read/write index
 * @ampdu: true if this queue is an ampdu queue for an specific RA/TID
 * @wd_timeout: queue watchdog timeout (jiffies) - per queue
 * @frozen: tx stuck queue timer is frozen
 * @frozen_expiry_remainder: remember how long until the timer fires
 * @bc_tbl: byte count table of the queue (relevant only for gen2 transport)
 * @write_ptr: 1-st empty entry (index) host_w
 * @read_ptr: last used entry (index) host_r
 * @dma_addr:  physical addr for BD's
 * @n_window: safe queue window
 * @id: queue id
 * @low_mark: low watermark, resume queue if free space more than this
 * @high_mark: high watermark, stop queue if free space less than this
 *
 * A Tx queue consists of circular buffer of BDs (a.k.a. TFDs, transmit frame
 * descriptors) and required locking structures.
 *
 * Note the difference between TFD_QUEUE_SIZE_MAX and n_window: the hardware
 * always assumes 256 descriptors, so TFD_QUEUE_SIZE_MAX is always 256 (unless
 * there might be HW changes in the future). For the normal TX
 * queues, n_window, which is the size of the software queue data
 * is also 256; however, for the command queue, n_window is only
 * 32 since we don't need so many commands pending. Since the HW
 * still uses 256 BDs for DMA though, TFD_QUEUE_SIZE_MAX stays 256.
 * This means that we end up with the following:
 *  HW entries: | 0 | ... | N * 32 | ... | N * 32 + 31 | ... | 255 |
 *  SW entries:           | 0      | ... | 31          |
 * where N is a number between 0 and 7. This means that the SW
 * data is a window overlayed over the HW queue.
 */
struct iwl_txq {
	void *tfds;
	struct iwl_pcie_first_tb_buf *first_tb_bufs;
	dma_addr_t first_tb_dma;
	struct iwl_pcie_txq_entry *entries;
	/* lock for syncing changes on the queue */
	spinlock_t lock;
	unsigned long frozen_expiry_remainder;
	struct timer_list stuck_timer;
	struct iwl_trans *trans;
	bool need_update;
	bool frozen;
	bool ampdu;
	int block;
	unsigned long wd_timeout;
	struct sk_buff_head overflow_q;
	struct iwl_dma_ptr bc_tbl;

	int write_ptr;
	int read_ptr;
	dma_addr_t dma_addr;
	int n_window;
	u32 id;
	int low_mark;
	int high_mark;

	bool overflow_tx;
};

/**
 * struct iwl_trans_txqs - transport tx queues data
 *
 * @bc_table_dword: true if the BC table expects DWORD (as opposed to bytes)
 * @page_offs: offset from skb->cb to mac header page pointer
 * @dev_cmd_offs: offset from skb->cb to iwl_device_tx_cmd pointer
 * @queue_used - bit mask of used queues
 * @queue_stopped - bit mask of stopped queues
 * @scd_bc_tbls: gen1 pointer to the byte count table of the scheduler
 */
struct iwl_trans_txqs {
	unsigned long queue_used[BITS_TO_LONGS(IWL_MAX_TVQM_QUEUES)];
	unsigned long queue_stopped[BITS_TO_LONGS(IWL_MAX_TVQM_QUEUES)];
	struct iwl_txq *txq[IWL_MAX_TVQM_QUEUES];
	struct dma_pool *bc_pool;
	size_t bc_tbl_size;
	bool bc_table_dword;
	u8 page_offs;
	u8 dev_cmd_offs;
	struct iwl_tso_hdr_page __percpu *tso_hdr_page;

	struct {
		u8 fifo;
		u8 q_id;
		unsigned int wdg_timeout;
	} cmd;

	struct {
		u8 max_tbs;
		u16 size;
		u8 addr_size;
	} tfd;

	struct iwl_dma_ptr scd_bc_tbls;
};

/**
 * struct iwl_trans - transport common data
 *
 * @ops - pointer to iwl_trans_ops
 * @op_mode - pointer to the op_mode
 * @trans_cfg: the trans-specific configuration part
 * @cfg - pointer to the configuration
 * @drv - pointer to iwl_drv
 * @status: a bit-mask of transport status flags
 * @dev - pointer to struct device * that represents the device
 * @max_skb_frags: maximum number of fragments an SKB can have when transmitted.
 *	0 indicates that frag SKBs (NETIF_F_SG) aren't supported.
 * @hw_rf_id a u32 with the device RF ID
 * @hw_id: a u32 with the ID of the device / sub-device.
 *	Set during transport allocation.
 * @hw_id_str: a string with info about HW ID. Set during transport allocation.
 * @pm_support: set to true in start_hw if link pm is supported
 * @ltr_enabled: set to true if the LTR is enabled
 * @wide_cmd_header: true when ucode supports wide command header format
 * @wait_command_queue: wait queue for sync commands
 * @num_rx_queues: number of RX queues allocated by the transport;
 *	the transport must set this before calling iwl_drv_start()
 * @iml_len: the length of the image loader
 * @iml: a pointer to the image loader itself
 * @dev_cmd_pool: pool for Tx cmd allocation - for internal use only.
 *	The user should use iwl_trans_{alloc,free}_tx_cmd.
 * @rx_mpdu_cmd: MPDU RX command ID, must be assigned by opmode before
 *	starting the firmware, used for tracing
 * @rx_mpdu_cmd_hdr_size: used for tracing, amount of data before the
 *	start of the 802.11 header in the @rx_mpdu_cmd
 * @dflt_pwr_limit: default power limit fetched from the platform (ACPI)
 * @system_pm_mode: the system-wide power management mode in use.
 *	This mode is set dynamically, depending on the WoWLAN values
 *	configured from the userspace at runtime.
 * @iwl_trans_txqs: transport tx queues data.
 */
struct iwl_trans {
	const struct iwl_trans_ops *ops;
	struct iwl_op_mode *op_mode;
	const struct iwl_cfg_trans_params *trans_cfg;
	const struct iwl_cfg *cfg;
	struct iwl_drv *drv;
	enum iwl_trans_state state;
	unsigned long status;

	struct device *dev;
	u32 max_skb_frags;
	u32 hw_rev;
	u32 hw_rf_id;
	u32 hw_id;
	char hw_id_str[52];
	u32 sku_id[3];

	u8 rx_mpdu_cmd, rx_mpdu_cmd_hdr_size;

	bool pm_support;
	bool ltr_enabled;
	u8 pnvm_loaded:1;
	u8 reduce_power_loaded:1;

	const struct iwl_hcmd_arr *command_groups;
	int command_groups_size;
	bool wide_cmd_header;

	wait_queue_head_t wait_command_queue;
	u8 num_rx_queues;

	size_t iml_len;
	u8 *iml;

	/* The following fields are internal only */
	struct kmem_cache *dev_cmd_pool;
	char dev_cmd_pool_name[50];

	struct dentry *dbgfs_dir;

#ifdef CONFIG_LOCKDEP
	struct lockdep_map sync_cmd_lockdep_map;
#endif

	struct iwl_trans_debug dbg;
	struct iwl_self_init_dram init_dram;

	enum iwl_plat_pm_mode system_pm_mode;

	const char *name;
	struct iwl_trans_txqs txqs;

	/* pointer to trans specific struct */
	/*Ensure that this pointer will always be aligned to sizeof pointer */
	char trans_specific[] __aligned(sizeof(void *));
};

const char *iwl_get_cmd_string(struct iwl_trans *trans, u32 id);
int iwl_cmd_groups_verify_sorted(const struct iwl_trans_config *trans);

static inline void iwl_trans_configure(struct iwl_trans *trans,
				       const struct iwl_trans_config *trans_cfg)
{
	trans->op_mode = trans_cfg->op_mode;

	trans->ops->configure(trans, trans_cfg);
	WARN_ON(iwl_cmd_groups_verify_sorted(trans_cfg));
}

static inline int iwl_trans_start_hw(struct iwl_trans *trans)
{
	might_sleep();

	return trans->ops->start_hw(trans);
}

static inline void iwl_trans_op_mode_leave(struct iwl_trans *trans)
{
	might_sleep();

	if (trans->ops->op_mode_leave)
		trans->ops->op_mode_leave(trans);

	trans->op_mode = NULL;

	trans->state = IWL_TRANS_NO_FW;
}

static inline void iwl_trans_fw_alive(struct iwl_trans *trans, u32 scd_addr)
{
	might_sleep();

	trans->state = IWL_TRANS_FW_ALIVE;

	trans->ops->fw_alive(trans, scd_addr);
}

static inline int iwl_trans_start_fw(struct iwl_trans *trans,
				     const struct fw_img *fw,
				     bool run_in_rfkill)
{
	int ret;

	might_sleep();

	WARN_ON_ONCE(!trans->rx_mpdu_cmd);

	clear_bit(STATUS_FW_ERROR, &trans->status);
	ret = trans->ops->start_fw(trans, fw, run_in_rfkill);
	if (ret == 0)
		trans->state = IWL_TRANS_FW_STARTED;

	return ret;
}

static inline void iwl_trans_stop_device(struct iwl_trans *trans)
{
	might_sleep();

	trans->ops->stop_device(trans);

	trans->state = IWL_TRANS_NO_FW;
}

static inline int iwl_trans_d3_suspend(struct iwl_trans *trans, bool test,
				       bool reset)
{
	might_sleep();
	if (!trans->ops->d3_suspend)
		return 0;

	return trans->ops->d3_suspend(trans, test, reset);
}

static inline int iwl_trans_d3_resume(struct iwl_trans *trans,
				      enum iwl_d3_status *status,
				      bool test, bool reset)
{
	might_sleep();
	if (!trans->ops->d3_resume)
		return 0;

	return trans->ops->d3_resume(trans, status, test, reset);
}

static inline struct iwl_trans_dump_data *
iwl_trans_dump_data(struct iwl_trans *trans, u32 dump_mask)
{
	if (!trans->ops->dump_data)
		return NULL;
	return trans->ops->dump_data(trans, dump_mask);
}

static inline struct iwl_device_tx_cmd *
iwl_trans_alloc_tx_cmd(struct iwl_trans *trans)
{
	return kmem_cache_zalloc(trans->dev_cmd_pool, GFP_ATOMIC);
}

int iwl_trans_send_cmd(struct iwl_trans *trans, struct iwl_host_cmd *cmd);

static inline void iwl_trans_free_tx_cmd(struct iwl_trans *trans,
					 struct iwl_device_tx_cmd *dev_cmd)
{
	kmem_cache_free(trans->dev_cmd_pool, dev_cmd);
}

static inline int iwl_trans_tx(struct iwl_trans *trans, struct sk_buff *skb,
			       struct iwl_device_tx_cmd *dev_cmd, int queue)
{
	if (unlikely(test_bit(STATUS_FW_ERROR, &trans->status)))
		return -EIO;

	if (WARN_ON_ONCE(trans->state != IWL_TRANS_FW_ALIVE)) {
		IWL_ERR(trans, "%s bad state = %d\n", __func__, trans->state);
		return -EIO;
	}

	return trans->ops->tx(trans, skb, dev_cmd, queue);
}

static inline void iwl_trans_reclaim(struct iwl_trans *trans, int queue,
				     int ssn, struct sk_buff_head *skbs)
{
	if (WARN_ON_ONCE(trans->state != IWL_TRANS_FW_ALIVE)) {
		IWL_ERR(trans, "%s bad state = %d\n", __func__, trans->state);
		return;
	}

	trans->ops->reclaim(trans, queue, ssn, skbs);
}

static inline void iwl_trans_set_q_ptrs(struct iwl_trans *trans, int queue,
					int ptr)
{
	if (WARN_ON_ONCE(trans->state != IWL_TRANS_FW_ALIVE)) {
		IWL_ERR(trans, "%s bad state = %d\n", __func__, trans->state);
		return;
	}

	trans->ops->set_q_ptrs(trans, queue, ptr);
}

static inline void iwl_trans_txq_disable(struct iwl_trans *trans, int queue,
					 bool configure_scd)
{
	trans->ops->txq_disable(trans, queue, configure_scd);
}

static inline bool
iwl_trans_txq_enable_cfg(struct iwl_trans *trans, int queue, u16 ssn,
			 const struct iwl_trans_txq_scd_cfg *cfg,
			 unsigned int queue_wdg_timeout)
{
	might_sleep();

	if (WARN_ON_ONCE(trans->state != IWL_TRANS_FW_ALIVE)) {
		IWL_ERR(trans, "%s bad state = %d\n", __func__, trans->state);
		return false;
	}

	return trans->ops->txq_enable(trans, queue, ssn,
				      cfg, queue_wdg_timeout);
}

static inline int
iwl_trans_get_rxq_dma_data(struct iwl_trans *trans, int queue,
			   struct iwl_trans_rxq_dma_data *data)
{
	if (WARN_ON_ONCE(!trans->ops->rxq_dma_data))
		return -ENOTSUPP;

	return trans->ops->rxq_dma_data(trans, queue, data);
}

static inline void
iwl_trans_txq_free(struct iwl_trans *trans, int queue)
{
	if (WARN_ON_ONCE(!trans->ops->txq_free))
		return;

	trans->ops->txq_free(trans, queue);
}

static inline int
iwl_trans_txq_alloc(struct iwl_trans *trans,
		    __le16 flags, u8 sta_id, u8 tid,
		    int cmd_id, int size,
		    unsigned int wdg_timeout)
{
	might_sleep();

	if (WARN_ON_ONCE(!trans->ops->txq_alloc))
		return -ENOTSUPP;

	if (WARN_ON_ONCE(trans->state != IWL_TRANS_FW_ALIVE)) {
		IWL_ERR(trans, "%s bad state = %d\n", __func__, trans->state);
		return -EIO;
	}

	return trans->ops->txq_alloc(trans, flags, sta_id, tid,
				     cmd_id, size, wdg_timeout);
}

static inline void iwl_trans_txq_set_shared_mode(struct iwl_trans *trans,
						 int queue, bool shared_mode)
{
	if (trans->ops->txq_set_shared_mode)
		trans->ops->txq_set_shared_mode(trans, queue, shared_mode);
}

static inline void iwl_trans_txq_enable(struct iwl_trans *trans, int queue,
					int fifo, int sta_id, int tid,
					int frame_limit, u16 ssn,
					unsigned int queue_wdg_timeout)
{
	struct iwl_trans_txq_scd_cfg cfg = {
		.fifo = fifo,
		.sta_id = sta_id,
		.tid = tid,
		.frame_limit = frame_limit,
		.aggregate = sta_id >= 0,
	};

	iwl_trans_txq_enable_cfg(trans, queue, ssn, &cfg, queue_wdg_timeout);
}

static inline
void iwl_trans_ac_txq_enable(struct iwl_trans *trans, int queue, int fifo,
			     unsigned int queue_wdg_timeout)
{
	struct iwl_trans_txq_scd_cfg cfg = {
		.fifo = fifo,
		.sta_id = -1,
		.tid = IWL_MAX_TID_COUNT,
		.frame_limit = IWL_FRAME_LIMIT,
		.aggregate = false,
	};

	iwl_trans_txq_enable_cfg(trans, queue, 0, &cfg, queue_wdg_timeout);
}

static inline void iwl_trans_freeze_txq_timer(struct iwl_trans *trans,
					      unsigned long txqs,
					      bool freeze)
{
	if (WARN_ON_ONCE(trans->state != IWL_TRANS_FW_ALIVE)) {
		IWL_ERR(trans, "%s bad state = %d\n", __func__, trans->state);
		return;
	}

	if (trans->ops->freeze_txq_timer)
		trans->ops->freeze_txq_timer(trans, txqs, freeze);
}

static inline void iwl_trans_block_txq_ptrs(struct iwl_trans *trans,
					    bool block)
{
	if (WARN_ON_ONCE(trans->state != IWL_TRANS_FW_ALIVE)) {
		IWL_ERR(trans, "%s bad state = %d\n", __func__, trans->state);
		return;
	}

	if (trans->ops->block_txq_ptrs)
		trans->ops->block_txq_ptrs(trans, block);
}

static inline int iwl_trans_wait_tx_queues_empty(struct iwl_trans *trans,
						 u32 txqs)
{
	if (WARN_ON_ONCE(!trans->ops->wait_tx_queues_empty))
		return -ENOTSUPP;

	/* No need to wait if the firmware is not alive */
	if (trans->state != IWL_TRANS_FW_ALIVE) {
		IWL_ERR(trans, "%s bad state = %d\n", __func__, trans->state);
		return -EIO;
	}

	return trans->ops->wait_tx_queues_empty(trans, txqs);
}

static inline int iwl_trans_wait_txq_empty(struct iwl_trans *trans, int queue)
{
	if (WARN_ON_ONCE(!trans->ops->wait_txq_empty))
		return -ENOTSUPP;

	if (WARN_ON_ONCE(trans->state != IWL_TRANS_FW_ALIVE)) {
		IWL_ERR(trans, "%s bad state = %d\n", __func__, trans->state);
		return -EIO;
	}

	return trans->ops->wait_txq_empty(trans, queue);
}

static inline void iwl_trans_write8(struct iwl_trans *trans, u32 ofs, u8 val)
{
	trans->ops->write8(trans, ofs, val);
}

static inline void iwl_trans_write32(struct iwl_trans *trans, u32 ofs, u32 val)
{
	trans->ops->write32(trans, ofs, val);
}

static inline u32 iwl_trans_read32(struct iwl_trans *trans, u32 ofs)
{
	return trans->ops->read32(trans, ofs);
}

static inline u32 iwl_trans_read_prph(struct iwl_trans *trans, u32 ofs)
{
	return trans->ops->read_prph(trans, ofs);
}

static inline void iwl_trans_write_prph(struct iwl_trans *trans, u32 ofs,
					u32 val)
{
	return trans->ops->write_prph(trans, ofs, val);
}

static inline int iwl_trans_read_mem(struct iwl_trans *trans, u32 addr,
				     void *buf, int dwords)
{
	return trans->ops->read_mem(trans, addr, buf, dwords);
}

#define iwl_trans_read_mem_bytes(trans, addr, buf, bufsize)		      \
	do {								      \
		if (__builtin_constant_p(bufsize))			      \
			BUILD_BUG_ON((bufsize) % sizeof(u32));		      \
		iwl_trans_read_mem(trans, addr, buf, (bufsize) / sizeof(u32));\
	} while (0)

static inline u32 iwl_trans_read_mem32(struct iwl_trans *trans, u32 addr)
{
	u32 value;

	if (WARN_ON(iwl_trans_read_mem(trans, addr, &value, 1)))
		return 0xa5a5a5a5;

	return value;
}

static inline int iwl_trans_write_mem(struct iwl_trans *trans, u32 addr,
				      const void *buf, int dwords)
{
	return trans->ops->write_mem(trans, addr, buf, dwords);
}

static inline u32 iwl_trans_write_mem32(struct iwl_trans *trans, u32 addr,
					u32 val)
{
	return iwl_trans_write_mem(trans, addr, &val, 1);
}

static inline void iwl_trans_set_pmi(struct iwl_trans *trans, bool state)
{
	if (trans->ops->set_pmi)
		trans->ops->set_pmi(trans, state);
}

static inline void iwl_trans_sw_reset(struct iwl_trans *trans)
{
	if (trans->ops->sw_reset)
		trans->ops->sw_reset(trans);
}

static inline void
iwl_trans_set_bits_mask(struct iwl_trans *trans, u32 reg, u32 mask, u32 value)
{
	trans->ops->set_bits_mask(trans, reg, mask, value);
}

#define iwl_trans_grab_nic_access(trans)		\
	__cond_lock(nic_access,				\
		    likely((trans)->ops->grab_nic_access(trans)))

static inline void __releases(nic_access)
iwl_trans_release_nic_access(struct iwl_trans *trans)
{
	trans->ops->release_nic_access(trans);
	__release(nic_access);
}

static inline void iwl_trans_fw_error(struct iwl_trans *trans, bool sync)
{
	if (WARN_ON_ONCE(!trans->op_mode))
		return;

	/* prevent double restarts due to the same erroneous FW */
	if (!test_and_set_bit(STATUS_FW_ERROR, &trans->status)) {
		iwl_op_mode_nic_error(trans->op_mode, sync);
		trans->state = IWL_TRANS_NO_FW;
	}
}

static inline bool iwl_trans_fw_running(struct iwl_trans *trans)
{
	return trans->state == IWL_TRANS_FW_ALIVE;
}

static inline void iwl_trans_sync_nmi(struct iwl_trans *trans)
{
	if (trans->ops->sync_nmi)
		trans->ops->sync_nmi(trans);
}

void iwl_trans_sync_nmi_with_addr(struct iwl_trans *trans, u32 inta_addr,
				  u32 sw_err_bit);

static inline int iwl_trans_set_pnvm(struct iwl_trans *trans,
				     const void *data, u32 len)
{
	if (trans->ops->set_pnvm) {
		int ret = trans->ops->set_pnvm(trans, data, len);

		if (ret)
			return ret;
	}

	trans->pnvm_loaded = true;

	return 0;
}

static inline int iwl_trans_set_reduce_power(struct iwl_trans *trans,
					     const void *data, u32 len)
{
	if (trans->ops->set_reduce_power) {
		int ret = trans->ops->set_reduce_power(trans, data, len);

		if (ret)
			return ret;
	}

	trans->reduce_power_loaded = true;
	return 0;
}

static inline bool iwl_trans_dbg_ini_valid(struct iwl_trans *trans)
{
	return trans->dbg.internal_ini_cfg != IWL_INI_CFG_STATE_NOT_LOADED ||
		trans->dbg.external_ini_cfg != IWL_INI_CFG_STATE_NOT_LOADED;
}

static inline void iwl_trans_interrupts(struct iwl_trans *trans, bool enable)
{
	if (trans->ops->interrupts)
		trans->ops->interrupts(trans, enable);
}

/*****************************************************
 * transport helper functions
 *****************************************************/
struct iwl_trans *iwl_trans_alloc(unsigned int priv_size,
			  struct device *dev,
			  const struct iwl_trans_ops *ops,
			  const struct iwl_cfg_trans_params *cfg_trans);
int iwl_trans_init(struct iwl_trans *trans);
void iwl_trans_free(struct iwl_trans *trans);

/*****************************************************
* driver (transport) register/unregister functions
******************************************************/
int __must_check iwl_pci_register_driver(void);
void iwl_pci_unregister_driver(void);

#endif /* __iwl_trans_h__ */
struct iwl_trans_ops {

	int (*start_hw)(struct iwl_trans *iwl_trans);
	void (*op_mode_leave)(struct iwl_trans *iwl_trans);
	int (*start_fw)(struct iwl_trans *trans, const struct fw_img *fw,
			bool run_in_rfkill);
	void (*fw_alive)(struct iwl_trans *trans, u32 scd_addr);
	void (*stop_device)(struct iwl_trans *trans);

	int (*d3_suspend)(struct iwl_trans *trans, bool test, bool reset);
	int (*d3_resume)(struct iwl_trans *trans, enum iwl_d3_status *status,
			 bool test, bool reset);

	int (*send_cmd)(struct iwl_trans *trans, struct iwl_host_cmd *cmd);

	int (*tx)(struct iwl_trans *trans, struct sk_buff *skb,
		  struct iwl_device_tx_cmd *dev_cmd, int queue);
	void (*reclaim)(struct iwl_trans *trans, int queue, int ssn,
			struct sk_buff_head *skbs);

	void (*set_q_ptrs)(struct iwl_trans *trans, int queue, int ptr);

	bool (*txq_enable)(struct iwl_trans *trans, int queue, u16 ssn,
			   const struct iwl_trans_txq_scd_cfg *cfg,
			   unsigned int queue_wdg_timeout);
	void (*txq_disable)(struct iwl_trans *trans, int queue,
			    bool configure_scd);
	/* 22000 functions */
	int (*txq_alloc)(struct iwl_trans *trans,
			 __le16 flags, u8 sta_id, u8 tid,
			 int cmd_id, int size,
			 unsigned int queue_wdg_timeout);
	void (*txq_free)(struct iwl_trans *trans, int queue);
	int (*rxq_dma_data)(struct iwl_trans *trans, int queue,
			    struct iwl_trans_rxq_dma_data *data);

	void (*txq_set_shared_mode)(struct iwl_trans *trans, u32 txq_id,
				    bool shared);

	int (*wait_tx_queues_empty)(struct iwl_trans *trans, u32 txq_bm);
	int (*wait_txq_empty)(struct iwl_trans *trans, int queue);
	void (*freeze_txq_timer)(struct iwl_trans *trans, unsigned long txqs,
				 bool freeze);
	void (*block_txq_ptrs)(struct iwl_trans *trans, bool block);

	void (*write8)(struct iwl_trans *trans, u32 ofs, u8 val);
	void (*write32)(struct iwl_trans *trans, u32 ofs, u32 val);
	u32 (*read32)(struct iwl_trans *trans, u32 ofs);
	u32 (*read_prph)(struct iwl_trans *trans, u32 ofs);
	void (*write_prph)(struct iwl_trans *trans, u32 ofs, u32 val);
	int (*read_mem)(struct iwl_trans *trans, u32 addr,
			void *buf, int dwords);
	int (*write_mem)(struct iwl_trans *trans, u32 addr,
			 const void *buf, int dwords);
	int (*read_config32)(struct iwl_trans *trans, u32 ofs, u32 *val);
	void (*configure)(struct iwl_trans *trans,
			  const struct iwl_trans_config *trans_cfg);
	void (*set_pmi)(struct iwl_trans *trans, bool state);
	void (*sw_reset)(struct iwl_trans *trans);
	bool (*grab_nic_access)(struct iwl_trans *trans);
	void (*release_nic_access)(struct iwl_trans *trans);
	void (*set_bits_mask)(struct iwl_trans *trans, u32 reg, u32 mask,
			      u32 value);

	struct iwl_trans_dump_data *(*dump_data)(struct iwl_trans *trans,
						 u32 dump_mask);
	void (*debugfs_cleanup)(struct iwl_trans *trans);
	void (*sync_nmi)(struct iwl_trans *trans);
	int (*set_pnvm)(struct iwl_trans *trans, const void *data, u32 len);
	int (*set_reduce_power)(struct iwl_trans *trans,
				const void *data, u32 len);
	void (*interrupts)(struct iwl_trans *trans, bool enable);
};

/**
 * enum iwl_trans_state - state of the transport layer
 *
 * @IWL_TRANS_NO_FW: firmware wasn't started yet, or crashed
 * @IWL_TRANS_FW_STARTED: FW was started, but not alive yet
 * @IWL_TRANS_FW_ALIVE: FW has sent an alive response
 */
enum iwl_trans_state {
	IWL_TRANS_NO_FW,
	IWL_TRANS_FW_STARTED,
	IWL_TRANS_FW_ALIVE,
};

/**
 * DOC: Platform power management
 *
 * In system-wide power management the entire platform goes into a low
 * power state (e.g. idle or suspend to RAM) at the same time and the
 * device is configured as a wakeup source for the entire platform.
 * This is usually triggered by userspace activity (e.g. the user
 * presses the suspend button or a power management daemon decides to
 * put the platform in low power mode).  The device's behavior in this
 * mode is dictated by the wake-on-WLAN configuration.
 *
 * The terms used for the device's behavior are as follows:
 *
 *	- D0: the device is fully powered and the host is awake;
 *	- D3: the device is in low power mode and only reacts to
 *		specific events (e.g. magic-packet received or scan
 *		results found);
 *
 * These terms reflect the power modes in the firmware and are not to
 * be confused with the physical device power state.
 */

/**
 * enum iwl_plat_pm_mode - platform power management mode
 *
 * This enumeration describes the device's platform power management
 * behavior when in system-wide suspend (i.e WoWLAN).
 *
 * @IWL_PLAT_PM_MODE_DISABLED: power management is disabled for this
 *	device.  In system-wide suspend mode, it means that the all
 *	connections will be closed automatically by mac80211 before
 *	the platform is suspended.
 * @IWL_PLAT_PM_MODE_D3: the device goes into D3 mode (i.e. WoWLAN).
 */
enum iwl_plat_pm_mode {
	IWL_PLAT_PM_MODE_DISABLED,
	IWL_PLAT_PM_MODE_D3,
};

/**
 * enum iwl_ini_cfg_state
 * @IWL_INI_CFG_STATE_NOT_LOADED: no debug cfg was given
 * @IWL_INI_CFG_STATE_LOADED: debug cfg was found and loaded
 * @IWL_INI_CFG_STATE_CORRUPTED: debug cfg was found and some of the TLVs
 *	are corrupted. The rest of the debug TLVs will still be used
 */
enum iwl_ini_cfg_state {
	IWL_INI_CFG_STATE_NOT_LOADED,
	IWL_INI_CFG_STATE_LOADED,
	IWL_INI_CFG_STATE_CORRUPTED,
};

/* Max time to wait for nmi interrupt */
#define IWL_TRANS_NMI_TIMEOUT (HZ / 4)

/**
 * struct iwl_dram_data
 * @physical: page phy pointer
 * @block: pointer to the allocated block/page
 * @size: size of the block/page
 */
struct iwl_dram_data {
	dma_addr_t physical;
	void *block;
	int size;
};

/**
 * struct iwl_fw_mon - fw monitor per allocation id
 * @num_frags: number of fragments
 * @frags: an array of DRAM buffer fragments
 */
struct iwl_fw_mon {
	u32 num_frags;
	struct iwl_dram_data *frags;
};

/**
 * struct iwl_self_init_dram - dram data used by self init process
 * @fw: lmac and umac dram data
 * @fw_cnt: total number of items in array
 * @paging: paging dram data
 * @paging_cnt: total number of items in array
 */
struct iwl_self_init_dram {
	struct iwl_dram_data *fw;
	int fw_cnt;
	struct iwl_dram_data *paging;
	int paging_cnt;
};

/**
 * struct iwl_trans_debug - transport debug related data
 *
 * @n_dest_reg: num of reg_ops in %dbg_dest_tlv
 * @rec_on: true iff there is a fw debug recording currently active
 * @dest_tlv: points to the destination TLV for debug
 * @conf_tlv: array of pointers to configuration TLVs for debug
 * @trigger_tlv: array of pointers to triggers TLVs for debug
 * @lmac_error_event_table: addrs of lmacs error tables
 * @umac_error_event_table: addr of umac error table
 * @tcm_error_event_table: address of TCM error table
 * @error_event_table_tlv_status: bitmap that indicates what error table
 *	pointers was recevied via TLV. uses enum &iwl_error_event_table_status
 * @internal_ini_cfg: internal debug cfg state. Uses &enum iwl_ini_cfg_state
 * @external_ini_cfg: external debug cfg state. Uses &enum iwl_ini_cfg_state
 * @fw_mon_cfg: debug buffer allocation configuration
 * @fw_mon_ini: DRAM buffer fragments per allocation id
 * @fw_mon: DRAM buffer for firmware monitor
 * @hw_error: equals true if hw error interrupt was received from the FW
 * @ini_dest: debug monitor destination uses &enum iwl_fw_ini_buffer_location
 * @active_regions: active regions
 * @debug_info_tlv_list: list of debug info TLVs
 * @time_point: array of debug time points
 * @periodic_trig_list: periodic triggers list
 * @domains_bitmap: bitmap of active domains other than
 *	&IWL_FW_INI_DOMAIN_ALWAYS_ON
 */
struct iwl_trans_debug {
	u8 n_dest_reg;
	bool rec_on;

	const struct iwl_fw_dbg_dest_tlv_v1 *dest_tlv;
	const struct iwl_fw_dbg_conf_tlv *conf_tlv[FW_DBG_CONF_MAX];
	struct iwl_fw_dbg_trigger_tlv * const *trigger_tlv;

	u32 lmac_error_event_table[2];
	u32 umac_error_event_table;
	u32 tcm_error_event_table;
	unsigned int error_event_table_tlv_status;

	enum iwl_ini_cfg_state internal_ini_cfg;
	enum iwl_ini_cfg_state external_ini_cfg;

	struct iwl_fw_ini_allocation_tlv fw_mon_cfg[IWL_FW_INI_ALLOCATION_NUM];
	struct iwl_fw_mon fw_mon_ini[IWL_FW_INI_ALLOCATION_NUM];

	struct iwl_dram_data fw_mon;

	bool hw_error;
	enum iwl_fw_ini_buffer_location ini_dest;

	u64 unsupported_region_msk;
	struct iwl_ucode_tlv *active_regions[IWL_FW_INI_MAX_REGION_ID];
	struct list_head debug_info_tlv_list;
	struct iwl_dbg_tlv_time_point_data
		time_point[IWL_FW_INI_TIME_POINT_NUM];
	struct list_head periodic_trig_list;

	u32 domains_bitmap;
};

struct iwl_dma_ptr {
	dma_addr_t dma;
	void *addr;
	size_t size;
};

struct iwl_cmd_meta {
	/* only for SYNC commands, iff the reply skb is wanted */
	struct iwl_host_cmd *source;
	u32 flags;
	u32 tbs;
};

/*
 * The FH will write back to the first TB only, so we need to copy some data
 * into the buffer regardless of whether it should be mapped or not.
 * This indicates how big the first TB must be to include the scratch buffer
 * and the assigned PN.
 * Since PN location is 8 bytes at offset 12, it's 20 now.
 * If we make it bigger then allocations will be bigger and copy slower, so
 * that's probably not useful.
 */
#define IWL_FIRST_TB_SIZE	20
#define IWL_FIRST_TB_SIZE_ALIGN ALIGN(IWL_FIRST_TB_SIZE, 64)

struct iwl_pcie_txq_entry {
	void *cmd;
	struct sk_buff *skb;
	/* buffer to free after command completes */
	const void *free_buf;
	struct iwl_cmd_meta meta;
};

struct iwl_pcie_first_tb_buf {
	u8 buf[IWL_FIRST_TB_SIZE_ALIGN];
};

/**
 * struct iwl_txq - Tx Queue for DMA
 * @q: generic Rx/Tx queue descriptor
 * @tfds: transmit frame descriptors (DMA memory)
 * @first_tb_bufs: start of command headers, including scratch buffers, for
 *	the writeback -- this is DMA memory and an array holding one buffer
 *	for each command on the queue
 * @first_tb_dma: DMA address for the first_tb_bufs start
 * @entries: transmit entries (driver state)
 * @lock: queue lock
 * @stuck_timer: timer that fires if queue gets stuck
 * @trans: pointer back to transport (for timer)
 * @need_update: indicates need to update read/write index
 * @ampdu: true if this queue is an ampdu queue for an specific RA/TID
 * @wd_timeout: queue watchdog timeout (jiffies) - per queue
 * @frozen: tx stuck queue timer is frozen
 * @frozen_expiry_remainder: remember how long until the timer fires
 * @bc_tbl: byte count table of the queue (relevant only for gen2 transport)
 * @write_ptr: 1-st empty entry (index) host_w
 * @read_ptr: last used entry (index) host_r
 * @dma_addr:  physical addr for BD's
 * @n_window: safe queue window
 * @id: queue id
 * @low_mark: low watermark, resume queue if free space more than this
 * @high_mark: high watermark, stop queue if free space less than this
 *
 * A Tx queue consists of circular buffer of BDs (a.k.a. TFDs, transmit frame
 * descriptors) and required locking structures.
 *
 * Note the difference between TFD_QUEUE_SIZE_MAX and n_window: the hardware
 * always assumes 256 descriptors, so TFD_QUEUE_SIZE_MAX is always 256 (unless
 * there might be HW changes in the future). For the normal TX
 * queues, n_window, which is the size of the software queue data
 * is also 256; however, for the command queue, n_window is only
 * 32 since we don't need so many commands pending. Since the HW
 * still uses 256 BDs for DMA though, TFD_QUEUE_SIZE_MAX stays 256.
 * This means that we end up with the following:
 *  HW entries: | 0 | ... | N * 32 | ... | N * 32 + 31 | ... | 255 |
 *  SW entries:           | 0      | ... | 31          |
 * where N is a number between 0 and 7. This means that the SW
 * data is a window overlayed over the HW queue.
 */
struct iwl_txq {
	void *tfds;
	struct iwl_pcie_first_tb_buf *first_tb_bufs;
	dma_addr_t first_tb_dma;
	struct iwl_pcie_txq_entry *entries;
	/* lock for syncing changes on the queue */
	spinlock_t lock;
	unsigned long frozen_expiry_remainder;
	struct timer_list stuck_timer;
	struct iwl_trans *trans;
	bool need_update;
	bool frozen;
	bool ampdu;
	int block;
	unsigned long wd_timeout;
	struct sk_buff_head overflow_q;
	struct iwl_dma_ptr bc_tbl;

	int write_ptr;
	int read_ptr;
	dma_addr_t dma_addr;
	int n_window;
	u32 id;
	int low_mark;
	int high_mark;

	bool overflow_tx;
};

/**
 * struct iwl_trans_txqs - transport tx queues data
 *
 * @bc_table_dword: true if the BC table expects DWORD (as opposed to bytes)
 * @page_offs: offset from skb->cb to mac header page pointer
 * @dev_cmd_offs: offset from skb->cb to iwl_device_tx_cmd pointer
 * @queue_used - bit mask of used queues
 * @queue_stopped - bit mask of stopped queues
 * @scd_bc_tbls: gen1 pointer to the byte count table of the scheduler
 */
struct iwl_trans_txqs {
	unsigned long queue_used[BITS_TO_LONGS(IWL_MAX_TVQM_QUEUES)];
	unsigned long queue_stopped[BITS_TO_LONGS(IWL_MAX_TVQM_QUEUES)];
	struct iwl_txq *txq[IWL_MAX_TVQM_QUEUES];
	struct dma_pool *bc_pool;
	size_t bc_tbl_size;
	bool bc_table_dword;
	u8 page_offs;
	u8 dev_cmd_offs;
	struct iwl_tso_hdr_page __percpu *tso_hdr_page;

	struct {
		u8 fifo;
		u8 q_id;
		unsigned int wdg_timeout;
	} cmd;

	struct {
		u8 max_tbs;
		u16 size;
		u8 addr_size;
	} tfd;

	struct iwl_dma_ptr scd_bc_tbls;
};

/**
 * struct iwl_trans - transport common data
 *
 * @ops - pointer to iwl_trans_ops
 * @op_mode - pointer to the op_mode
 * @trans_cfg: the trans-specific configuration part
 * @cfg - pointer to the configuration
 * @drv - pointer to iwl_drv
 * @status: a bit-mask of transport status flags
 * @dev - pointer to struct device * that represents the device
 * @max_skb_frags: maximum number of fragments an SKB can have when transmitted.
 *	0 indicates that frag SKBs (NETIF_F_SG) aren't supported.
 * @hw_rf_id a u32 with the device RF ID
 * @hw_id: a u32 with the ID of the device / sub-device.
 *	Set during transport allocation.
 * @hw_id_str: a string with info about HW ID. Set during transport allocation.
 * @pm_support: set to true in start_hw if link pm is supported
 * @ltr_enabled: set to true if the LTR is enabled
 * @wide_cmd_header: true when ucode supports wide command header format
 * @wait_command_queue: wait queue for sync commands
 * @num_rx_queues: number of RX queues allocated by the transport;
 *	the transport must set this before calling iwl_drv_start()
 * @iml_len: the length of the image loader
 * @iml: a pointer to the image loader itself
 * @dev_cmd_pool: pool for Tx cmd allocation - for internal use only.
 *	The user should use iwl_trans_{alloc,free}_tx_cmd.
 * @rx_mpdu_cmd: MPDU RX command ID, must be assigned by opmode before
 *	starting the firmware, used for tracing
 * @rx_mpdu_cmd_hdr_size: used for tracing, amount of data before the
 *	start of the 802.11 header in the @rx_mpdu_cmd
 * @dflt_pwr_limit: default power limit fetched from the platform (ACPI)
 * @system_pm_mode: the system-wide power management mode in use.
 *	This mode is set dynamically, depending on the WoWLAN values
 *	configured from the userspace at runtime.
 * @iwl_trans_txqs: transport tx queues data.
 */
struct iwl_trans {
	const struct iwl_trans_ops *ops;
	struct iwl_op_mode *op_mode;
	const struct iwl_cfg_trans_params *trans_cfg;
	const struct iwl_cfg *cfg;
	struct iwl_drv *drv;
	enum iwl_trans_state state;
	unsigned long status;

	struct device *dev;
	u32 max_skb_frags;
	u32 hw_rev;
	u32 hw_rf_id;
	u32 hw_id;
	char hw_id_str[52];
	u32 sku_id[3];

	u8 rx_mpdu_cmd, rx_mpdu_cmd_hdr_size;

	bool pm_support;
	bool ltr_enabled;
	u8 pnvm_loaded:1;
	u8 reduce_power_loaded:1;

	const struct iwl_hcmd_arr *command_groups;
	int command_groups_size;
	bool wide_cmd_header;

	wait_queue_head_t wait_command_queue;
	u8 num_rx_queues;

	size_t iml_len;
	u8 *iml;

	/* The following fields are internal only */
	struct kmem_cache *dev_cmd_pool;
	char dev_cmd_pool_name[50];

	struct dentry *dbgfs_dir;

#ifdef CONFIG_LOCKDEP
	struct lockdep_map sync_cmd_lockdep_map;
#endif

	struct iwl_trans_debug dbg;
	struct iwl_self_init_dram init_dram;

	enum iwl_plat_pm_mode system_pm_mode;

	const char *name;
	struct iwl_trans_txqs txqs;

	/* pointer to trans specific struct */
	/*Ensure that this pointer will always be aligned to sizeof pointer */
	char trans_specific[] __aligned(sizeof(void *));
};

const char *iwl_get_cmd_string(struct iwl_trans *trans, u32 id);
int iwl_cmd_groups_verify_sorted(const struct iwl_trans_config *trans);

static inline void iwl_trans_configure(struct iwl_trans *trans,
				       const struct iwl_trans_config *trans_cfg)
{
	trans->op_mode = trans_cfg->op_mode;

	trans->ops->configure(trans, trans_cfg);
	WARN_ON(iwl_cmd_groups_verify_sorted(trans_cfg));
}

static inline int iwl_trans_start_hw(struct iwl_trans *trans)
{
	might_sleep();

	return trans->ops->start_hw(trans);
}

static inline void iwl_trans_op_mode_leave(struct iwl_trans *trans)
{
	might_sleep();

	if (trans->ops->op_mode_leave)
		trans->ops->op_mode_leave(trans);

	trans->op_mode = NULL;

	trans->state = IWL_TRANS_NO_FW;
}

static inline void iwl_trans_fw_alive(struct iwl_trans *trans, u32 scd_addr)
{
	might_sleep();

	trans->state = IWL_TRANS_FW_ALIVE;

	trans->ops->fw_alive(trans, scd_addr);
}

static inline int iwl_trans_start_fw(struct iwl_trans *trans,
				     const struct fw_img *fw,
				     bool run_in_rfkill)
{
	int ret;

	might_sleep();

	WARN_ON_ONCE(!trans->rx_mpdu_cmd);

	clear_bit(STATUS_FW_ERROR, &trans->status);
	ret = trans->ops->start_fw(trans, fw, run_in_rfkill);
	if (ret == 0)
		trans->state = IWL_TRANS_FW_STARTED;

	return ret;
}

static inline void iwl_trans_stop_device(struct iwl_trans *trans)
{
	might_sleep();

	trans->ops->stop_device(trans);

	trans->state = IWL_TRANS_NO_FW;
}

static inline int iwl_trans_d3_suspend(struct iwl_trans *trans, bool test,
				       bool reset)
{
	might_sleep();
	if (!trans->ops->d3_suspend)
		return 0;

	return trans->ops->d3_suspend(trans, test, reset);
}

static inline int iwl_trans_d3_resume(struct iwl_trans *trans,
				      enum iwl_d3_status *status,
				      bool test, bool reset)
{
	might_sleep();
	if (!trans->ops->d3_resume)
		return 0;

	return trans->ops->d3_resume(trans, status, test, reset);
}

static inline struct iwl_trans_dump_data *
iwl_trans_dump_data(struct iwl_trans *trans, u32 dump_mask)
{
	if (!trans->ops->dump_data)
		return NULL;
	return trans->ops->dump_data(trans, dump_mask);
}

static inline struct iwl_device_tx_cmd *
iwl_trans_alloc_tx_cmd(struct iwl_trans *trans)
{
	return kmem_cache_zalloc(trans->dev_cmd_pool, GFP_ATOMIC);
}

int iwl_trans_send_cmd(struct iwl_trans *trans, struct iwl_host_cmd *cmd);

static inline void iwl_trans_free_tx_cmd(struct iwl_trans *trans,
					 struct iwl_device_tx_cmd *dev_cmd)
{
	kmem_cache_free(trans->dev_cmd_pool, dev_cmd);
}

static inline int iwl_trans_tx(struct iwl_trans *trans, struct sk_buff *skb,
			       struct iwl_device_tx_cmd *dev_cmd, int queue)
{
	if (unlikely(test_bit(STATUS_FW_ERROR, &trans->status)))
		return -EIO;

	if (WARN_ON_ONCE(trans->state != IWL_TRANS_FW_ALIVE)) {
		IWL_ERR(trans, "%s bad state = %d\n", __func__, trans->state);
		return -EIO;
	}

	return trans->ops->tx(trans, skb, dev_cmd, queue);
}

static inline void iwl_trans_reclaim(struct iwl_trans *trans, int queue,
				     int ssn, struct sk_buff_head *skbs)
{
	if (WARN_ON_ONCE(trans->state != IWL_TRANS_FW_ALIVE)) {
		IWL_ERR(trans, "%s bad state = %d\n", __func__, trans->state);
		return;
	}

	trans->ops->reclaim(trans, queue, ssn, skbs);
}

static inline void iwl_trans_set_q_ptrs(struct iwl_trans *trans, int queue,
					int ptr)
{
	if (WARN_ON_ONCE(trans->state != IWL_TRANS_FW_ALIVE)) {
		IWL_ERR(trans, "%s bad state = %d\n", __func__, trans->state);
		return;
	}

	trans->ops->set_q_ptrs(trans, queue, ptr);
}

static inline void iwl_trans_txq_disable(struct iwl_trans *trans, int queue,
					 bool configure_scd)
{
	trans->ops->txq_disable(trans, queue, configure_scd);
}

static inline bool
iwl_trans_txq_enable_cfg(struct iwl_trans *trans, int queue, u16 ssn,
			 const struct iwl_trans_txq_scd_cfg *cfg,
			 unsigned int queue_wdg_timeout)
{
	might_sleep();

	if (WARN_ON_ONCE(trans->state != IWL_TRANS_FW_ALIVE)) {
		IWL_ERR(trans, "%s bad state = %d\n", __func__, trans->state);
		return false;
	}

	return trans->ops->txq_enable(trans, queue, ssn,
				      cfg, queue_wdg_timeout);
}

static inline int
iwl_trans_get_rxq_dma_data(struct iwl_trans *trans, int queue,
			   struct iwl_trans_rxq_dma_data *data)
{
	if (WARN_ON_ONCE(!trans->ops->rxq_dma_data))
		return -ENOTSUPP;

	return trans->ops->rxq_dma_data(trans, queue, data);
}

static inline void
iwl_trans_txq_free(struct iwl_trans *trans, int queue)
{
	if (WARN_ON_ONCE(!trans->ops->txq_free))
		return;

	trans->ops->txq_free(trans, queue);
}

static inline int
iwl_trans_txq_alloc(struct iwl_trans *trans,
		    __le16 flags, u8 sta_id, u8 tid,
		    int cmd_id, int size,
		    unsigned int wdg_timeout)
{
	might_sleep();

	if (WARN_ON_ONCE(!trans->ops->txq_alloc))
		return -ENOTSUPP;

	if (WARN_ON_ONCE(trans->state != IWL_TRANS_FW_ALIVE)) {
		IWL_ERR(trans, "%s bad state = %d\n", __func__, trans->state);
		return -EIO;
	}

	return trans->ops->txq_alloc(trans, flags, sta_id, tid,
				     cmd_id, size, wdg_timeout);
}

static inline void iwl_trans_txq_set_shared_mode(struct iwl_trans *trans,
						 int queue, bool shared_mode)
{
	if (trans->ops->txq_set_shared_mode)
		trans->ops->txq_set_shared_mode(trans, queue, shared_mode);
}

static inline void iwl_trans_txq_enable(struct iwl_trans *trans, int queue,
					int fifo, int sta_id, int tid,
					int frame_limit, u16 ssn,
					unsigned int queue_wdg_timeout)
{
	struct iwl_trans_txq_scd_cfg cfg = {
		.fifo = fifo,
		.sta_id = sta_id,
		.tid = tid,
		.frame_limit = frame_limit,
		.aggregate = sta_id >= 0,
	};

	iwl_trans_txq_enable_cfg(trans, queue, ssn, &cfg, queue_wdg_timeout);
}

static inline
void iwl_trans_ac_txq_enable(struct iwl_trans *trans, int queue, int fifo,
			     unsigned int queue_wdg_timeout)
{
	struct iwl_trans_txq_scd_cfg cfg = {
		.fifo = fifo,
		.sta_id = -1,
		.tid = IWL_MAX_TID_COUNT,
		.frame_limit = IWL_FRAME_LIMIT,
		.aggregate = false,
	};

	iwl_trans_txq_enable_cfg(trans, queue, 0, &cfg, queue_wdg_timeout);
}

static inline void iwl_trans_freeze_txq_timer(struct iwl_trans *trans,
					      unsigned long txqs,
					      bool freeze)
{
	if (WARN_ON_ONCE(trans->state != IWL_TRANS_FW_ALIVE)) {
		IWL_ERR(trans, "%s bad state = %d\n", __func__, trans->state);
		return;
	}

	if (trans->ops->freeze_txq_timer)
		trans->ops->freeze_txq_timer(trans, txqs, freeze);
}

static inline void iwl_trans_block_txq_ptrs(struct iwl_trans *trans,
					    bool block)
{
	if (WARN_ON_ONCE(trans->state != IWL_TRANS_FW_ALIVE)) {
		IWL_ERR(trans, "%s bad state = %d\n", __func__, trans->state);
		return;
	}

	if (trans->ops->block_txq_ptrs)
		trans->ops->block_txq_ptrs(trans, block);
}

static inline int iwl_trans_wait_tx_queues_empty(struct iwl_trans *trans,
						 u32 txqs)
{
	if (WARN_ON_ONCE(!trans->ops->wait_tx_queues_empty))
		return -ENOTSUPP;

	/* No need to wait if the firmware is not alive */
	if (trans->state != IWL_TRANS_FW_ALIVE) {
		IWL_ERR(trans, "%s bad state = %d\n", __func__, trans->state);
		return -EIO;
	}

	return trans->ops->wait_tx_queues_empty(trans, txqs);
}

static inline int iwl_trans_wait_txq_empty(struct iwl_trans *trans, int queue)
{
	if (WARN_ON_ONCE(!trans->ops->wait_txq_empty))
		return -ENOTSUPP;

	if (WARN_ON_ONCE(trans->state != IWL_TRANS_FW_ALIVE)) {
		IWL_ERR(trans, "%s bad state = %d\n", __func__, trans->state);
		return -EIO;
	}

	return trans->ops->wait_txq_empty(trans, queue);
}

static inline void iwl_trans_write8(struct iwl_trans *trans, u32 ofs, u8 val)
{
	trans->ops->write8(trans, ofs, val);
}

static inline void iwl_trans_write32(struct iwl_trans *trans, u32 ofs, u32 val)
{
	trans->ops->write32(trans, ofs, val);
}

static inline u32 iwl_trans_read32(struct iwl_trans *trans, u32 ofs)
{
	return trans->ops->read32(trans, ofs);
}

static inline u32 iwl_trans_read_prph(struct iwl_trans *trans, u32 ofs)
{
	return trans->ops->read_prph(trans, ofs);
}

static inline void iwl_trans_write_prph(struct iwl_trans *trans, u32 ofs,
					u32 val)
{
	return trans->ops->write_prph(trans, ofs, val);
}

static inline int iwl_trans_read_mem(struct iwl_trans *trans, u32 addr,
				     void *buf, int dwords)
{
	return trans->ops->read_mem(trans, addr, buf, dwords);
}

#define iwl_trans_read_mem_bytes(trans, addr, buf, bufsize)		      \
	do {								      \
		if (__builtin_constant_p(bufsize))			      \
			BUILD_BUG_ON((bufsize) % sizeof(u32));		      \
		iwl_trans_read_mem(trans, addr, buf, (bufsize) / sizeof(u32));\
	} while (0)

static inline u32 iwl_trans_read_mem32(struct iwl_trans *trans, u32 addr)
{
	u32 value;

	if (WARN_ON(iwl_trans_read_mem(trans, addr, &value, 1)))
		return 0xa5a5a5a5;

	return value;
}

static inline int iwl_trans_write_mem(struct iwl_trans *trans, u32 addr,
				      const void *buf, int dwords)
{
	return trans->ops->write_mem(trans, addr, buf, dwords);
}

static inline u32 iwl_trans_write_mem32(struct iwl_trans *trans, u32 addr,
					u32 val)
{
	return iwl_trans_write_mem(trans, addr, &val, 1);
}

static inline void iwl_trans_set_pmi(struct iwl_trans *trans, bool state)
{
	if (trans->ops->set_pmi)
		trans->ops->set_pmi(trans, state);
}

static inline void iwl_trans_sw_reset(struct iwl_trans *trans)
{
	if (trans->ops->sw_reset)
		trans->ops->sw_reset(trans);
}

static inline void
iwl_trans_set_bits_mask(struct iwl_trans *trans, u32 reg, u32 mask, u32 value)
{
	trans->ops->set_bits_mask(trans, reg, mask, value);
}

#define iwl_trans_grab_nic_access(trans)		\
	__cond_lock(nic_access,				\
		    likely((trans)->ops->grab_nic_access(trans)))

static inline void __releases(nic_access)
iwl_trans_release_nic_access(struct iwl_trans *trans)
{
	trans->ops->release_nic_access(trans);
	__release(nic_access);
}

static inline void iwl_trans_fw_error(struct iwl_trans *trans, bool sync)
{
	if (WARN_ON_ONCE(!trans->op_mode))
		return;

	/* prevent double restarts due to the same erroneous FW */
	if (!test_and_set_bit(STATUS_FW_ERROR, &trans->status)) {
		iwl_op_mode_nic_error(trans->op_mode, sync);
		trans->state = IWL_TRANS_NO_FW;
	}
}

static inline bool iwl_trans_fw_running(struct iwl_trans *trans)
{
	return trans->state == IWL_TRANS_FW_ALIVE;
}

static inline void iwl_trans_sync_nmi(struct iwl_trans *trans)
{
	if (trans->ops->sync_nmi)
		trans->ops->sync_nmi(trans);
}

void iwl_trans_sync_nmi_with_addr(struct iwl_trans *trans, u32 inta_addr,
				  u32 sw_err_bit);

static inline int iwl_trans_set_pnvm(struct iwl_trans *trans,
				     const void *data, u32 len)
{
	if (trans->ops->set_pnvm) {
		int ret = trans->ops->set_pnvm(trans, data, len);

		if (ret)
			return ret;
	}

	trans->pnvm_loaded = true;

	return 0;
}

static inline int iwl_trans_set_reduce_power(struct iwl_trans *trans,
					     const void *data, u32 len)
{
	if (trans->ops->set_reduce_power) {
		int ret = trans->ops->set_reduce_power(trans, data, len);

		if (ret)
			return ret;
	}

	trans->reduce_power_loaded = true;
	return 0;
}

static inline bool iwl_trans_dbg_ini_valid(struct iwl_trans *trans)
{
	return trans->dbg.internal_ini_cfg != IWL_INI_CFG_STATE_NOT_LOADED ||
		trans->dbg.external_ini_cfg != IWL_INI_CFG_STATE_NOT_LOADED;
}

static inline void iwl_trans_interrupts(struct iwl_trans *trans, bool enable)
{
	if (trans->ops->interrupts)
		trans->ops->interrupts(trans, enable);
}

/*****************************************************
 * transport helper functions
 *****************************************************/
struct iwl_trans *iwl_trans_alloc(unsigned int priv_size,
			  struct device *dev,
			  const struct iwl_trans_ops *ops,
			  const struct iwl_cfg_trans_params *cfg_trans);
int iwl_trans_init(struct iwl_trans *trans);
void iwl_trans_free(struct iwl_trans *trans);

/*****************************************************
* driver (transport) register/unregister functions
******************************************************/
int __must_check iwl_pci_register_driver(void);
void iwl_pci_unregister_driver(void);

#endif /* __iwl_trans_h__ */
struct iwl_self_init_dram {
	struct iwl_dram_data *fw;
	int fw_cnt;
	struct iwl_dram_data *paging;
	int paging_cnt;
};

/**
 * struct iwl_trans_debug - transport debug related data
 *
 * @n_dest_reg: num of reg_ops in %dbg_dest_tlv
 * @rec_on: true iff there is a fw debug recording currently active
 * @dest_tlv: points to the destination TLV for debug
 * @conf_tlv: array of pointers to configuration TLVs for debug
 * @trigger_tlv: array of pointers to triggers TLVs for debug
 * @lmac_error_event_table: addrs of lmacs error tables
 * @umac_error_event_table: addr of umac error table
 * @tcm_error_event_table: address of TCM error table
 * @error_event_table_tlv_status: bitmap that indicates what error table
 *	pointers was recevied via TLV. uses enum &iwl_error_event_table_status
 * @internal_ini_cfg: internal debug cfg state. Uses &enum iwl_ini_cfg_state
 * @external_ini_cfg: external debug cfg state. Uses &enum iwl_ini_cfg_state
 * @fw_mon_cfg: debug buffer allocation configuration
 * @fw_mon_ini: DRAM buffer fragments per allocation id
 * @fw_mon: DRAM buffer for firmware monitor
 * @hw_error: equals true if hw error interrupt was received from the FW
 * @ini_dest: debug monitor destination uses &enum iwl_fw_ini_buffer_location
 * @active_regions: active regions
 * @debug_info_tlv_list: list of debug info TLVs
 * @time_point: array of debug time points
 * @periodic_trig_list: periodic triggers list
 * @domains_bitmap: bitmap of active domains other than
 *	&IWL_FW_INI_DOMAIN_ALWAYS_ON
 */
struct iwl_trans_debug {
	u8 n_dest_reg;
	bool rec_on;

	const struct iwl_fw_dbg_dest_tlv_v1 *dest_tlv;
	const struct iwl_fw_dbg_conf_tlv *conf_tlv[FW_DBG_CONF_MAX];
	struct iwl_fw_dbg_trigger_tlv * const *trigger_tlv;

	u32 lmac_error_event_table[2];
	u32 umac_error_event_table;
	u32 tcm_error_event_table;
	unsigned int error_event_table_tlv_status;

	enum iwl_ini_cfg_state internal_ini_cfg;
	enum iwl_ini_cfg_state external_ini_cfg;

	struct iwl_fw_ini_allocation_tlv fw_mon_cfg[IWL_FW_INI_ALLOCATION_NUM];
	struct iwl_fw_mon fw_mon_ini[IWL_FW_INI_ALLOCATION_NUM];

	struct iwl_dram_data fw_mon;

	bool hw_error;
	enum iwl_fw_ini_buffer_location ini_dest;

	u64 unsupported_region_msk;
	struct iwl_ucode_tlv *active_regions[IWL_FW_INI_MAX_REGION_ID];
	struct list_head debug_info_tlv_list;
	struct iwl_dbg_tlv_time_point_data
		time_point[IWL_FW_INI_TIME_POINT_NUM];
	struct list_head periodic_trig_list;

	u32 domains_bitmap;
};

struct iwl_dma_ptr {
	dma_addr_t dma;
	void *addr;
	size_t size;
};

struct iwl_cmd_meta {
	/* only for SYNC commands, iff the reply skb is wanted */
	struct iwl_host_cmd *source;
	u32 flags;
	u32 tbs;
};

/*
 * The FH will write back to the first TB only, so we need to copy some data
 * into the buffer regardless of whether it should be mapped or not.
 * This indicates how big the first TB must be to include the scratch buffer
 * and the assigned PN.
 * Since PN location is 8 bytes at offset 12, it's 20 now.
 * If we make it bigger then allocations will be bigger and copy slower, so
 * that's probably not useful.
 */
#define IWL_FIRST_TB_SIZE	20
#define IWL_FIRST_TB_SIZE_ALIGN ALIGN(IWL_FIRST_TB_SIZE, 64)

struct iwl_pcie_txq_entry {
	void *cmd;
	struct sk_buff *skb;
	/* buffer to free after command completes */
	const void *free_buf;
	struct iwl_cmd_meta meta;
};

struct iwl_pcie_first_tb_buf {
	u8 buf[IWL_FIRST_TB_SIZE_ALIGN];
};

/**
 * struct iwl_txq - Tx Queue for DMA
 * @q: generic Rx/Tx queue descriptor
 * @tfds: transmit frame descriptors (DMA memory)
 * @first_tb_bufs: start of command headers, including scratch buffers, for
 *	the writeback -- this is DMA memory and an array holding one buffer
 *	for each command on the queue
 * @first_tb_dma: DMA address for the first_tb_bufs start
 * @entries: transmit entries (driver state)
 * @lock: queue lock
 * @stuck_timer: timer that fires if queue gets stuck
 * @trans: pointer back to transport (for timer)
 * @need_update: indicates need to update read/write index
 * @ampdu: true if this queue is an ampdu queue for an specific RA/TID
 * @wd_timeout: queue watchdog timeout (jiffies) - per queue
 * @frozen: tx stuck queue timer is frozen
 * @frozen_expiry_remainder: remember how long until the timer fires
 * @bc_tbl: byte count table of the queue (relevant only for gen2 transport)
 * @write_ptr: 1-st empty entry (index) host_w
 * @read_ptr: last used entry (index) host_r
 * @dma_addr:  physical addr for BD's
 * @n_window: safe queue window
 * @id: queue id
 * @low_mark: low watermark, resume queue if free space more than this
 * @high_mark: high watermark, stop queue if free space less than this
 *
 * A Tx queue consists of circular buffer of BDs (a.k.a. TFDs, transmit frame
 * descriptors) and required locking structures.
 *
 * Note the difference between TFD_QUEUE_SIZE_MAX and n_window: the hardware
 * always assumes 256 descriptors, so TFD_QUEUE_SIZE_MAX is always 256 (unless
 * there might be HW changes in the future). For the normal TX
 * queues, n_window, which is the size of the software queue data
 * is also 256; however, for the command queue, n_window is only
 * 32 since we don't need so many commands pending. Since the HW
 * still uses 256 BDs for DMA though, TFD_QUEUE_SIZE_MAX stays 256.
 * This means that we end up with the following:
 *  HW entries: | 0 | ... | N * 32 | ... | N * 32 + 31 | ... | 255 |
 *  SW entries:           | 0      | ... | 31          |
 * where N is a number between 0 and 7. This means that the SW
 * data is a window overlayed over the HW queue.
 */
struct iwl_txq {
	void *tfds;
	struct iwl_pcie_first_tb_buf *first_tb_bufs;
	dma_addr_t first_tb_dma;
	struct iwl_pcie_txq_entry *entries;
	/* lock for syncing changes on the queue */
	spinlock_t lock;
	unsigned long frozen_expiry_remainder;
	struct timer_list stuck_timer;
	struct iwl_trans *trans;
	bool need_update;
	bool frozen;
	bool ampdu;
	int block;
	unsigned long wd_timeout;
	struct sk_buff_head overflow_q;
	struct iwl_dma_ptr bc_tbl;

	int write_ptr;
	int read_ptr;
	dma_addr_t dma_addr;
	int n_window;
	u32 id;
	int low_mark;
	int high_mark;

	bool overflow_tx;
};

/**
 * struct iwl_trans_txqs - transport tx queues data
 *
 * @bc_table_dword: true if the BC table expects DWORD (as opposed to bytes)
 * @page_offs: offset from skb->cb to mac header page pointer
 * @dev_cmd_offs: offset from skb->cb to iwl_device_tx_cmd pointer
 * @queue_used - bit mask of used queues
 * @queue_stopped - bit mask of stopped queues
 * @scd_bc_tbls: gen1 pointer to the byte count table of the scheduler
 */
struct iwl_trans_txqs {
	unsigned long queue_used[BITS_TO_LONGS(IWL_MAX_TVQM_QUEUES)];
	unsigned long queue_stopped[BITS_TO_LONGS(IWL_MAX_TVQM_QUEUES)];
	struct iwl_txq *txq[IWL_MAX_TVQM_QUEUES];
	struct dma_pool *bc_pool;
	size_t bc_tbl_size;
	bool bc_table_dword;
	u8 page_offs;
	u8 dev_cmd_offs;
	struct iwl_tso_hdr_page __percpu *tso_hdr_page;

	struct {
		u8 fifo;
		u8 q_id;
		unsigned int wdg_timeout;
	} cmd;

	struct {
		u8 max_tbs;
		u16 size;
		u8 addr_size;
	} tfd;

	struct iwl_dma_ptr scd_bc_tbls;
};

/**
 * struct iwl_trans - transport common data
 *
 * @ops - pointer to iwl_trans_ops
 * @op_mode - pointer to the op_mode
 * @trans_cfg: the trans-specific configuration part
 * @cfg - pointer to the configuration
 * @drv - pointer to iwl_drv
 * @status: a bit-mask of transport status flags
 * @dev - pointer to struct device * that represents the device
 * @max_skb_frags: maximum number of fragments an SKB can have when transmitted.
 *	0 indicates that frag SKBs (NETIF_F_SG) aren't supported.
 * @hw_rf_id a u32 with the device RF ID
 * @hw_id: a u32 with the ID of the device / sub-device.
 *	Set during transport allocation.
 * @hw_id_str: a string with info about HW ID. Set during transport allocation.
 * @pm_support: set to true in start_hw if link pm is supported
 * @ltr_enabled: set to true if the LTR is enabled
 * @wide_cmd_header: true when ucode supports wide command header format
 * @wait_command_queue: wait queue for sync commands
 * @num_rx_queues: number of RX queues allocated by the transport;
 *	the transport must set this before calling iwl_drv_start()
 * @iml_len: the length of the image loader
 * @iml: a pointer to the image loader itself
 * @dev_cmd_pool: pool for Tx cmd allocation - for internal use only.
 *	The user should use iwl_trans_{alloc,free}_tx_cmd.
 * @rx_mpdu_cmd: MPDU RX command ID, must be assigned by opmode before
 *	starting the firmware, used for tracing
 * @rx_mpdu_cmd_hdr_size: used for tracing, amount of data before the
 *	start of the 802.11 header in the @rx_mpdu_cmd
 * @dflt_pwr_limit: default power limit fetched from the platform (ACPI)
 * @system_pm_mode: the system-wide power management mode in use.
 *	This mode is set dynamically, depending on the WoWLAN values
 *	configured from the userspace at runtime.
 * @iwl_trans_txqs: transport tx queues data.
 */
struct iwl_trans {
	const struct iwl_trans_ops *ops;
	struct iwl_op_mode *op_mode;
	const struct iwl_cfg_trans_params *trans_cfg;
	const struct iwl_cfg *cfg;
	struct iwl_drv *drv;
	enum iwl_trans_state state;
	unsigned long status;

	struct device *dev;
	u32 max_skb_frags;
	u32 hw_rev;
	u32 hw_rf_id;
	u32 hw_id;
	char hw_id_str[52];
	u32 sku_id[3];

	u8 rx_mpdu_cmd, rx_mpdu_cmd_hdr_size;

	bool pm_support;
	bool ltr_enabled;
	u8 pnvm_loaded:1;
	u8 reduce_power_loaded:1;

	const struct iwl_hcmd_arr *command_groups;
	int command_groups_size;
	bool wide_cmd_header;

	wait_queue_head_t wait_command_queue;
	u8 num_rx_queues;

	size_t iml_len;
	u8 *iml;

	/* The following fields are internal only */
	struct kmem_cache *dev_cmd_pool;
	char dev_cmd_pool_name[50];

	struct dentry *dbgfs_dir;

#ifdef CONFIG_LOCKDEP
	struct lockdep_map sync_cmd_lockdep_map;
#endif

	struct iwl_trans_debug dbg;
	struct iwl_self_init_dram init_dram;

	enum iwl_plat_pm_mode system_pm_mode;

	const char *name;
	struct iwl_trans_txqs txqs;

	/* pointer to trans specific struct */
	/*Ensure that this pointer will always be aligned to sizeof pointer */
	char trans_specific[] __aligned(sizeof(void *));
};

const char *iwl_get_cmd_string(struct iwl_trans *trans, u32 id);
int iwl_cmd_groups_verify_sorted(const struct iwl_trans_config *trans);

static inline void iwl_trans_configure(struct iwl_trans *trans,
				       const struct iwl_trans_config *trans_cfg)
{
	trans->op_mode = trans_cfg->op_mode;

	trans->ops->configure(trans, trans_cfg);
	WARN_ON(iwl_cmd_groups_verify_sorted(trans_cfg));
}

static inline int iwl_trans_start_hw(struct iwl_trans *trans)
{
	might_sleep();

	return trans->ops->start_hw(trans);
}

static inline void iwl_trans_op_mode_leave(struct iwl_trans *trans)
{
	might_sleep();

	if (trans->ops->op_mode_leave)
		trans->ops->op_mode_leave(trans);

	trans->op_mode = NULL;

	trans->state = IWL_TRANS_NO_FW;
}

static inline void iwl_trans_fw_alive(struct iwl_trans *trans, u32 scd_addr)
{
	might_sleep();

	trans->state = IWL_TRANS_FW_ALIVE;

	trans->ops->fw_alive(trans, scd_addr);
}

static inline int iwl_trans_start_fw(struct iwl_trans *trans,
				     const struct fw_img *fw,
				     bool run_in_rfkill)
{
	int ret;

	might_sleep();

	WARN_ON_ONCE(!trans->rx_mpdu_cmd);

	clear_bit(STATUS_FW_ERROR, &trans->status);
	ret = trans->ops->start_fw(trans, fw, run_in_rfkill);
	if (ret == 0)
		trans->state = IWL_TRANS_FW_STARTED;

	return ret;
}

static inline void iwl_trans_stop_device(struct iwl_trans *trans)
{
	might_sleep();

	trans->ops->stop_device(trans);

	trans->state = IWL_TRANS_NO_FW;
}

static inline int iwl_trans_d3_suspend(struct iwl_trans *trans, bool test,
				       bool reset)
{
	might_sleep();
	if (!trans->ops->d3_suspend)
		return 0;

	return trans->ops->d3_suspend(trans, test, reset);
}

static inline int iwl_trans_d3_resume(struct iwl_trans *trans,
				      enum iwl_d3_status *status,
				      bool test, bool reset)
{
	might_sleep();
	if (!trans->ops->d3_resume)
		return 0;

	return trans->ops->d3_resume(trans, status, test, reset);
}

static inline struct iwl_trans_dump_data *
iwl_trans_dump_data(struct iwl_trans *trans, u32 dump_mask)
{
	if (!trans->ops->dump_data)
		return NULL;
	return trans->ops->dump_data(trans, dump_mask);
}

static inline struct iwl_device_tx_cmd *
iwl_trans_alloc_tx_cmd(struct iwl_trans *trans)
{
	return kmem_cache_zalloc(trans->dev_cmd_pool, GFP_ATOMIC);
}

int iwl_trans_send_cmd(struct iwl_trans *trans, struct iwl_host_cmd *cmd);

static inline void iwl_trans_free_tx_cmd(struct iwl_trans *trans,
					 struct iwl_device_tx_cmd *dev_cmd)
{
	kmem_cache_free(trans->dev_cmd_pool, dev_cmd);
}

static inline int iwl_trans_tx(struct iwl_trans *trans, struct sk_buff *skb,
			       struct iwl_device_tx_cmd *dev_cmd, int queue)
{
	if (unlikely(test_bit(STATUS_FW_ERROR, &trans->status)))
		return -EIO;

	if (WARN_ON_ONCE(trans->state != IWL_TRANS_FW_ALIVE)) {
		IWL_ERR(trans, "%s bad state = %d\n", __func__, trans->state);
		return -EIO;
	}

	return trans->ops->tx(trans, skb, dev_cmd, queue);
}

static inline void iwl_trans_reclaim(struct iwl_trans *trans, int queue,
				     int ssn, struct sk_buff_head *skbs)
{
	if (WARN_ON_ONCE(trans->state != IWL_TRANS_FW_ALIVE)) {
		IWL_ERR(trans, "%s bad state = %d\n", __func__, trans->state);
		return;
	}

	trans->ops->reclaim(trans, queue, ssn, skbs);
}

static inline void iwl_trans_set_q_ptrs(struct iwl_trans *trans, int queue,
					int ptr)
{
	if (WARN_ON_ONCE(trans->state != IWL_TRANS_FW_ALIVE)) {
		IWL_ERR(trans, "%s bad state = %d\n", __func__, trans->state);
		return;
	}

	trans->ops->set_q_ptrs(trans, queue, ptr);
}

static inline void iwl_trans_txq_disable(struct iwl_trans *trans, int queue,
					 bool configure_scd)
{
	trans->ops->txq_disable(trans, queue, configure_scd);
}

static inline bool
iwl_trans_txq_enable_cfg(struct iwl_trans *trans, int queue, u16 ssn,
			 const struct iwl_trans_txq_scd_cfg *cfg,
			 unsigned int queue_wdg_timeout)
{
	might_sleep();

	if (WARN_ON_ONCE(trans->state != IWL_TRANS_FW_ALIVE)) {
		IWL_ERR(trans, "%s bad state = %d\n", __func__, trans->state);
		return false;
	}

	return trans->ops->txq_enable(trans, queue, ssn,
				      cfg, queue_wdg_timeout);
}

static inline int
iwl_trans_get_rxq_dma_data(struct iwl_trans *trans, int queue,
			   struct iwl_trans_rxq_dma_data *data)
{
	if (WARN_ON_ONCE(!trans->ops->rxq_dma_data))
		return -ENOTSUPP;

	return trans->ops->rxq_dma_data(trans, queue, data);
}

static inline void
iwl_trans_txq_free(struct iwl_trans *trans, int queue)
{
	if (WARN_ON_ONCE(!trans->ops->txq_free))
		return;

	trans->ops->txq_free(trans, queue);
}

static inline int
iwl_trans_txq_alloc(struct iwl_trans *trans,
		    __le16 flags, u8 sta_id, u8 tid,
		    int cmd_id, int size,
		    unsigned int wdg_timeout)
{
	might_sleep();

	if (WARN_ON_ONCE(!trans->ops->txq_alloc))
		return -ENOTSUPP;

	if (WARN_ON_ONCE(trans->state != IWL_TRANS_FW_ALIVE)) {
		IWL_ERR(trans, "%s bad state = %d\n", __func__, trans->state);
		return -EIO;
	}

	return trans->ops->txq_alloc(trans, flags, sta_id, tid,
				     cmd_id, size, wdg_timeout);
}

static inline void iwl_trans_txq_set_shared_mode(struct iwl_trans *trans,
						 int queue, bool shared_mode)
{
	if (trans->ops->txq_set_shared_mode)
		trans->ops->txq_set_shared_mode(trans, queue, shared_mode);
}

static inline void iwl_trans_txq_enable(struct iwl_trans *trans, int queue,
					int fifo, int sta_id, int tid,
					int frame_limit, u16 ssn,
					unsigned int queue_wdg_timeout)
{
	struct iwl_trans_txq_scd_cfg cfg = {
		.fifo = fifo,
		.sta_id = sta_id,
		.tid = tid,
		.frame_limit = frame_limit,
		.aggregate = sta_id >= 0,
	};

	iwl_trans_txq_enable_cfg(trans, queue, ssn, &cfg, queue_wdg_timeout);
}

static inline
void iwl_trans_ac_txq_enable(struct iwl_trans *trans, int queue, int fifo,
			     unsigned int queue_wdg_timeout)
{
	struct iwl_trans_txq_scd_cfg cfg = {
		.fifo = fifo,
		.sta_id = -1,
		.tid = IWL_MAX_TID_COUNT,
		.frame_limit = IWL_FRAME_LIMIT,
		.aggregate = false,
	};

	iwl_trans_txq_enable_cfg(trans, queue, 0, &cfg, queue_wdg_timeout);
}

static inline void iwl_trans_freeze_txq_timer(struct iwl_trans *trans,
					      unsigned long txqs,
					      bool freeze)
{
	if (WARN_ON_ONCE(trans->state != IWL_TRANS_FW_ALIVE)) {
		IWL_ERR(trans, "%s bad state = %d\n", __func__, trans->state);
		return;
	}

	if (trans->ops->freeze_txq_timer)
		trans->ops->freeze_txq_timer(trans, txqs, freeze);
}

static inline void iwl_trans_block_txq_ptrs(struct iwl_trans *trans,
					    bool block)
{
	if (WARN_ON_ONCE(trans->state != IWL_TRANS_FW_ALIVE)) {
		IWL_ERR(trans, "%s bad state = %d\n", __func__, trans->state);
		return;
	}

	if (trans->ops->block_txq_ptrs)
		trans->ops->block_txq_ptrs(trans, block);
}

static inline int iwl_trans_wait_tx_queues_empty(struct iwl_trans *trans,
						 u32 txqs)
{
	if (WARN_ON_ONCE(!trans->ops->wait_tx_queues_empty))
		return -ENOTSUPP;

	/* No need to wait if the firmware is not alive */
	if (trans->state != IWL_TRANS_FW_ALIVE) {
		IWL_ERR(trans, "%s bad state = %d\n", __func__, trans->state);
		return -EIO;
	}

	return trans->ops->wait_tx_queues_empty(trans, txqs);
}

static inline int iwl_trans_wait_txq_empty(struct iwl_trans *trans, int queue)
{
	if (WARN_ON_ONCE(!trans->ops->wait_txq_empty))
		return -ENOTSUPP;

	if (WARN_ON_ONCE(trans->state != IWL_TRANS_FW_ALIVE)) {
		IWL_ERR(trans, "%s bad state = %d\n", __func__, trans->state);
		return -EIO;
	}

	return trans->ops->wait_txq_empty(trans, queue);
}

static inline void iwl_trans_write8(struct iwl_trans *trans, u32 ofs, u8 val)
{
	trans->ops->write8(trans, ofs, val);
}

static inline void iwl_trans_write32(struct iwl_trans *trans, u32 ofs, u32 val)
{
	trans->ops->write32(trans, ofs, val);
}

static inline u32 iwl_trans_read32(struct iwl_trans *trans, u32 ofs)
{
	return trans->ops->read32(trans, ofs);
}

static inline u32 iwl_trans_read_prph(struct iwl_trans *trans, u32 ofs)
{
	return trans->ops->read_prph(trans, ofs);
}

static inline void iwl_trans_write_prph(struct iwl_trans *trans, u32 ofs,
					u32 val)
{
	return trans->ops->write_prph(trans, ofs, val);
}

static inline int iwl_trans_read_mem(struct iwl_trans *trans, u32 addr,
				     void *buf, int dwords)
{
	return trans->ops->read_mem(trans, addr, buf, dwords);
}

#define iwl_trans_read_mem_bytes(trans, addr, buf, bufsize)		      \
	do {								      \
		if (__builtin_constant_p(bufsize))			      \
			BUILD_BUG_ON((bufsize) % sizeof(u32));		      \
		iwl_trans_read_mem(trans, addr, buf, (bufsize) / sizeof(u32));\
	} while (0)

static inline u32 iwl_trans_read_mem32(struct iwl_trans *trans, u32 addr)
{
	u32 value;

	if (WARN_ON(iwl_trans_read_mem(trans, addr, &value, 1)))
		return 0xa5a5a5a5;

	return value;
}

static inline int iwl_trans_write_mem(struct iwl_trans *trans, u32 addr,
				      const void *buf, int dwords)
{
	return trans->ops->write_mem(trans, addr, buf, dwords);
}

static inline u32 iwl_trans_write_mem32(struct iwl_trans *trans, u32 addr,
					u32 val)
{
	return iwl_trans_write_mem(trans, addr, &val, 1);
}

static inline void iwl_trans_set_pmi(struct iwl_trans *trans, bool state)
{
	if (trans->ops->set_pmi)
		trans->ops->set_pmi(trans, state);
}

static inline void iwl_trans_sw_reset(struct iwl_trans *trans)
{
	if (trans->ops->sw_reset)
		trans->ops->sw_reset(trans);
}

static inline void
iwl_trans_set_bits_mask(struct iwl_trans *trans, u32 reg, u32 mask, u32 value)
{
	trans->ops->set_bits_mask(trans, reg, mask, value);
}

#define iwl_trans_grab_nic_access(trans)		\
	__cond_lock(nic_access,				\
		    likely((trans)->ops->grab_nic_access(trans)))

static inline void __releases(nic_access)
iwl_trans_release_nic_access(struct iwl_trans *trans)
{
	trans->ops->release_nic_access(trans);
	__release(nic_access);
}

static inline void iwl_trans_fw_error(struct iwl_trans *trans, bool sync)
{
	if (WARN_ON_ONCE(!trans->op_mode))
		return;

	/* prevent double restarts due to the same erroneous FW */
	if (!test_and_set_bit(STATUS_FW_ERROR, &trans->status)) {
		iwl_op_mode_nic_error(trans->op_mode, sync);
		trans->state = IWL_TRANS_NO_FW;
	}
}

static inline bool iwl_trans_fw_running(struct iwl_trans *trans)
{
	return trans->state == IWL_TRANS_FW_ALIVE;
}

static inline void iwl_trans_sync_nmi(struct iwl_trans *trans)
{
	if (trans->ops->sync_nmi)
		trans->ops->sync_nmi(trans);
}

void iwl_trans_sync_nmi_with_addr(struct iwl_trans *trans, u32 inta_addr,
				  u32 sw_err_bit);

static inline int iwl_trans_set_pnvm(struct iwl_trans *trans,
				     const void *data, u32 len)
{
	if (trans->ops->set_pnvm) {
		int ret = trans->ops->set_pnvm(trans, data, len);

		if (ret)
			return ret;
	}

	trans->pnvm_loaded = true;

	return 0;
}

static inline int iwl_trans_set_reduce_power(struct iwl_trans *trans,
					     const void *data, u32 len)
{
	if (trans->ops->set_reduce_power) {
		int ret = trans->ops->set_reduce_power(trans, data, len);

		if (ret)
			return ret;
	}

	trans->reduce_power_loaded = true;
	return 0;
}

static inline bool iwl_trans_dbg_ini_valid(struct iwl_trans *trans)
{
	return trans->dbg.internal_ini_cfg != IWL_INI_CFG_STATE_NOT_LOADED ||
		trans->dbg.external_ini_cfg != IWL_INI_CFG_STATE_NOT_LOADED;
}

static inline void iwl_trans_interrupts(struct iwl_trans *trans, bool enable)
{
	if (trans->ops->interrupts)
		trans->ops->interrupts(trans, enable);
}

/*****************************************************
 * transport helper functions
 *****************************************************/
struct iwl_trans *iwl_trans_alloc(unsigned int priv_size,
			  struct device *dev,
			  const struct iwl_trans_ops *ops,
			  const struct iwl_cfg_trans_params *cfg_trans);
int iwl_trans_init(struct iwl_trans *trans);
void iwl_trans_free(struct iwl_trans *trans);

/*****************************************************
* driver (transport) register/unregister functions
******************************************************/
int __must_check iwl_pci_register_driver(void);
void iwl_pci_unregister_driver(void);

#endif /* __iwl_trans_h__ */
struct iwl_trans_debug {
	u8 n_dest_reg;
	bool rec_on;

	const struct iwl_fw_dbg_dest_tlv_v1 *dest_tlv;
	const struct iwl_fw_dbg_conf_tlv *conf_tlv[FW_DBG_CONF_MAX];
	struct iwl_fw_dbg_trigger_tlv * const *trigger_tlv;

	u32 lmac_error_event_table[2];
	u32 umac_error_event_table;
	u32 tcm_error_event_table;
	unsigned int error_event_table_tlv_status;

	enum iwl_ini_cfg_state internal_ini_cfg;
	enum iwl_ini_cfg_state external_ini_cfg;

	struct iwl_fw_ini_allocation_tlv fw_mon_cfg[IWL_FW_INI_ALLOCATION_NUM];
	struct iwl_fw_mon fw_mon_ini[IWL_FW_INI_ALLOCATION_NUM];

	struct iwl_dram_data fw_mon;

	bool hw_error;
	enum iwl_fw_ini_buffer_location ini_dest;

	u64 unsupported_region_msk;
	struct iwl_ucode_tlv *active_regions[IWL_FW_INI_MAX_REGION_ID];
	struct list_head debug_info_tlv_list;
	struct iwl_dbg_tlv_time_point_data
		time_point[IWL_FW_INI_TIME_POINT_NUM];
	struct list_head periodic_trig_list;

	u32 domains_bitmap;
};

struct iwl_dma_ptr {
	dma_addr_t dma;
	void *addr;
	size_t size;
};

struct iwl_cmd_meta {
	/* only for SYNC commands, iff the reply skb is wanted */
	struct iwl_host_cmd *source;
	u32 flags;
	u32 tbs;
};

/*
 * The FH will write back to the first TB only, so we need to copy some data
 * into the buffer regardless of whether it should be mapped or not.
 * This indicates how big the first TB must be to include the scratch buffer
 * and the assigned PN.
 * Since PN location is 8 bytes at offset 12, it's 20 now.
 * If we make it bigger then allocations will be bigger and copy slower, so
 * that's probably not useful.
 */
#define IWL_FIRST_TB_SIZE	20
#define IWL_FIRST_TB_SIZE_ALIGN ALIGN(IWL_FIRST_TB_SIZE, 64)

struct iwl_pcie_txq_entry {
	void *cmd;
	struct sk_buff *skb;
	/* buffer to free after command completes */
	const void *free_buf;
	struct iwl_cmd_meta meta;
};

struct iwl_pcie_first_tb_buf {
	u8 buf[IWL_FIRST_TB_SIZE_ALIGN];
};

/**
 * struct iwl_txq - Tx Queue for DMA
 * @q: generic Rx/Tx queue descriptor
 * @tfds: transmit frame descriptors (DMA memory)
 * @first_tb_bufs: start of command headers, including scratch buffers, for
 *	the writeback -- this is DMA memory and an array holding one buffer
 *	for each command on the queue
 * @first_tb_dma: DMA address for the first_tb_bufs start
 * @entries: transmit entries (driver state)
 * @lock: queue lock
 * @stuck_timer: timer that fires if queue gets stuck
 * @trans: pointer back to transport (for timer)
 * @need_update: indicates need to update read/write index
 * @ampdu: true if this queue is an ampdu queue for an specific RA/TID
 * @wd_timeout: queue watchdog timeout (jiffies) - per queue
 * @frozen: tx stuck queue timer is frozen
 * @frozen_expiry_remainder: remember how long until the timer fires
 * @bc_tbl: byte count table of the queue (relevant only for gen2 transport)
 * @write_ptr: 1-st empty entry (index) host_w
 * @read_ptr: last used entry (index) host_r
 * @dma_addr:  physical addr for BD's
 * @n_window: safe queue window
 * @id: queue id
 * @low_mark: low watermark, resume queue if free space more than this
 * @high_mark: high watermark, stop queue if free space less than this
 *
 * A Tx queue consists of circular buffer of BDs (a.k.a. TFDs, transmit frame
 * descriptors) and required locking structures.
 *
 * Note the difference between TFD_QUEUE_SIZE_MAX and n_window: the hardware
 * always assumes 256 descriptors, so TFD_QUEUE_SIZE_MAX is always 256 (unless
 * there might be HW changes in the future). For the normal TX
 * queues, n_window, which is the size of the software queue data
 * is also 256; however, for the command queue, n_window is only
 * 32 since we don't need so many commands pending. Since the HW
 * still uses 256 BDs for DMA though, TFD_QUEUE_SIZE_MAX stays 256.
 * This means that we end up with the following:
 *  HW entries: | 0 | ... | N * 32 | ... | N * 32 + 31 | ... | 255 |
 *  SW entries:           | 0      | ... | 31          |
 * where N is a number between 0 and 7. This means that the SW
 * data is a window overlayed over the HW queue.
 */
struct iwl_txq {
	void *tfds;
	struct iwl_pcie_first_tb_buf *first_tb_bufs;
	dma_addr_t first_tb_dma;
	struct iwl_pcie_txq_entry *entries;
	/* lock for syncing changes on the queue */
	spinlock_t lock;
	unsigned long frozen_expiry_remainder;
	struct timer_list stuck_timer;
	struct iwl_trans *trans;
	bool need_update;
	bool frozen;
	bool ampdu;
	int block;
	unsigned long wd_timeout;
	struct sk_buff_head overflow_q;
	struct iwl_dma_ptr bc_tbl;

	int write_ptr;
	int read_ptr;
	dma_addr_t dma_addr;
	int n_window;
	u32 id;
	int low_mark;
	int high_mark;

	bool overflow_tx;
};

/**
 * struct iwl_trans_txqs - transport tx queues data
 *
 * @bc_table_dword: true if the BC table expects DWORD (as opposed to bytes)
 * @page_offs: offset from skb->cb to mac header page pointer
 * @dev_cmd_offs: offset from skb->cb to iwl_device_tx_cmd pointer
 * @queue_used - bit mask of used queues
 * @queue_stopped - bit mask of stopped queues
 * @scd_bc_tbls: gen1 pointer to the byte count table of the scheduler
 */
struct iwl_trans_txqs {
	unsigned long queue_used[BITS_TO_LONGS(IWL_MAX_TVQM_QUEUES)];
	unsigned long queue_stopped[BITS_TO_LONGS(IWL_MAX_TVQM_QUEUES)];
	struct iwl_txq *txq[IWL_MAX_TVQM_QUEUES];
	struct dma_pool *bc_pool;
	size_t bc_tbl_size;
	bool bc_table_dword;
	u8 page_offs;
	u8 dev_cmd_offs;
	struct iwl_tso_hdr_page __percpu *tso_hdr_page;

	struct {
		u8 fifo;
		u8 q_id;
		unsigned int wdg_timeout;
	} cmd;

	struct {
		u8 max_tbs;
		u16 size;
		u8 addr_size;
	} tfd;

	struct iwl_dma_ptr scd_bc_tbls;
};

/**
 * struct iwl_trans - transport common data
 *
 * @ops - pointer to iwl_trans_ops
 * @op_mode - pointer to the op_mode
 * @trans_cfg: the trans-specific configuration part
 * @cfg - pointer to the configuration
 * @drv - pointer to iwl_drv
 * @status: a bit-mask of transport status flags
 * @dev - pointer to struct device * that represents the device
 * @max_skb_frags: maximum number of fragments an SKB can have when transmitted.
 *	0 indicates that frag SKBs (NETIF_F_SG) aren't supported.
 * @hw_rf_id a u32 with the device RF ID
 * @hw_id: a u32 with the ID of the device / sub-device.
 *	Set during transport allocation.
 * @hw_id_str: a string with info about HW ID. Set during transport allocation.
 * @pm_support: set to true in start_hw if link pm is supported
 * @ltr_enabled: set to true if the LTR is enabled
 * @wide_cmd_header: true when ucode supports wide command header format
 * @wait_command_queue: wait queue for sync commands
 * @num_rx_queues: number of RX queues allocated by the transport;
 *	the transport must set this before calling iwl_drv_start()
 * @iml_len: the length of the image loader
 * @iml: a pointer to the image loader itself
 * @dev_cmd_pool: pool for Tx cmd allocation - for internal use only.
 *	The user should use iwl_trans_{alloc,free}_tx_cmd.
 * @rx_mpdu_cmd: MPDU RX command ID, must be assigned by opmode before
 *	starting the firmware, used for tracing
 * @rx_mpdu_cmd_hdr_size: used for tracing, amount of data before the
 *	start of the 802.11 header in the @rx_mpdu_cmd
 * @dflt_pwr_limit: default power limit fetched from the platform (ACPI)
 * @system_pm_mode: the system-wide power management mode in use.
 *	This mode is set dynamically, depending on the WoWLAN values
 *	configured from the userspace at runtime.
 * @iwl_trans_txqs: transport tx queues data.
 */
struct iwl_trans {
	const struct iwl_trans_ops *ops;
	struct iwl_op_mode *op_mode;
	const struct iwl_cfg_trans_params *trans_cfg;
	const struct iwl_cfg *cfg;
	struct iwl_drv *drv;
	enum iwl_trans_state state;
	unsigned long status;

	struct device *dev;
	u32 max_skb_frags;
	u32 hw_rev;
	u32 hw_rf_id;
	u32 hw_id;
	char hw_id_str[52];
	u32 sku_id[3];

	u8 rx_mpdu_cmd, rx_mpdu_cmd_hdr_size;

	bool pm_support;
	bool ltr_enabled;
	u8 pnvm_loaded:1;
	u8 reduce_power_loaded:1;

	const struct iwl_hcmd_arr *command_groups;
	int command_groups_size;
	bool wide_cmd_header;

	wait_queue_head_t wait_command_queue;
	u8 num_rx_queues;

	size_t iml_len;
	u8 *iml;

	/* The following fields are internal only */
	struct kmem_cache *dev_cmd_pool;
	char dev_cmd_pool_name[50];

	struct dentry *dbgfs_dir;

#ifdef CONFIG_LOCKDEP
	struct lockdep_map sync_cmd_lockdep_map;
#endif

	struct iwl_trans_debug dbg;
	struct iwl_self_init_dram init_dram;

	enum iwl_plat_pm_mode system_pm_mode;

	const char *name;
	struct iwl_trans_txqs txqs;

	/* pointer to trans specific struct */
	/*Ensure that this pointer will always be aligned to sizeof pointer */
	char trans_specific[] __aligned(sizeof(void *));
};

const char *iwl_get_cmd_string(struct iwl_trans *trans, u32 id);
int iwl_cmd_groups_verify_sorted(const struct iwl_trans_config *trans);

static inline void iwl_trans_configure(struct iwl_trans *trans,
				       const struct iwl_trans_config *trans_cfg)
{
	trans->op_mode = trans_cfg->op_mode;

	trans->ops->configure(trans, trans_cfg);
	WARN_ON(iwl_cmd_groups_verify_sorted(trans_cfg));
}

static inline int iwl_trans_start_hw(struct iwl_trans *trans)
{
	might_sleep();

	return trans->ops->start_hw(trans);
}

static inline void iwl_trans_op_mode_leave(struct iwl_trans *trans)
{
	might_sleep();

	if (trans->ops->op_mode_leave)
		trans->ops->op_mode_leave(trans);

	trans->op_mode = NULL;

	trans->state = IWL_TRANS_NO_FW;
}

static inline void iwl_trans_fw_alive(struct iwl_trans *trans, u32 scd_addr)
{
	might_sleep();

	trans->state = IWL_TRANS_FW_ALIVE;

	trans->ops->fw_alive(trans, scd_addr);
}

static inline int iwl_trans_start_fw(struct iwl_trans *trans,
				     const struct fw_img *fw,
				     bool run_in_rfkill)
{
	int ret;

	might_sleep();

	WARN_ON_ONCE(!trans->rx_mpdu_cmd);

	clear_bit(STATUS_FW_ERROR, &trans->status);
	ret = trans->ops->start_fw(trans, fw, run_in_rfkill);
	if (ret == 0)
		trans->state = IWL_TRANS_FW_STARTED;

	return ret;
}

static inline void iwl_trans_stop_device(struct iwl_trans *trans)
{
	might_sleep();

	trans->ops->stop_device(trans);

	trans->state = IWL_TRANS_NO_FW;
}

static inline int iwl_trans_d3_suspend(struct iwl_trans *trans, bool test,
				       bool reset)
{
	might_sleep();
	if (!trans->ops->d3_suspend)
		return 0;

	return trans->ops->d3_suspend(trans, test, reset);
}

static inline int iwl_trans_d3_resume(struct iwl_trans *trans,
				      enum iwl_d3_status *status,
				      bool test, bool reset)
{
	might_sleep();
	if (!trans->ops->d3_resume)
		return 0;

	return trans->ops->d3_resume(trans, status, test, reset);
}

static inline struct iwl_trans_dump_data *
iwl_trans_dump_data(struct iwl_trans *trans, u32 dump_mask)
{
	if (!trans->ops->dump_data)
		return NULL;
	return trans->ops->dump_data(trans, dump_mask);
}

static inline struct iwl_device_tx_cmd *
iwl_trans_alloc_tx_cmd(struct iwl_trans *trans)
{
	return kmem_cache_zalloc(trans->dev_cmd_pool, GFP_ATOMIC);
}

int iwl_trans_send_cmd(struct iwl_trans *trans, struct iwl_host_cmd *cmd);

static inline void iwl_trans_free_tx_cmd(struct iwl_trans *trans,
					 struct iwl_device_tx_cmd *dev_cmd)
{
	kmem_cache_free(trans->dev_cmd_pool, dev_cmd);
}

static inline int iwl_trans_tx(struct iwl_trans *trans, struct sk_buff *skb,
			       struct iwl_device_tx_cmd *dev_cmd, int queue)
{
	if (unlikely(test_bit(STATUS_FW_ERROR, &trans->status)))
		return -EIO;

	if (WARN_ON_ONCE(trans->state != IWL_TRANS_FW_ALIVE)) {
		IWL_ERR(trans, "%s bad state = %d\n", __func__, trans->state);
		return -EIO;
	}

	return trans->ops->tx(trans, skb, dev_cmd, queue);
}

static inline void iwl_trans_reclaim(struct iwl_trans *trans, int queue,
				     int ssn, struct sk_buff_head *skbs)
{
	if (WARN_ON_ONCE(trans->state != IWL_TRANS_FW_ALIVE)) {
		IWL_ERR(trans, "%s bad state = %d\n", __func__, trans->state);
		return;
	}

	trans->ops->reclaim(trans, queue, ssn, skbs);
}

static inline void iwl_trans_set_q_ptrs(struct iwl_trans *trans, int queue,
					int ptr)
{
	if (WARN_ON_ONCE(trans->state != IWL_TRANS_FW_ALIVE)) {
		IWL_ERR(trans, "%s bad state = %d\n", __func__, trans->state);
		return;
	}

	trans->ops->set_q_ptrs(trans, queue, ptr);
}

static inline void iwl_trans_txq_disable(struct iwl_trans *trans, int queue,
					 bool configure_scd)
{
	trans->ops->txq_disable(trans, queue, configure_scd);
}

static inline bool
iwl_trans_txq_enable_cfg(struct iwl_trans *trans, int queue, u16 ssn,
			 const struct iwl_trans_txq_scd_cfg *cfg,
			 unsigned int queue_wdg_timeout)
{
	might_sleep();

	if (WARN_ON_ONCE(trans->state != IWL_TRANS_FW_ALIVE)) {
		IWL_ERR(trans, "%s bad state = %d\n", __func__, trans->state);
		return false;
	}

	return trans->ops->txq_enable(trans, queue, ssn,
				      cfg, queue_wdg_timeout);
}

static inline int
iwl_trans_get_rxq_dma_data(struct iwl_trans *trans, int queue,
			   struct iwl_trans_rxq_dma_data *data)
{
	if (WARN_ON_ONCE(!trans->ops->rxq_dma_data))
		return -ENOTSUPP;

	return trans->ops->rxq_dma_data(trans, queue, data);
}

static inline void
iwl_trans_txq_free(struct iwl_trans *trans, int queue)
{
	if (WARN_ON_ONCE(!trans->ops->txq_free))
		return;

	trans->ops->txq_free(trans, queue);
}

static inline int
iwl_trans_txq_alloc(struct iwl_trans *trans,
		    __le16 flags, u8 sta_id, u8 tid,
		    int cmd_id, int size,
		    unsigned int wdg_timeout)
{
	might_sleep();

	if (WARN_ON_ONCE(!trans->ops->txq_alloc))
		return -ENOTSUPP;

	if (WARN_ON_ONCE(trans->state != IWL_TRANS_FW_ALIVE)) {
		IWL_ERR(trans, "%s bad state = %d\n", __func__, trans->state);
		return -EIO;
	}

	return trans->ops->txq_alloc(trans, flags, sta_id, tid,
				     cmd_id, size, wdg_timeout);
}

static inline void iwl_trans_txq_set_shared_mode(struct iwl_trans *trans,
						 int queue, bool shared_mode)
{
	if (trans->ops->txq_set_shared_mode)
		trans->ops->txq_set_shared_mode(trans, queue, shared_mode);
}

static inline void iwl_trans_txq_enable(struct iwl_trans *trans, int queue,
					int fifo, int sta_id, int tid,
					int frame_limit, u16 ssn,
					unsigned int queue_wdg_timeout)
{
	struct iwl_trans_txq_scd_cfg cfg = {
		.fifo = fifo,
		.sta_id = sta_id,
		.tid = tid,
		.frame_limit = frame_limit,
		.aggregate = sta_id >= 0,
	};

	iwl_trans_txq_enable_cfg(trans, queue, ssn, &cfg, queue_wdg_timeout);
}

static inline
void iwl_trans_ac_txq_enable(struct iwl_trans *trans, int queue, int fifo,
			     unsigned int queue_wdg_timeout)
{
	struct iwl_trans_txq_scd_cfg cfg = {
		.fifo = fifo,
		.sta_id = -1,
		.tid = IWL_MAX_TID_COUNT,
		.frame_limit = IWL_FRAME_LIMIT,
		.aggregate = false,
	};

	iwl_trans_txq_enable_cfg(trans, queue, 0, &cfg, queue_wdg_timeout);
}

static inline void iwl_trans_freeze_txq_timer(struct iwl_trans *trans,
					      unsigned long txqs,
					      bool freeze)
{
	if (WARN_ON_ONCE(trans->state != IWL_TRANS_FW_ALIVE)) {
		IWL_ERR(trans, "%s bad state = %d\n", __func__, trans->state);
		return;
	}

	if (trans->ops->freeze_txq_timer)
		trans->ops->freeze_txq_timer(trans, txqs, freeze);
}

static inline void iwl_trans_block_txq_ptrs(struct iwl_trans *trans,
					    bool block)
{
	if (WARN_ON_ONCE(trans->state != IWL_TRANS_FW_ALIVE)) {
		IWL_ERR(trans, "%s bad state = %d\n", __func__, trans->state);
		return;
	}

	if (trans->ops->block_txq_ptrs)
		trans->ops->block_txq_ptrs(trans, block);
}

static inline int iwl_trans_wait_tx_queues_empty(struct iwl_trans *trans,
						 u32 txqs)
{
	if (WARN_ON_ONCE(!trans->ops->wait_tx_queues_empty))
		return -ENOTSUPP;

	/* No need to wait if the firmware is not alive */
	if (trans->state != IWL_TRANS_FW_ALIVE) {
		IWL_ERR(trans, "%s bad state = %d\n", __func__, trans->state);
		return -EIO;
	}

	return trans->ops->wait_tx_queues_empty(trans, txqs);
}

static inline int iwl_trans_wait_txq_empty(struct iwl_trans *trans, int queue)
{
	if (WARN_ON_ONCE(!trans->ops->wait_txq_empty))
		return -ENOTSUPP;

	if (WARN_ON_ONCE(trans->state != IWL_TRANS_FW_ALIVE)) {
		IWL_ERR(trans, "%s bad state = %d\n", __func__, trans->state);
		return -EIO;
	}

	return trans->ops->wait_txq_empty(trans, queue);
}

static inline void iwl_trans_write8(struct iwl_trans *trans, u32 ofs, u8 val)
{
	trans->ops->write8(trans, ofs, val);
}

static inline void iwl_trans_write32(struct iwl_trans *trans, u32 ofs, u32 val)
{
	trans->ops->write32(trans, ofs, val);
}

static inline u32 iwl_trans_read32(struct iwl_trans *trans, u32 ofs)
{
	return trans->ops->read32(trans, ofs);
}

static inline u32 iwl_trans_read_prph(struct iwl_trans *trans, u32 ofs)
{
	return trans->ops->read_prph(trans, ofs);
}

static inline void iwl_trans_write_prph(struct iwl_trans *trans, u32 ofs,
					u32 val)
{
	return trans->ops->write_prph(trans, ofs, val);
}

static inline int iwl_trans_read_mem(struct iwl_trans *trans, u32 addr,
				     void *buf, int dwords)
{
	return trans->ops->read_mem(trans, addr, buf, dwords);
}

#define iwl_trans_read_mem_bytes(trans, addr, buf, bufsize)		      \
	do {								      \
		if (__builtin_constant_p(bufsize))			      \
			BUILD_BUG_ON((bufsize) % sizeof(u32));		      \
		iwl_trans_read_mem(trans, addr, buf, (bufsize) / sizeof(u32));\
	} while (0)

static inline u32 iwl_trans_read_mem32(struct iwl_trans *trans, u32 addr)
{
	u32 value;

	if (WARN_ON(iwl_trans_read_mem(trans, addr, &value, 1)))
		return 0xa5a5a5a5;

	return value;
}

static inline int iwl_trans_write_mem(struct iwl_trans *trans, u32 addr,
				      const void *buf, int dwords)
{
	return trans->ops->write_mem(trans, addr, buf, dwords);
}

static inline u32 iwl_trans_write_mem32(struct iwl_trans *trans, u32 addr,
					u32 val)
{
	return iwl_trans_write_mem(trans, addr, &val, 1);
}

static inline void iwl_trans_set_pmi(struct iwl_trans *trans, bool state)
{
	if (trans->ops->set_pmi)
		trans->ops->set_pmi(trans, state);
}

static inline void iwl_trans_sw_reset(struct iwl_trans *trans)
{
	if (trans->ops->sw_reset)
		trans->ops->sw_reset(trans);
}

static inline void
iwl_trans_set_bits_mask(struct iwl_trans *trans, u32 reg, u32 mask, u32 value)
{
	trans->ops->set_bits_mask(trans, reg, mask, value);
}

#define iwl_trans_grab_nic_access(trans)		\
	__cond_lock(nic_access,				\
		    likely((trans)->ops->grab_nic_access(trans)))

static inline void __releases(nic_access)
iwl_trans_release_nic_access(struct iwl_trans *trans)
{
	trans->ops->release_nic_access(trans);
	__release(nic_access);
}

static inline void iwl_trans_fw_error(struct iwl_trans *trans, bool sync)
{
	if (WARN_ON_ONCE(!trans->op_mode))
		return;

	/* prevent double restarts due to the same erroneous FW */
	if (!test_and_set_bit(STATUS_FW_ERROR, &trans->status)) {
		iwl_op_mode_nic_error(trans->op_mode, sync);
		trans->state = IWL_TRANS_NO_FW;
	}
}

static inline bool iwl_trans_fw_running(struct iwl_trans *trans)
{
	return trans->state == IWL_TRANS_FW_ALIVE;
}

static inline void iwl_trans_sync_nmi(struct iwl_trans *trans)
{
	if (trans->ops->sync_nmi)
		trans->ops->sync_nmi(trans);
}

void iwl_trans_sync_nmi_with_addr(struct iwl_trans *trans, u32 inta_addr,
				  u32 sw_err_bit);

static inline int iwl_trans_set_pnvm(struct iwl_trans *trans,
				     const void *data, u32 len)
{
	if (trans->ops->set_pnvm) {
		int ret = trans->ops->set_pnvm(trans, data, len);

		if (ret)
			return ret;
	}

	trans->pnvm_loaded = true;

	return 0;
}

static inline int iwl_trans_set_reduce_power(struct iwl_trans *trans,
					     const void *data, u32 len)
{
	if (trans->ops->set_reduce_power) {
		int ret = trans->ops->set_reduce_power(trans, data, len);

		if (ret)
			return ret;
	}

	trans->reduce_power_loaded = true;
	return 0;
}

static inline bool iwl_trans_dbg_ini_valid(struct iwl_trans *trans)
{
	return trans->dbg.internal_ini_cfg != IWL_INI_CFG_STATE_NOT_LOADED ||
		trans->dbg.external_ini_cfg != IWL_INI_CFG_STATE_NOT_LOADED;
}

static inline void iwl_trans_interrupts(struct iwl_trans *trans, bool enable)
{
	if (trans->ops->interrupts)
		trans->ops->interrupts(trans, enable);
}

/*****************************************************
 * transport helper functions
 *****************************************************/
struct iwl_trans *iwl_trans_alloc(unsigned int priv_size,
			  struct device *dev,
			  const struct iwl_trans_ops *ops,
			  const struct iwl_cfg_trans_params *cfg_trans);
int iwl_trans_init(struct iwl_trans *trans);
void iwl_trans_free(struct iwl_trans *trans);

/*****************************************************
* driver (transport) register/unregister functions
******************************************************/
int __must_check iwl_pci_register_driver(void);
void iwl_pci_unregister_driver(void);

#endif /* __iwl_trans_h__ */
{
	might_sleep();
	if (!trans->ops->d3_resume)
		return 0;

	return trans->ops->d3_resume(trans, status, test, reset);
}

static inline struct iwl_trans_dump_data *
iwl_trans_dump_data(struct iwl_trans *trans, u32 dump_mask)
{
	if (!trans->ops->dump_data)
		return NULL;
	return trans->ops->dump_data(trans, dump_mask);
}
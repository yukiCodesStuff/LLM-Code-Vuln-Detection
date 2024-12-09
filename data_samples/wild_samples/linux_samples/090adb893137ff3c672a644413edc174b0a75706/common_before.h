{
}
#endif

/*****************************************************
 * Handlers
 ***************************************************/
void il_hdl_pm_sleep(struct il_priv *il, struct il_rx_buf *rxb);
void il_hdl_pm_debug_stats(struct il_priv *il, struct il_rx_buf *rxb);
void il_hdl_error(struct il_priv *il, struct il_rx_buf *rxb);
void il_hdl_csa(struct il_priv *il, struct il_rx_buf *rxb);

/*****************************************************
* RX
******************************************************/
void il_cmd_queue_unmap(struct il_priv *il);
void il_cmd_queue_free(struct il_priv *il);
int il_rx_queue_alloc(struct il_priv *il);
void il_rx_queue_update_write_ptr(struct il_priv *il, struct il_rx_queue *q);
int il_rx_queue_space(const struct il_rx_queue *q);
void il_tx_cmd_complete(struct il_priv *il, struct il_rx_buf *rxb);

void il_hdl_spectrum_measurement(struct il_priv *il, struct il_rx_buf *rxb);
void il_recover_from_stats(struct il_priv *il, struct il_rx_pkt *pkt);
void il_chswitch_done(struct il_priv *il, bool is_success);

/*****************************************************
* TX
******************************************************/
extern void il_txq_update_write_ptr(struct il_priv *il, struct il_tx_queue *txq);
extern int il_tx_queue_init(struct il_priv *il, u32 txq_id);
extern void il_tx_queue_reset(struct il_priv *il, u32 txq_id);
extern void il_tx_queue_unmap(struct il_priv *il, int txq_id);
extern void il_tx_queue_free(struct il_priv *il, int txq_id);
extern void il_setup_watchdog(struct il_priv *il);
/*****************************************************
 * TX power
 ****************************************************/
int il_set_tx_power(struct il_priv *il, s8 tx_power, bool force);

/*******************************************************************************
 * Rate
 ******************************************************************************/

u8 il_get_lowest_plcp(struct il_priv *il);

/*******************************************************************************
 * Scanning
 ******************************************************************************/
void il_init_scan_params(struct il_priv *il);
int il_scan_cancel(struct il_priv *il);
int il_scan_cancel_timeout(struct il_priv *il, unsigned long ms);
void il_force_scan_end(struct il_priv *il);
int il_mac_hw_scan(struct ieee80211_hw *hw, struct ieee80211_vif *vif,
		   struct cfg80211_scan_request *req);
void il_internal_short_hw_scan(struct il_priv *il);
int il_force_reset(struct il_priv *il, bool external);
u16 il_fill_probe_req(struct il_priv *il, struct ieee80211_mgmt *frame,
		      const u8 *ta, const u8 *ie, int ie_len, int left);
void il_setup_rx_scan_handlers(struct il_priv *il);
u16 il_get_active_dwell_time(struct il_priv *il, enum ieee80211_band band,
			     u8 n_probes);
u16 il_get_passive_dwell_time(struct il_priv *il, enum ieee80211_band band,
			      struct ieee80211_vif *vif);
void il_setup_scan_deferred_work(struct il_priv *il);
void il_cancel_scan_deferred_work(struct il_priv *il);

/* For faster active scanning, scan will move to the next channel if fewer than
 * PLCP_QUIET_THRESH packets are heard on this channel within
 * ACTIVE_QUIET_TIME after sending probe request.  This shortens the dwell
 * time if it's a quiet channel (nothing responded to our probe, and there's
 * no other traffic).
 * Disable "quiet" feature by setting PLCP_QUIET_THRESH to 0. */
#define IL_ACTIVE_QUIET_TIME       cpu_to_le16(10)	/* msec */
#define IL_PLCP_QUIET_THRESH       cpu_to_le16(1)	/* packets */

#define IL_SCAN_CHECK_WATCHDOG		(HZ * 7)

/*****************************************************
 *   S e n d i n g     H o s t     C o m m a n d s   *
 *****************************************************/

const char *il_get_cmd_string(u8 cmd);
int __must_check il_send_cmd_sync(struct il_priv *il, struct il_host_cmd *cmd);
int il_send_cmd(struct il_priv *il, struct il_host_cmd *cmd);
int __must_check il_send_cmd_pdu(struct il_priv *il, u8 id, u16 len,
				 const void *data);
int il_send_cmd_pdu_async(struct il_priv *il, u8 id, u16 len, const void *data,
			  void (*callback) (struct il_priv *il,
					    struct il_device_cmd *cmd,
					    struct il_rx_pkt *pkt));

int il_enqueue_hcmd(struct il_priv *il, struct il_host_cmd *cmd);

/*****************************************************
 * PCI						     *
 *****************************************************/

void il_bg_watchdog(unsigned long data);
u32 il_usecs_to_beacons(struct il_priv *il, u32 usec, u32 beacon_interval);
__le32 il_add_beacon_time(struct il_priv *il, u32 base, u32 addon,
			  u32 beacon_interval);

#ifdef CONFIG_PM
extern const struct dev_pm_ops il_pm_ops;

#define IL_LEGACY_PM_OPS	(&il_pm_ops)

#else /* !CONFIG_PM */

#define IL_LEGACY_PM_OPS	NULL

#endif /* !CONFIG_PM */

/*****************************************************
*  Error Handling Debugging
******************************************************/
void il4965_dump_nic_error_log(struct il_priv *il);
#ifdef CONFIG_IWLEGACY_DEBUG
void il_print_rx_config_cmd(struct il_priv *il);
#else
static inline void
il_print_rx_config_cmd(struct il_priv *il)
{
}
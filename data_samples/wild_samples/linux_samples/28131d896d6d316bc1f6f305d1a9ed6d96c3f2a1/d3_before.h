struct iwl_wowlan_status_v6 {
	struct iwl_wowlan_gtk_status_v1 gtk;
	__le64 replay_ctr;
	__le16 pattern_number;
	__le16 non_qos_seq_ctr;
	__le16 qos_seq_ctr[8];
	__le32 wakeup_reasons;
	__le32 num_of_gtk_rekeys;
	__le32 transmitted_ndps;
	__le32 received_beacons;
	__le32 wake_packet_length;
	__le32 wake_packet_bufsize;
	u8 wake_packet[]; /* can be truncated from _length to _bufsize */
} __packed; /* WOWLAN_STATUSES_API_S_VER_6 */

/**
 * struct iwl_wowlan_status - WoWLAN status
 * @gtk: GTK data
 * @igtk: IGTK data
 * @replay_ctr: GTK rekey replay counter
 * @pattern_number: number of the matched pattern
 * @non_qos_seq_ctr: non-QoS sequence counter to use next
 * @qos_seq_ctr: QoS sequence counters to use next
 * @wakeup_reasons: wakeup reasons, see &enum iwl_wowlan_wakeup_reason
 * @num_of_gtk_rekeys: number of GTK rekeys
 * @transmitted_ndps: number of transmitted neighbor discovery packets
 * @received_beacons: number of received beacons
 * @wake_packet_length: wakeup packet length
 * @wake_packet_bufsize: wakeup packet buffer size
 * @wake_packet: wakeup packet
 */
struct iwl_wowlan_status_v7 {
	struct iwl_wowlan_gtk_status gtk[WOWLAN_GTK_KEYS_NUM];
	struct iwl_wowlan_igtk_status igtk[WOWLAN_IGTK_KEYS_NUM];
	__le64 replay_ctr;
	__le16 pattern_number;
	__le16 non_qos_seq_ctr;
	__le16 qos_seq_ctr[8];
	__le32 wakeup_reasons;
	__le32 num_of_gtk_rekeys;
	__le32 transmitted_ndps;
	__le32 received_beacons;
	__le32 wake_packet_length;
	__le32 wake_packet_bufsize;
	u8 wake_packet[]; /* can be truncated from _length to _bufsize */
} __packed; /* WOWLAN_STATUSES_API_S_VER_7 */

/**
 * struct iwl_wowlan_status_v9 - WoWLAN status (versions 9 and 10)
 * @gtk: GTK data
 * @igtk: IGTK data
 * @replay_ctr: GTK rekey replay counter
 * @pattern_number: number of the matched pattern
 * @non_qos_seq_ctr: non-QoS sequence counter to use next.
 *                   Reserved if the struct has version >= 10.
 * @qos_seq_ctr: QoS sequence counters to use next
 * @wakeup_reasons: wakeup reasons, see &enum iwl_wowlan_wakeup_reason
 * @num_of_gtk_rekeys: number of GTK rekeys
 * @transmitted_ndps: number of transmitted neighbor discovery packets
 * @received_beacons: number of received beacons
 * @wake_packet_length: wakeup packet length
 * @wake_packet_bufsize: wakeup packet buffer size
 * @tid_tear_down: bit mask of tids whose BA sessions were closed
 *		   in suspend state
 * @reserved: unused
 * @wake_packet: wakeup packet
 */
struct iwl_wowlan_status_v9 {
	struct iwl_wowlan_gtk_status gtk[WOWLAN_GTK_KEYS_NUM];
	struct iwl_wowlan_igtk_status igtk[WOWLAN_IGTK_KEYS_NUM];
	__le64 replay_ctr;
	__le16 pattern_number;
	__le16 non_qos_seq_ctr;
	__le16 qos_seq_ctr[8];
	__le32 wakeup_reasons;
	__le32 num_of_gtk_rekeys;
	__le32 transmitted_ndps;
	__le32 received_beacons;
	__le32 wake_packet_length;
	__le32 wake_packet_bufsize;
	u8 tid_tear_down;
	u8 reserved[3];
	u8 wake_packet[]; /* can be truncated from _length to _bufsize */
} __packed; /* WOWLAN_STATUSES_RSP_API_S_VER_9 */

/**
 * struct iwl_wowlan_status - WoWLAN status
 * @gtk: GTK data
 * @igtk: IGTK data
 * @bigtk: BIGTK data
 * @replay_ctr: GTK rekey replay counter
 * @pattern_number: number of the matched pattern
 * @non_qos_seq_ctr: non-QoS sequence counter to use next
 * @qos_seq_ctr: QoS sequence counters to use next
 * @wakeup_reasons: wakeup reasons, see &enum iwl_wowlan_wakeup_reason
 * @num_of_gtk_rekeys: number of GTK rekeys
 * @tid_tear_down: bitmap of TIDs torn down
 * @reserved: reserved
 * @received_beacons: number of received beacons
 * @wake_packet_length: wakeup packet length
 * @wake_packet_bufsize: wakeup packet buffer size
 * @tid_tear_down: bit mask of tids whose BA sessions were closed
 *		   in suspend state
 * @wake_packet: wakeup packet
 */
struct iwl_wowlan_status {
	struct iwl_wowlan_gtk_status gtk[1];
	struct iwl_wowlan_igtk_status igtk[1];
	struct iwl_wowlan_igtk_status bigtk[WOWLAN_IGTK_KEYS_NUM];
	__le64 replay_ctr;
	__le16 pattern_number;
	__le16 non_qos_seq_ctr;
	__le16 qos_seq_ctr[8];
	__le32 wakeup_reasons;
	__le32 num_of_gtk_rekeys;
	u8 tid_tear_down;
	u8 reserved[3];
	__le32 received_beacons;
	__le32 wake_packet_length;
	__le32 wake_packet_bufsize;
	u8 wake_packet[]; /* can be truncated from _length to _bufsize */
} __packed; /* WOWLAN_STATUSES_API_S_VER_11 */

static inline u8 iwlmvm_wowlan_gtk_idx(struct iwl_wowlan_gtk_status *gtk)
{
	return gtk->key_flags & IWL_WOWLAN_GTK_IDX_MASK;
}

/* TODO: NetDetect API */

#endif /* __iwl_fw_api_d3_h__ */
struct iwl_wowlan_status_v9 {
	struct iwl_wowlan_gtk_status gtk[WOWLAN_GTK_KEYS_NUM];
	struct iwl_wowlan_igtk_status igtk[WOWLAN_IGTK_KEYS_NUM];
	__le64 replay_ctr;
	__le16 pattern_number;
	__le16 non_qos_seq_ctr;
	__le16 qos_seq_ctr[8];
	__le32 wakeup_reasons;
	__le32 num_of_gtk_rekeys;
	__le32 transmitted_ndps;
	__le32 received_beacons;
	__le32 wake_packet_length;
	__le32 wake_packet_bufsize;
	u8 tid_tear_down;
	u8 reserved[3];
	u8 wake_packet[]; /* can be truncated from _length to _bufsize */
} __packed; /* WOWLAN_STATUSES_RSP_API_S_VER_9 */

/**
 * struct iwl_wowlan_status - WoWLAN status
 * @gtk: GTK data
 * @igtk: IGTK data
 * @bigtk: BIGTK data
 * @replay_ctr: GTK rekey replay counter
 * @pattern_number: number of the matched pattern
 * @non_qos_seq_ctr: non-QoS sequence counter to use next
 * @qos_seq_ctr: QoS sequence counters to use next
 * @wakeup_reasons: wakeup reasons, see &enum iwl_wowlan_wakeup_reason
 * @num_of_gtk_rekeys: number of GTK rekeys
 * @tid_tear_down: bitmap of TIDs torn down
 * @reserved: reserved
 * @received_beacons: number of received beacons
 * @wake_packet_length: wakeup packet length
 * @wake_packet_bufsize: wakeup packet buffer size
 * @tid_tear_down: bit mask of tids whose BA sessions were closed
 *		   in suspend state
 * @wake_packet: wakeup packet
 */
struct iwl_wowlan_status {
	struct iwl_wowlan_gtk_status gtk[1];
	struct iwl_wowlan_igtk_status igtk[1];
	struct iwl_wowlan_igtk_status bigtk[WOWLAN_IGTK_KEYS_NUM];
	__le64 replay_ctr;
	__le16 pattern_number;
	__le16 non_qos_seq_ctr;
	__le16 qos_seq_ctr[8];
	__le32 wakeup_reasons;
	__le32 num_of_gtk_rekeys;
	u8 tid_tear_down;
	u8 reserved[3];
	__le32 received_beacons;
	__le32 wake_packet_length;
	__le32 wake_packet_bufsize;
	u8 wake_packet[]; /* can be truncated from _length to _bufsize */
} __packed; /* WOWLAN_STATUSES_API_S_VER_11 */

static inline u8 iwlmvm_wowlan_gtk_idx(struct iwl_wowlan_gtk_status *gtk)
{
	return gtk->key_flags & IWL_WOWLAN_GTK_IDX_MASK;
}

/* TODO: NetDetect API */

#endif /* __iwl_fw_api_d3_h__ */
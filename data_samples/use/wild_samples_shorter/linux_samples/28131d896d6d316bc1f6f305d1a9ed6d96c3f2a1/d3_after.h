} __packed; /* WOWLAN_STATUSES_API_S_VER_6 */

/**
 * struct iwl_wowlan_status_v7 - WoWLAN status
 * @gtk: GTK data
 * @igtk: IGTK data
 * @replay_ctr: GTK rekey replay counter
 * @pattern_number: number of the matched pattern
	u8 wake_packet[]; /* can be truncated from _length to _bufsize */
} __packed; /* WOWLAN_STATUSES_RSP_API_S_VER_9 */

/* TODO: NetDetect API */

#endif /* __iwl_fw_api_d3_h__ */
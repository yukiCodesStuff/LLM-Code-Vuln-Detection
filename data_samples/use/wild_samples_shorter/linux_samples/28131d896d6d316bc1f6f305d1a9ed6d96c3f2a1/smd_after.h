int wcn36xx_smd_start_hw_scan(struct wcn36xx *wcn, struct ieee80211_vif *vif,
			      struct cfg80211_scan_request *req);
int wcn36xx_smd_stop_hw_scan(struct wcn36xx *wcn);
int wcn36xx_smd_update_channel_list(struct wcn36xx *wcn, struct cfg80211_scan_request *req);
int wcn36xx_smd_add_sta_self(struct wcn36xx *wcn, struct ieee80211_vif *vif);
int wcn36xx_smd_delete_sta_self(struct wcn36xx *wcn, u8 *addr);
int wcn36xx_smd_delete_sta(struct wcn36xx *wcn, u8 sta_index);
int wcn36xx_smd_join(struct wcn36xx *wcn, const u8 *bssid, u8 *vif, u8 ch);
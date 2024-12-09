{
	u8 *ie, *new_ie, *pos;
	const struct element *nontrans_ssid;
	const u8 *trans_ssid, *mbssid;
	size_t ielen = len - offsetof(struct ieee80211_mgmt,
				      u.probe_resp.variable);
	size_t new_ie_len;
	struct cfg80211_bss_ies *new_ies;
	const struct cfg80211_bss_ies *old;
	u8 cpy_len;

	lockdep_assert_held(&wiphy_to_rdev(wiphy)->bss_lock);

	ie = mgmt->u.probe_resp.variable;

	new_ie_len = ielen;
	trans_ssid = cfg80211_find_ie(WLAN_EID_SSID, ie, ielen);
	if (!trans_ssid)
		return;
	new_ie_len -= trans_ssid[1];
	mbssid = cfg80211_find_ie(WLAN_EID_MULTIPLE_BSSID, ie, ielen);
	/*
	 * It's not valid to have the MBSSID element before SSID
	 * ignore if that happens - the code below assumes it is
	 * after (while copying things inbetween).
	 */
	if (!mbssid || mbssid < trans_ssid)
		return;
	new_ie_len -= mbssid[1];

	nontrans_ssid = ieee80211_bss_get_elem(nontrans_bss, WLAN_EID_SSID);
	if (!nontrans_ssid)
		return;

	new_ie_len += nontrans_ssid->datalen;

	/* generate new ie for nontrans BSS
	 * 1. replace SSID with nontrans BSS' SSID
	 * 2. skip MBSSID IE
	 */
	new_ie = kzalloc(new_ie_len, GFP_ATOMIC);
	if (!new_ie)
		return;

	new_ies = kzalloc(sizeof(*new_ies) + new_ie_len, GFP_ATOMIC);
	if (!new_ies)
		goto out_free;

	pos = new_ie;

	/* copy the nontransmitted SSID */
	cpy_len = nontrans_ssid->datalen + 2;
	memcpy(pos, nontrans_ssid, cpy_len);
	pos += cpy_len;
	/* copy the IEs between SSID and MBSSID */
	cpy_len = trans_ssid[1] + 2;
	memcpy(pos, (trans_ssid + cpy_len), (mbssid - (trans_ssid + cpy_len)));
	pos += (mbssid - (trans_ssid + cpy_len));
	/* copy the IEs after MBSSID */
	cpy_len = mbssid[1] + 2;
	memcpy(pos, mbssid + cpy_len, ((ie + ielen) - (mbssid + cpy_len)));

	/* update ie */
	new_ies->len = new_ie_len;
	new_ies->tsf = le64_to_cpu(mgmt->u.probe_resp.timestamp);
	new_ies->from_beacon = ieee80211_is_beacon(mgmt->frame_control);
	memcpy(new_ies->data, new_ie, new_ie_len);
	if (ieee80211_is_probe_resp(mgmt->frame_control)) {
		old = rcu_access_pointer(nontrans_bss->proberesp_ies);
		rcu_assign_pointer(nontrans_bss->proberesp_ies, new_ies);
		rcu_assign_pointer(nontrans_bss->ies, new_ies);
		if (old)
			kfree_rcu((struct cfg80211_bss_ies *)old, rcu_head);
	} else {
		old = rcu_access_pointer(nontrans_bss->beacon_ies);
		rcu_assign_pointer(nontrans_bss->beacon_ies, new_ies);
		rcu_assign_pointer(nontrans_bss->ies, new_ies);
		if (old)
			kfree_rcu((struct cfg80211_bss_ies *)old, rcu_head);
	}

out_free:
	kfree(new_ie);
}
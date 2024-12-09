	case NL80211_IFTYPE_AP:
	case NL80211_IFTYPE_ADHOC:
		/*
		 * Handle legacy hostapd as well, where station will be added
		 * only just before sending the association response.
		 * Also take care of the case where we send a deauth to a
		 * station that we don't have, or similarly an association
		 * response (with non-success status) for a station we can't
		 * accept.
		 * Also, disassociate frames might happen, particular with
		 * reason 7 ("Class 3 frame received from nonassociated STA").
		 */
		if (ieee80211_is_probe_resp(fc) || ieee80211_is_auth(fc) ||
		    ieee80211_is_deauth(fc) || ieee80211_is_assoc_resp(fc) ||
		    ieee80211_is_disassoc(fc))
			return mvm->probe_queue;
		if (info->hw_queue == info->control.vif->cab_queue)
			return mvmvif->cab_queue;

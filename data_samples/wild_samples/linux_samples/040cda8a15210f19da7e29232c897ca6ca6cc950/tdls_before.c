		switch (*pos) {
		case WLAN_EID_SUPP_RATES:
			sta_ptr->tdls_cap.rates_len = pos[1];
			for (i = 0; i < pos[1]; i++)
				sta_ptr->tdls_cap.rates[i] = pos[i + 2];
			break;

		case WLAN_EID_EXT_SUPP_RATES:
			basic = sta_ptr->tdls_cap.rates_len;
			for (i = 0; i < pos[1]; i++)
				sta_ptr->tdls_cap.rates[basic + i] = pos[i + 2];
			sta_ptr->tdls_cap.rates_len += pos[1];
			break;
		case WLAN_EID_HT_CAPABILITY:
			memcpy((u8 *)&sta_ptr->tdls_cap.ht_capb, pos,
			       sizeof(struct ieee80211_ht_cap));
			sta_ptr->is_11n_enabled = 1;
			break;
		case WLAN_EID_HT_OPERATION:
			memcpy(&sta_ptr->tdls_cap.ht_oper, pos,
			       sizeof(struct ieee80211_ht_operation));
			break;
		case WLAN_EID_BSS_COEX_2040:
			sta_ptr->tdls_cap.coex_2040 = pos[2];
			break;
		case WLAN_EID_EXT_CAPABILITY:
			memcpy((u8 *)&sta_ptr->tdls_cap.extcap, pos,
			       sizeof(struct ieee_types_header) +
			       min_t(u8, pos[1], 8));
			break;
		case WLAN_EID_RSN:
			memcpy((u8 *)&sta_ptr->tdls_cap.rsn_ie, pos,
			       sizeof(struct ieee_types_header) +
			       min_t(u8, pos[1], IEEE_MAX_IE_SIZE -
				     sizeof(struct ieee_types_header)));
			break;
		case WLAN_EID_QOS_CAPA:
			sta_ptr->tdls_cap.qos_info = pos[2];
			break;
		case WLAN_EID_VHT_OPERATION:
			if (priv->adapter->is_hw_11ac_capable)
				memcpy(&sta_ptr->tdls_cap.vhtoper, pos,
				       sizeof(struct ieee80211_vht_operation));
			break;
		case WLAN_EID_VHT_CAPABILITY:
			if (priv->adapter->is_hw_11ac_capable) {
				memcpy((u8 *)&sta_ptr->tdls_cap.vhtcap, pos,
				       sizeof(struct ieee80211_vht_cap));
				sta_ptr->is_11ac_enabled = 1;
			}
			break;
		case WLAN_EID_AID:
			if (priv->adapter->is_hw_11ac_capable)
				sta_ptr->tdls_cap.aid =
					get_unaligned_le16((pos + 2));
		default:
			break;
		}
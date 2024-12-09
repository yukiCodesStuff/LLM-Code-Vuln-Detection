		if (requires_sequential_pn(rx, fc)) {
			int queue = rx->security_idx;

			/* Store CCMP/GCMP PN so that we can verify that the
			 * next fragment has a sequential PN value.
			 */
			entry->check_sequential_pn = true;
			memcpy(entry->last_pn,
			       rx->key->u.ccmp.rx_pn[queue],
			       IEEE80211_CCMP_PN_LEN);
			BUILD_BUG_ON(offsetof(struct ieee80211_key,
					      u.ccmp.rx_pn) !=
				     offsetof(struct ieee80211_key,
					      u.gcmp.rx_pn));
			BUILD_BUG_ON(sizeof(rx->key->u.ccmp.rx_pn[queue]) !=
				     sizeof(rx->key->u.gcmp.rx_pn[queue]));
			BUILD_BUG_ON(IEEE80211_CCMP_PN_LEN !=
				     IEEE80211_GCMP_PN_LEN);
		}
		return RX_QUEUED;
	}

	/* This is a fragment for a frame that should already be pending in
	 * fragment cache. Add this fragment to the end of the pending entry.
	 */
	entry = ieee80211_reassemble_find(rx->sdata, frag, seq,
					  rx->seqno_idx, hdr);
	if (!entry) {
		I802_DEBUG_INC(rx->local->rx_handlers_drop_defrag);
		return RX_DROP_MONITOR;
	}

	/* "The receiver shall discard MSDUs and MMPDUs whose constituent
	 *  MPDU PN values are not incrementing in steps of 1."
	 * see IEEE P802.11-REVmc/D5.0, 12.5.3.4.4, item d (for CCMP)
	 * and IEEE P802.11-REVmc/D5.0, 12.5.5.4.4, item d (for GCMP)
	 */
	if (entry->check_sequential_pn) {
		int i;
		u8 pn[IEEE80211_CCMP_PN_LEN], *rpn;
		int queue;

		if (!requires_sequential_pn(rx, fc))
			return RX_DROP_UNUSABLE;
		memcpy(pn, entry->last_pn, IEEE80211_CCMP_PN_LEN);
		for (i = IEEE80211_CCMP_PN_LEN - 1; i >= 0; i--) {
			pn[i]++;
			if (pn[i])
				break;
		}
		queue = rx->security_idx;
		rpn = rx->key->u.ccmp.rx_pn[queue];
		if (memcmp(pn, rpn, IEEE80211_CCMP_PN_LEN))
			return RX_DROP_UNUSABLE;
		memcpy(entry->last_pn, pn, IEEE80211_CCMP_PN_LEN);
	}

	skb_pull(rx->skb, ieee80211_hdrlen(fc));
	__skb_queue_tail(&entry->skb_list, rx->skb);
	entry->last_frag = frag;
	entry->extra_len += rx->skb->len;
	if (ieee80211_has_morefrags(fc)) {
		rx->skb = NULL;
		return RX_QUEUED;
	}

	rx->skb = __skb_dequeue(&entry->skb_list);
	if (skb_tailroom(rx->skb) < entry->extra_len) {
		I802_DEBUG_INC(rx->local->rx_expand_skb_head_defrag);
		if (unlikely(pskb_expand_head(rx->skb, 0, entry->extra_len,
					      GFP_ATOMIC))) {
			I802_DEBUG_INC(rx->local->rx_handlers_drop_defrag);
			__skb_queue_purge(&entry->skb_list);
			return RX_DROP_UNUSABLE;
		}
	}
	while ((skb = __skb_dequeue(&entry->skb_list))) {
		skb_put_data(rx->skb, skb->data, skb->len);
		dev_kfree_skb(skb);
	}

 out:
	ieee80211_led_rx(rx->local);
 out_no_led:
	if (rx->sta)
		rx->sta->rx_stats.packets++;
	return RX_CONTINUE;
}

static int ieee80211_802_1x_port_control(struct ieee80211_rx_data *rx)
{
	if (unlikely(!rx->sta || !test_sta_flag(rx->sta, WLAN_STA_AUTHORIZED)))
		return -EACCES;

	return 0;
}

static int ieee80211_drop_unencrypted(struct ieee80211_rx_data *rx, __le16 fc)
{
	struct ieee80211_hdr *hdr = (void *)rx->skb->data;
	struct sk_buff *skb = rx->skb;
	struct ieee80211_rx_status *status = IEEE80211_SKB_RXCB(skb);

	/*
	 * Pass through unencrypted frames if the hardware has
	 * decrypted them already.
	 */
	if (status->flag & RX_FLAG_DECRYPTED)
		return 0;

	/* check mesh EAPOL frames first */
	if (unlikely(rx->sta && ieee80211_vif_is_mesh(&rx->sdata->vif) &&
		     ieee80211_is_data(fc))) {
		struct ieee80211s_hdr *mesh_hdr;
		u16 hdr_len = ieee80211_hdrlen(fc);
		u16 ethertype_offset;
		__be16 ethertype;

		if (!ether_addr_equal(hdr->addr1, rx->sdata->vif.addr))
			goto drop_check;

		/* make sure fixed part of mesh header is there, also checks skb len */
		if (!pskb_may_pull(rx->skb, hdr_len + 6))
			goto drop_check;

		mesh_hdr = (struct ieee80211s_hdr *)(skb->data + hdr_len);
		ethertype_offset = hdr_len + ieee80211_get_mesh_hdrlen(mesh_hdr) +
				   sizeof(rfc1042_header);

		if (skb_copy_bits(rx->skb, ethertype_offset, &ethertype, 2) == 0 &&
		    ethertype == rx->sdata->control_port_protocol)
			return 0;
	}

drop_check:
	/* Drop unencrypted frames if key is set. */
	if (unlikely(!ieee80211_has_protected(fc) &&
		     !ieee80211_is_any_nullfunc(fc) &&
		     ieee80211_is_data(fc) && rx->key))
		return -EACCES;

	return 0;
}

static int ieee80211_drop_unencrypted_mgmt(struct ieee80211_rx_data *rx)
{
	struct ieee80211_hdr *hdr = (struct ieee80211_hdr *)rx->skb->data;
	struct ieee80211_rx_status *status = IEEE80211_SKB_RXCB(rx->skb);
	__le16 fc = hdr->frame_control;

	/*
	 * Pass through unencrypted frames if the hardware has
	 * decrypted them already.
	 */
	if (status->flag & RX_FLAG_DECRYPTED)
		return 0;

	if (rx->sta && test_sta_flag(rx->sta, WLAN_STA_MFP)) {
		if (unlikely(!ieee80211_has_protected(fc) &&
			     ieee80211_is_unicast_robust_mgmt_frame(rx->skb) &&
			     rx->key)) {
			if (ieee80211_is_deauth(fc) ||
			    ieee80211_is_disassoc(fc))
				cfg80211_rx_unprot_mlme_mgmt(rx->sdata->dev,
							     rx->skb->data,
							     rx->skb->len);
			return -EACCES;
		}
		/* BIP does not use Protected field, so need to check MMIE */
		if (unlikely(ieee80211_is_multicast_robust_mgmt_frame(rx->skb) &&
			     ieee80211_get_mmie_keyidx(rx->skb) < 0)) {
			if (ieee80211_is_deauth(fc) ||
			    ieee80211_is_disassoc(fc))
				cfg80211_rx_unprot_mlme_mgmt(rx->sdata->dev,
							     rx->skb->data,
							     rx->skb->len);
			return -EACCES;
		}
		if (unlikely(ieee80211_is_beacon(fc) && rx->key &&
			     ieee80211_get_mmie_keyidx(rx->skb) < 0)) {
			cfg80211_rx_unprot_mlme_mgmt(rx->sdata->dev,
						     rx->skb->data,
						     rx->skb->len);
			return -EACCES;
		}
		/*
		 * When using MFP, Action frames are not allowed prior to
		 * having configured keys.
		 */
		if (unlikely(ieee80211_is_action(fc) && !rx->key &&
			     ieee80211_is_robust_mgmt_frame(rx->skb)))
			return -EACCES;
	}

	return 0;
}

static int
__ieee80211_data_to_8023(struct ieee80211_rx_data *rx, bool *port_control)
{
	struct ieee80211_sub_if_data *sdata = rx->sdata;
	struct ieee80211_hdr *hdr = (struct ieee80211_hdr *)rx->skb->data;
	bool check_port_control = false;
	struct ethhdr *ehdr;
	int ret;

	*port_control = false;
	if (ieee80211_has_a4(hdr->frame_control) &&
	    sdata->vif.type == NL80211_IFTYPE_AP_VLAN && !sdata->u.vlan.sta)
		return -1;

	if (sdata->vif.type == NL80211_IFTYPE_STATION &&
	    !!sdata->u.mgd.use_4addr != !!ieee80211_has_a4(hdr->frame_control)) {

		if (!sdata->u.mgd.use_4addr)
			return -1;
		else if (!ether_addr_equal(hdr->addr1, sdata->vif.addr))
			check_port_control = true;
	}

	if (is_multicast_ether_addr(hdr->addr1) &&
	    sdata->vif.type == NL80211_IFTYPE_AP_VLAN && sdata->u.vlan.sta)
		return -1;

	ret = ieee80211_data_to_8023(rx->skb, sdata->vif.addr, sdata->vif.type);
	if (ret < 0)
		return ret;

	ehdr = (struct ethhdr *) rx->skb->data;
	if (ehdr->h_proto == rx->sdata->control_port_protocol)
		*port_control = true;
	else if (check_port_control)
		return -1;

	return 0;
}

/*
 * requires that rx->skb is a frame with ethernet header
 */
static bool ieee80211_frame_allowed(struct ieee80211_rx_data *rx, __le16 fc)
{
	static const u8 pae_group_addr[ETH_ALEN] __aligned(2)
		= { 0x01, 0x80, 0xC2, 0x00, 0x00, 0x03 };
	struct ethhdr *ehdr = (struct ethhdr *) rx->skb->data;

	/*
	 * Allow EAPOL frames to us/the PAE group address regardless
	 * of whether the frame was encrypted or not.
	 */
	if (ehdr->h_proto == rx->sdata->control_port_protocol &&
	    (ether_addr_equal(ehdr->h_dest, rx->sdata->vif.addr) ||
	     ether_addr_equal(ehdr->h_dest, pae_group_addr)))
		return true;

	if (ieee80211_802_1x_port_control(rx) ||
	    ieee80211_drop_unencrypted(rx, fc))
		return false;

	return true;
}

static void ieee80211_deliver_skb_to_local_stack(struct sk_buff *skb,
						 struct ieee80211_rx_data *rx)
{
	struct ieee80211_sub_if_data *sdata = rx->sdata;
	struct net_device *dev = sdata->dev;

	if (unlikely((skb->protocol == sdata->control_port_protocol ||
		     (skb->protocol == cpu_to_be16(ETH_P_PREAUTH) &&
		      !sdata->control_port_no_preauth)) &&
		     sdata->control_port_over_nl80211)) {
		struct ieee80211_rx_status *status = IEEE80211_SKB_RXCB(skb);
		bool noencrypt = !(status->flag & RX_FLAG_DECRYPTED);

		cfg80211_rx_control_port(dev, skb, noencrypt);
		dev_kfree_skb(skb);
	} else {
		memset(skb->cb, 0, sizeof(skb->cb));

		/* deliver to local stack */
		if (rx->list)
			list_add_tail(&skb->list, rx->list);
		else
			netif_receive_skb(skb);
	}
}

/*
 * requires that rx->skb is a frame with ethernet header
 */
static void
ieee80211_deliver_skb(struct ieee80211_rx_data *rx)
{
	struct ieee80211_sub_if_data *sdata = rx->sdata;
	struct net_device *dev = sdata->dev;
	struct sk_buff *skb, *xmit_skb;
	struct ethhdr *ehdr = (struct ethhdr *) rx->skb->data;
	struct sta_info *dsta;

	skb = rx->skb;
	xmit_skb = NULL;

	dev_sw_netstats_rx_add(dev, skb->len);

	if (rx->sta) {
		/* The seqno index has the same property as needed
		 * for the rx_msdu field, i.e. it is IEEE80211_NUM_TIDS
		 * for non-QoS-data frames. Here we know it's a data
		 * frame, so count MSDUs.
		 */
		u64_stats_update_begin(&rx->sta->rx_stats.syncp);
		rx->sta->rx_stats.msdu[rx->seqno_idx]++;
		u64_stats_update_end(&rx->sta->rx_stats.syncp);
	}

	if ((sdata->vif.type == NL80211_IFTYPE_AP ||
	     sdata->vif.type == NL80211_IFTYPE_AP_VLAN) &&
	    !(sdata->flags & IEEE80211_SDATA_DONT_BRIDGE_PACKETS) &&
	    (sdata->vif.type != NL80211_IFTYPE_AP_VLAN || !sdata->u.vlan.sta)) {
		if (is_multicast_ether_addr(ehdr->h_dest) &&
		    ieee80211_vif_get_num_mcast_if(sdata) != 0) {
			/*
			 * send multicast frames both to higher layers in
			 * local net stack and back to the wireless medium
			 */
			xmit_skb = skb_copy(skb, GFP_ATOMIC);
			if (!xmit_skb)
				net_info_ratelimited("%s: failed to clone multicast frame\n",
						    dev->name);
		} else if (!is_multicast_ether_addr(ehdr->h_dest) &&
			   !ether_addr_equal(ehdr->h_dest, ehdr->h_source)) {
			dsta = sta_info_get(sdata, ehdr->h_dest);
			if (dsta) {
				/*
				 * The destination station is associated to
				 * this AP (in this VLAN), so send the frame
				 * directly to it and do not pass it to local
				 * net stack.
				 */
				xmit_skb = skb;
				skb = NULL;
			}
		}
	}
	if (entry->check_sequential_pn) {
		int i;
		u8 pn[IEEE80211_CCMP_PN_LEN], *rpn;
		int queue;

		if (!requires_sequential_pn(rx, fc))
			return RX_DROP_UNUSABLE;
		memcpy(pn, entry->last_pn, IEEE80211_CCMP_PN_LEN);
		for (i = IEEE80211_CCMP_PN_LEN - 1; i >= 0; i--) {
			pn[i]++;
			if (pn[i])
				break;
		}
// SPDX-License-Identifier: GPL-2.0-only
/*
 * Copyright 2002-2005, Instant802 Networks, Inc.
 * Copyright 2005-2006, Devicescape Software, Inc.
 * Copyright 2006-2007	Jiri Benc <jbenc@suse.cz>
 * Copyright 2007-2010	Johannes Berg <johannes@sipsolutions.net>
 * Copyright 2013-2014  Intel Mobile Communications GmbH
 * Copyright(c) 2015 - 2017 Intel Deutschland GmbH
 * Copyright (C) 2018-2021 Intel Corporation
 */

#include <linux/jiffies.h>
#include <linux/slab.h>
#include <linux/kernel.h>
#include <linux/skbuff.h>
#include <linux/netdevice.h>
#include <linux/etherdevice.h>
#include <linux/rcupdate.h>
#include <linux/export.h>
#include <linux/kcov.h>
#include <linux/bitops.h>
#include <net/mac80211.h>
#include <net/ieee80211_radiotap.h>
#include <asm/unaligned.h>

#include "ieee80211_i.h"
#include "driver-ops.h"
#include "led.h"
#include "mesh.h"
#include "wep.h"
#include "wpa.h"
#include "tkip.h"
#include "wme.h"
#include "rate.h"

/*
 * monitor mode reception
 *
 * This function cleans up the SKB, i.e. it removes all the stuff
 * only useful for monitoring.
 */
static struct sk_buff *ieee80211_clean_skb(struct sk_buff *skb,
					   unsigned int present_fcs_len,
					   unsigned int rtap_space)
{
	struct ieee80211_hdr *hdr;
	unsigned int hdrlen;
	__le16 fc;

	if (present_fcs_len)
		__pskb_trim(skb, skb->len - present_fcs_len);
	__pskb_pull(skb, rtap_space);

	hdr = (void *)skb->data;
	fc = hdr->frame_control;

	/*
	 * Remove the HT-Control field (if present) on management
	 * frames after we've sent the frame to monitoring. We
	 * (currently) don't need it, and don't properly parse
	 * frames with it present, due to the assumption of a
	 * fixed management header length.
	 */
	if (likely(!ieee80211_is_mgmt(fc) || !ieee80211_has_order(fc)))
		return skb;

	hdrlen = ieee80211_hdrlen(fc);
	hdr->frame_control &= ~cpu_to_le16(IEEE80211_FCTL_ORDER);

	if (!pskb_may_pull(skb, hdrlen)) {
		dev_kfree_skb(skb);
		return NULL;
	}

	memmove(skb->data + IEEE80211_HT_CTL_LEN, skb->data,
		hdrlen - IEEE80211_HT_CTL_LEN);
	__pskb_pull(skb, IEEE80211_HT_CTL_LEN);

	return skb;
}
		switch (rx->sdata->vif.type) {
		case NL80211_IFTYPE_AP_VLAN:
			if (!rx->sdata->u.vlan.sta)
				return RX_DROP_UNUSABLE;
			break;
		case NL80211_IFTYPE_STATION:
			if (!rx->sdata->u.mgd.use_4addr)
				return RX_DROP_UNUSABLE;
			break;
		default:
			return RX_DROP_UNUSABLE;
		}
	}

	if (is_multicast_ether_addr(hdr->addr1))
		return RX_DROP_UNUSABLE;

	if (rx->key) {
		/*
		 * We should not receive A-MSDUs on pre-HT connections,
		 * and HT connections cannot use old ciphers. Thus drop
		 * them, as in those cases we couldn't even have SPP
		 * A-MSDUs or such.
		 */
		switch (rx->key->conf.cipher) {
		case WLAN_CIPHER_SUITE_WEP40:
		case WLAN_CIPHER_SUITE_WEP104:
		case WLAN_CIPHER_SUITE_TKIP:
			return RX_DROP_UNUSABLE;
		default:
			break;
		}
	}
/* SPDX-License-Identifier: GPL-2.0-only */
/******************************************************************************
 *
 * Copyright(c) 2009 - 2014 Intel Corporation. All rights reserved.
 * Copyright(C) 2016        Intel Deutschland GmbH
 * Copyright(c) 2018        Intel Corporation
 *****************************************************************************/

#ifndef __IWLWIFI_DEVICE_TRACE
#include <linux/skbuff.h>
#include <linux/ieee80211.h>
#include <net/cfg80211.h>
#include "iwl-trans.h"
#if !defined(__IWLWIFI_DEVICE_TRACE)
static inline bool iwl_trace_data(struct sk_buff *skb)
{
	struct ieee80211_hdr *hdr = (void *)skb->data;
	__le16 fc = hdr->frame_control;
	int offs = 24; /* start with normal header length */

	if (!ieee80211_is_data(fc))
		return false;

	/* Try to determine if the frame is EAPOL. This might have false
	 * positives (if there's no RFC 1042 header and we compare to some
	 * payload instead) but since we're only doing tracing that's not
	 * a problem.
	 */

	if (ieee80211_has_a4(fc))
		offs += 6;
	if (ieee80211_is_data_qos(fc))
		offs += 2;
	/* don't account for crypto - these are unencrypted */

	/* also account for the RFC 1042 header, of course */
	offs += 6;

	return skb->len <= offs + 2 ||
		*(__be16 *)(skb->data + offs) != cpu_to_be16(ETH_P_PAE);
}
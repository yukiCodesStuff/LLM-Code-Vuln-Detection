 * Copyright 2007-2010	Johannes Berg <johannes@sipsolutions.net>
 * Copyright 2013-2014  Intel Mobile Communications GmbH
 * Copyright(c) 2015 - 2017 Intel Deutschland GmbH
 * Copyright (C) 2018-2021 Intel Corporation
 */

#include <linux/jiffies.h>
#include <linux/slab.h>
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

	return __ieee80211_rx_h_amsdu(rx, 0);
}

#ifdef CONFIG_MAC80211_MESH
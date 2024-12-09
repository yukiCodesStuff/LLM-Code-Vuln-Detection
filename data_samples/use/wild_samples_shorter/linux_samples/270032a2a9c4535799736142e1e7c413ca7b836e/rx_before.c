 * Copyright 2007-2010	Johannes Berg <johannes@sipsolutions.net>
 * Copyright 2013-2014  Intel Mobile Communications GmbH
 * Copyright(c) 2015 - 2017 Intel Deutschland GmbH
 * Copyright (C) 2018-2020 Intel Corporation
 */

#include <linux/jiffies.h>
#include <linux/slab.h>
	if (is_multicast_ether_addr(hdr->addr1))
		return RX_DROP_UNUSABLE;

	return __ieee80211_rx_h_amsdu(rx, 0);
}

#ifdef CONFIG_MAC80211_MESH
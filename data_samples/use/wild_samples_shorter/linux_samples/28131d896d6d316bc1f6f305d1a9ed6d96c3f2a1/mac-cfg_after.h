/* SPDX-License-Identifier: GPL-2.0 OR BSD-3-Clause */
/*
 * Copyright (C) 2012-2014, 2018-2019, 2021 Intel Corporation
 * Copyright (C) 2013-2015 Intel Mobile Communications GmbH
 * Copyright (C) 2016-2017 Intel Deutschland GmbH
 */
#ifndef __iwl_fw_api_mac_cfg_h__
	PROBE_RESPONSE_DATA_NOTIF = 0xFC,

	/**
	 * @CHANNEL_SWITCH_START_NOTIF: &struct iwl_channel_switch_start_notif
	 */
	CHANNEL_SWITCH_START_NOTIF = 0xFF,
};

#define IWL_P2P_NOA_DESC_COUNT	(2)

} __packed; /* MISSED_VAP_NTFY_API_S_VER_1 */

/**
 * struct iwl_channel_switch_start_notif - Channel switch start notification
 *
 * @id_and_color: ID and color of the MAC
 */
struct iwl_channel_switch_start_notif {
	__le32 id_and_color;
} __packed; /* CHANNEL_SWITCH_START_NTFY_API_S_VER_1 */

/**
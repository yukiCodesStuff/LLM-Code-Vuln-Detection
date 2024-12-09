/* SPDX-License-Identifier: GPL-2.0 OR BSD-3-Clause */
/*
 * Copyright (C) 2012-2014, 2018-2020 Intel Corporation
 * Copyright (C) 2016-2017 Intel Deutschland GmbH
 */
#ifndef __iwl_fw_api_tx_h__
#define __iwl_fw_api_tx_h__
	IWL_TX_FLAGS_CMD_RATE		= BIT(0),
	IWL_TX_FLAGS_ENCRYPT_DIS	= BIT(1),
	IWL_TX_FLAGS_HIGH_PRI		= BIT(2),
}; /* TX_FLAGS_BITS_API_S_VER_3 */

/**
 * enum iwl_tx_pm_timeouts - pm timeout values in TX command
	struct iwl_dram_sec_info dram_info;
	__le32 rate_n_flags;
	struct ieee80211_hdr hdr[];
} __packed; /* TX_CMD_API_S_VER_7 */

/**
 * struct iwl_tx_cmd_gen3 - TX command struct to FW for AX210+ devices
 * ( TX_CMD = 0x1c )
	__le32 rate_n_flags;
	__le64 ttl;
	struct ieee80211_hdr hdr[];
} __packed; /* TX_CMD_API_S_VER_8 */

/*
 * TX response related data
 */
	__le16 tx_queue;
	__le16 reserved2;
	struct agg_tx_status status;
} __packed; /* TX_RSP_API_S_VER_6 */

/**
 * struct iwl_mvm_ba_notif - notifies about reception of BA
 * ( BA_NOTIF = 0xc5 )
	__le16 ra_tid_cnt;
	struct iwl_mvm_compressed_ba_ratid ra_tid[0];
	struct iwl_mvm_compressed_ba_tfd tfd[];
} __packed; /* COMPRESSED_BA_RES_API_S_VER_4 */

/**
 * struct iwl_mac_beacon_cmd_v6 - beacon template command
 * @tx: the tx commands associated with the beacon frame
	struct ieee80211_hdr frame[];
} __packed; /* BEACON_TEMPLATE_CMD_API_S_VER_7 */

enum iwl_mac_beacon_flags {
	IWL_MAC_BEACON_CCK	= BIT(8),
	IWL_MAC_BEACON_ANT_A	= BIT(9),
	IWL_MAC_BEACON_ANT_B	= BIT(10),
	IWL_MAC_BEACON_ANT_C	= BIT(11),
	IWL_MAC_BEACON_FILS	= BIT(12),
};

/**
 * struct iwl_mac_beacon_cmd - beacon template command with offloaded CSA
	__le32 ecsa_offset;
	__le32 csa_offset;
	struct ieee80211_hdr frame[];
} __packed; /* BEACON_TEMPLATE_CMD_API_S_VER_10 */

struct iwl_beacon_notif {
	struct iwl_mvm_tx_resp beacon_notify_hdr;
	__le64 tsf;
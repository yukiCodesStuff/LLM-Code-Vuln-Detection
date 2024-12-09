// SPDX-License-Identifier: GPL-2.0 OR BSD-3-Clause
/*
 * Copyright (C) 2012-2014, 2018-2019 Intel Corporation
 * Copyright (C) 2013-2014 Intel Mobile Communications GmbH
 * Copyright (C) 2015-2017 Intel Deutschland GmbH
 */
#include <linux/kernel.h>
	enum ieee80211_ac_numbers ac;
	bool tid_found = false;

#ifdef CONFIG_IWLWIFI_DEBUGFS
	/* set advanced pm flag with no uapsd ACs to enable ps-poll */
	if (mvmvif->dbgfs_pm.use_ps_poll) {
		cmd->flags |= cpu_to_le16(POWER_FLAGS_ADVANCE_PM_ENA_MSK);

	cmd->uapsd_max_sp = mvm->hw->uapsd_max_sp_len;

	if (test_bit(IWL_MVM_STATUS_IN_D3, &mvm->status) ||
	    cmd->flags & cpu_to_le16(POWER_FLAGS_SNOOZE_ENA_MSK)) {
		cmd->rx_data_timeout_uapsd =
			cpu_to_le32(IWL_MVM_WOWLAN_PS_RX_DATA_TIMEOUT);
		cmd->tx_data_timeout_uapsd =
			cpu_to_le32(IWL_MVM_WOWLAN_PS_TX_DATA_TIMEOUT);
	} else {
		cmd->rx_data_timeout_uapsd =
			cpu_to_le32(IWL_MVM_UAPSD_RX_DATA_TIMEOUT);
		cmd->tx_data_timeout_uapsd =
			cpu_to_le32(IWL_MVM_UAPSD_TX_DATA_TIMEOUT);
	}

	if (cmd->flags & cpu_to_le16(POWER_FLAGS_SNOOZE_ENA_MSK)) {
		cmd->heavy_tx_thld_packets =
			IWL_MVM_PS_SNOOZE_HEAVY_TX_THLD_PACKETS;
		cmd->heavy_rx_thld_packets =
 * Copyright(c) 2015 Intel Mobile Communications GmbH
 * Copyright(c) 2017 Intel Deutschland GmbH
 * Copyright(c) 2018 - 2019 Intel Corporation
 *
 * Contact Information:
 *  Intel Linux Wireless <linuxwifi@intel.com>
 * Intel Corporation, 5200 N.E. Elam Young Parkway, Hillsboro, OR 97124-6497
 *
 *****************************************************************************/

#ifndef __rs_h__
#define __rs_h__

#define IWL_RATE_60M_PLCP 3

enum {
	IWL_RATE_INVM_INDEX = IWL_RATE_COUNT,
	IWL_RATE_INVALID = IWL_RATE_COUNT,
};

#define LINK_QUAL_MAX_RETRY_NUM 16

enum {
	IWL_RATE_6M_INDEX_TABLE = 0,
#define is_ht80(rate)         ((rate)->bw == RATE_MCS_CHAN_WIDTH_80)
#define is_ht160(rate)        ((rate)->bw == RATE_MCS_CHAN_WIDTH_160)

#define IWL_MAX_MCS_DISPLAY_SIZE	12

struct iwl_rate_mcs_info {
	char	mbps[IWL_MAX_MCS_DISPLAY_SIZE];
	char	mcs[IWL_MAX_MCS_DISPLAY_SIZE];
};

/**
 * struct iwl_lq_sta_rs_fw - rate and related statistics for RS in FW
 * @last_rate_n_flags: last rate reported by FW
 * @sta_id: the id of the station
/*
 * Copyright (c) 2013 Eugene Krasnikov <k.eugene.e@gmail.com>
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY
 * SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION
 * OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN
 * CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#define pr_fmt(fmt) KBUILD_MODNAME ": " fmt

#include <linux/bitfield.h>
#include <linux/etherdevice.h>
#include <linux/firmware.h>
#include <linux/bitops.h>
#include <linux/rpmsg.h>
#include "smd.h"

struct wcn36xx_cfg_val {
	u32 cfg_id;
	u32 value;
};

#define WCN36XX_CFG_VAL(id, val) \
{ \
	.cfg_id = WCN36XX_HAL_CFG_ ## id, \
	.value = val \
}
	if (sta->ht_cap.ht_supported) {
		unsigned long caps = sta->ht_cap.cap;
		sta_params->ht_capable = sta->ht_cap.ht_supported;
		sta_params->tx_channel_width_set = is_cap_supported(caps,
			IEEE80211_HT_CAP_SUP_WIDTH_20_40);
		sta_params->lsig_txop_protection = is_cap_supported(caps,
			IEEE80211_HT_CAP_LSIG_TXOP_PROT);

		sta_params->max_ampdu_size = sta->ht_cap.ampdu_factor;
		sta_params->max_ampdu_density = sta->ht_cap.ampdu_density;
		/* max_amsdu_size: 1 : 3839 bytes, 0 : 7935 bytes (max) */
		sta_params->max_amsdu_size = !is_cap_supported(caps,
			IEEE80211_HT_CAP_MAX_AMSDU);
		sta_params->sgi_20Mhz = is_cap_supported(caps,
			IEEE80211_HT_CAP_SGI_20);
		sta_params->sgi_40mhz =	is_cap_supported(caps,
			IEEE80211_HT_CAP_SGI_40);
		sta_params->green_field_capable = is_cap_supported(caps,
			IEEE80211_HT_CAP_GRN_FLD);
		sta_params->delayed_ba_support = is_cap_supported(caps,
			IEEE80211_HT_CAP_DELAY_BA);
		sta_params->dsss_cck_mode_40mhz = is_cap_supported(caps,
			IEEE80211_HT_CAP_DSSSCCK40);
	}
}

static void wcn36xx_smd_set_sta_vht_params(struct wcn36xx *wcn,
		struct ieee80211_sta *sta,
		struct wcn36xx_hal_config_sta_params_v1 *sta_params)
{
	if (sta->vht_cap.vht_supported) {
		unsigned long caps = sta->vht_cap.cap;

		sta_params->vht_capable = sta->vht_cap.vht_supported;
		sta_params->vht_ldpc_enabled =
			is_cap_supported(caps, IEEE80211_VHT_CAP_RXLDPC);
		if (get_feat_caps(wcn->fw_feat_caps, MU_MIMO)) {
			sta_params->vht_tx_mu_beamformee_capable =
				is_cap_supported(caps, IEEE80211_VHT_CAP_MU_BEAMFORMER_CAPABLE);
			if (sta_params->vht_tx_mu_beamformee_capable)
			       sta_params->vht_tx_bf_enabled = 1;
		} else {
			sta_params->vht_tx_mu_beamformee_capable = 0;
		}
		sta_params->vht_tx_channel_width_set = 0;
	}
	if (ret) {
		wcn36xx_err("hal_stop_scan_offload response failed err=%d\n",
			    ret);
		goto out;
	}
out:
	mutex_unlock(&wcn->hal_mutex);
	return ret;
}

int wcn36xx_smd_update_channel_list(struct wcn36xx *wcn, struct cfg80211_scan_request *req)
{
	struct wcn36xx_hal_update_channel_list_req_msg *msg_body;
	int ret, i;

	msg_body = kzalloc(sizeof(*msg_body), GFP_KERNEL);
	if (!msg_body)
		return -ENOMEM;

	INIT_HAL_MSG((*msg_body), WCN36XX_HAL_UPDATE_CHANNEL_LIST_REQ);

	msg_body->num_channel = min_t(u8, req->n_channels, sizeof(msg_body->channels));
	for (i = 0; i < msg_body->num_channel; i++) {
		struct wcn36xx_hal_channel_param *param = &msg_body->channels[i];
		u32 min_power = WCN36XX_HAL_DEFAULT_MIN_POWER;
		u32 ant_gain = WCN36XX_HAL_DEFAULT_ANT_GAIN;

		param->mhz = req->channels[i]->center_freq;
		param->band_center_freq1 = req->channels[i]->center_freq;
		param->band_center_freq2 = 0;

		if (req->channels[i]->flags & IEEE80211_CHAN_NO_IR)
			param->channel_info |= WCN36XX_HAL_CHAN_INFO_FLAG_PASSIVE;

		if (req->channels[i]->flags & IEEE80211_CHAN_RADAR)
			param->channel_info |= WCN36XX_HAL_CHAN_INFO_FLAG_DFS;

		if (req->channels[i]->band == NL80211_BAND_5GHZ) {
			param->channel_info |= WCN36XX_HAL_CHAN_INFO_FLAG_HT;
			param->channel_info |= WCN36XX_HAL_CHAN_INFO_FLAG_VHT;
			param->channel_info |= WCN36XX_HAL_CHAN_INFO_PHY_11A;
		} else {
			param->channel_info |= WCN36XX_HAL_CHAN_INFO_PHY_11BG;
		}

		if (min_power > req->channels[i]->max_power)
			min_power = req->channels[i]->max_power;

		if (req->channels[i]->max_antenna_gain)
			ant_gain = req->channels[i]->max_antenna_gain;

		u32p_replace_bits(&param->reg_info_1, min_power,
				  WCN36XX_HAL_CHAN_REG1_MIN_PWR_MASK);
		u32p_replace_bits(&param->reg_info_1, req->channels[i]->max_power,
				  WCN36XX_HAL_CHAN_REG1_MAX_PWR_MASK);
		u32p_replace_bits(&param->reg_info_1, req->channels[i]->max_reg_power,
				  WCN36XX_HAL_CHAN_REG1_REG_PWR_MASK);
		u32p_replace_bits(&param->reg_info_1, 0,
				  WCN36XX_HAL_CHAN_REG1_CLASS_ID_MASK);
		u32p_replace_bits(&param->reg_info_2, ant_gain,
				  WCN36XX_HAL_CHAN_REG2_ANT_GAIN_MASK);

		wcn36xx_dbg(WCN36XX_DBG_HAL,
			    "%s: freq=%u, channel_info=%08x, reg_info1=%08x, reg_info2=%08x\n",
			    __func__, param->mhz, param->channel_info, param->reg_info_1,
			    param->reg_info_2);
	}

	mutex_lock(&wcn->hal_mutex);

	PREPARE_HAL_BUF(wcn->hal_buf, (*msg_body));

	ret = wcn36xx_smd_send_and_wait(wcn, msg_body->header.len);
	if (ret) {
		wcn36xx_err("Sending hal_update_channel_list failed\n");
		goto out;
	}

	ret = wcn36xx_smd_rsp_status_check(wcn->hal_buf, wcn->hal_rsp_len);
	if (ret) {
		wcn36xx_err("hal_update_channel_list response failed err=%d\n", ret);
		goto out;
	}

out:
	kfree(msg_body);
	mutex_unlock(&wcn->hal_mutex);
	return ret;
}
{
	struct wcn36xx_hal_feat_caps_msg msg_body, *rsp;
	int ret, i;

	mutex_lock(&wcn->hal_mutex);
	INIT_HAL_MSG(msg_body, WCN36XX_HAL_FEATURE_CAPS_EXCHANGE_REQ);

	set_feat_caps(msg_body.feat_caps, STA_POWERSAVE);
	if (wcn->rf_id == RF_IRIS_WCN3680) {
		set_feat_caps(msg_body.feat_caps, DOT11AC);
		set_feat_caps(msg_body.feat_caps, WLAN_CH144);
		set_feat_caps(msg_body.feat_caps, ANTENNA_DIVERSITY_SELECTION);
	}
	switch (msg_header->msg_type) {
	case WCN36XX_HAL_START_RSP:
	case WCN36XX_HAL_CONFIG_STA_RSP:
	case WCN36XX_HAL_CONFIG_BSS_RSP:
	case WCN36XX_HAL_ADD_STA_SELF_RSP:
	case WCN36XX_HAL_STOP_RSP:
	case WCN36XX_HAL_DEL_STA_SELF_RSP:
	case WCN36XX_HAL_DELETE_STA_RSP:
	case WCN36XX_HAL_INIT_SCAN_RSP:
	case WCN36XX_HAL_START_SCAN_RSP:
	case WCN36XX_HAL_END_SCAN_RSP:
	case WCN36XX_HAL_FINISH_SCAN_RSP:
	case WCN36XX_HAL_DOWNLOAD_NV_RSP:
	case WCN36XX_HAL_DELETE_BSS_RSP:
	case WCN36XX_HAL_SEND_BEACON_RSP:
	case WCN36XX_HAL_SET_LINK_ST_RSP:
	case WCN36XX_HAL_UPDATE_PROBE_RSP_TEMPLATE_RSP:
	case WCN36XX_HAL_SET_BSSKEY_RSP:
	case WCN36XX_HAL_SET_STAKEY_RSP:
	case WCN36XX_HAL_RMV_STAKEY_RSP:
	case WCN36XX_HAL_RMV_BSSKEY_RSP:
	case WCN36XX_HAL_ENTER_BMPS_RSP:
	case WCN36XX_HAL_SET_POWER_PARAMS_RSP:
	case WCN36XX_HAL_EXIT_BMPS_RSP:
	case WCN36XX_HAL_KEEP_ALIVE_RSP:
	case WCN36XX_HAL_DUMP_COMMAND_RSP:
	case WCN36XX_HAL_ADD_BA_SESSION_RSP:
	case WCN36XX_HAL_ADD_BA_RSP:
	case WCN36XX_HAL_DEL_BA_RSP:
	case WCN36XX_HAL_TRIGGER_BA_RSP:
	case WCN36XX_HAL_UPDATE_CFG_RSP:
	case WCN36XX_HAL_JOIN_RSP:
	case WCN36XX_HAL_UPDATE_SCAN_PARAM_RSP:
	case WCN36XX_HAL_CH_SWITCH_RSP:
	case WCN36XX_HAL_PROCESS_PTT_RSP:
	case WCN36XX_HAL_FEATURE_CAPS_EXCHANGE_RSP:
	case WCN36XX_HAL_8023_MULTICAST_LIST_RSP:
	case WCN36XX_HAL_START_SCAN_OFFLOAD_RSP:
	case WCN36XX_HAL_STOP_SCAN_OFFLOAD_RSP:
	case WCN36XX_HAL_HOST_OFFLOAD_RSP:
	case WCN36XX_HAL_GTK_OFFLOAD_RSP:
	case WCN36XX_HAL_GTK_OFFLOAD_GETINFO_RSP:
	case WCN36XX_HAL_HOST_RESUME_RSP:
	case WCN36XX_HAL_ENTER_IMPS_RSP:
	case WCN36XX_HAL_EXIT_IMPS_RSP:
	case WCN36XX_HAL_UPDATE_CHANNEL_LIST_RSP:
		memcpy(wcn->hal_buf, buf, len);
		wcn->hal_rsp_len = len;
		complete(&wcn->hal_rsp_compl);
		break;

	case WCN36XX_HAL_COEX_IND:
	case WCN36XX_HAL_AVOID_FREQ_RANGE_IND:
	case WCN36XX_HAL_DEL_BA_IND:
	case WCN36XX_HAL_OTA_TX_COMPL_IND:
	case WCN36XX_HAL_MISSED_BEACON_IND:
	case WCN36XX_HAL_DELETE_STA_CONTEXT_IND:
	case WCN36XX_HAL_PRINT_REG_INFO_IND:
	case WCN36XX_HAL_SCAN_OFFLOAD_IND:
		msg_ind = kmalloc(sizeof(*msg_ind) + len, GFP_ATOMIC);
		if (!msg_ind) {
			wcn36xx_err("Run out of memory while handling SMD_EVENT (%d)\n",
				    msg_header->msg_type);
			return -ENOMEM;
		}
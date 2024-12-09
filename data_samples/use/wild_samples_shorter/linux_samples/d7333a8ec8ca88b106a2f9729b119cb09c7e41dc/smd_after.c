
#define pr_fmt(fmt) KBUILD_MODNAME ": " fmt

#include <linux/bitfield.h>
#include <linux/etherdevice.h>
#include <linux/firmware.h>
#include <linux/bitops.h>
#include <linux/rpmsg.h>

		sta_params->max_ampdu_size = sta->ht_cap.ampdu_factor;
		sta_params->max_ampdu_density = sta->ht_cap.ampdu_density;
		/* max_amsdu_size: 1 : 3839 bytes, 0 : 7935 bytes (max) */
		sta_params->max_amsdu_size = !is_cap_supported(caps,
			IEEE80211_HT_CAP_MAX_AMSDU);
		sta_params->sgi_20Mhz = is_cap_supported(caps,
			IEEE80211_HT_CAP_SGI_20);
		sta_params->sgi_40mhz =	is_cap_supported(caps,
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

static int wcn36xx_smd_switch_channel_rsp(void *buf, size_t len)
{
	struct wcn36xx_hal_switch_channel_rsp_msg *rsp;
	int ret;
	INIT_HAL_MSG(msg_body, WCN36XX_HAL_FEATURE_CAPS_EXCHANGE_REQ);

	set_feat_caps(msg_body.feat_caps, STA_POWERSAVE);
	if (wcn->rf_id == RF_IRIS_WCN3680) {
		set_feat_caps(msg_body.feat_caps, DOT11AC);
		set_feat_caps(msg_body.feat_caps, WLAN_CH144);
		set_feat_caps(msg_body.feat_caps, ANTENNA_DIVERSITY_SELECTION);
	}

	PREPARE_HAL_BUF(wcn->hal_buf, msg_body);

	ret = wcn36xx_smd_send_and_wait(wcn, msg_body.header.len);
	case WCN36XX_HAL_HOST_RESUME_RSP:
	case WCN36XX_HAL_ENTER_IMPS_RSP:
	case WCN36XX_HAL_EXIT_IMPS_RSP:
	case WCN36XX_HAL_UPDATE_CHANNEL_LIST_RSP:
		memcpy(wcn->hal_buf, buf, len);
		wcn->hal_rsp_len = len;
		complete(&wcn->hal_rsp_compl);
		break;
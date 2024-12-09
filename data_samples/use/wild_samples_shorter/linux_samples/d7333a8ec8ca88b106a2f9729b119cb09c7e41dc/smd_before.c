
#define pr_fmt(fmt) KBUILD_MODNAME ": " fmt

#include <linux/etherdevice.h>
#include <linux/firmware.h>
#include <linux/bitops.h>
#include <linux/rpmsg.h>

		sta_params->max_ampdu_size = sta->ht_cap.ampdu_factor;
		sta_params->max_ampdu_density = sta->ht_cap.ampdu_density;
		sta_params->max_amsdu_size = is_cap_supported(caps,
			IEEE80211_HT_CAP_MAX_AMSDU);
		sta_params->sgi_20Mhz = is_cap_supported(caps,
			IEEE80211_HT_CAP_SGI_20);
		sta_params->sgi_40mhz =	is_cap_supported(caps,
	return ret;
}

static int wcn36xx_smd_switch_channel_rsp(void *buf, size_t len)
{
	struct wcn36xx_hal_switch_channel_rsp_msg *rsp;
	int ret;
	INIT_HAL_MSG(msg_body, WCN36XX_HAL_FEATURE_CAPS_EXCHANGE_REQ);

	set_feat_caps(msg_body.feat_caps, STA_POWERSAVE);
	if (wcn->rf_id == RF_IRIS_WCN3680)
		set_feat_caps(msg_body.feat_caps, DOT11AC);

	PREPARE_HAL_BUF(wcn->hal_buf, msg_body);

	ret = wcn36xx_smd_send_and_wait(wcn, msg_body.header.len);
	case WCN36XX_HAL_HOST_RESUME_RSP:
	case WCN36XX_HAL_ENTER_IMPS_RSP:
	case WCN36XX_HAL_EXIT_IMPS_RSP:
		memcpy(wcn->hal_buf, buf, len);
		wcn->hal_rsp_len = len;
		complete(&wcn->hal_rsp_compl);
		break;
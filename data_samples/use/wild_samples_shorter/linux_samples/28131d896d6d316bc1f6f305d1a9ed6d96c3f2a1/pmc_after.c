
#include "wcn36xx.h"

#define WCN36XX_BMPS_FAIL_THREHOLD 3

int wcn36xx_pmc_enter_bmps_state(struct wcn36xx *wcn,
				 struct ieee80211_vif *vif)
{
	int ret = 0;
	struct wcn36xx_vif *vif_priv = wcn36xx_vif_to_priv(vif);
	/* TODO: Make sure the TX chain clean */
	ret = wcn36xx_smd_enter_bmps(wcn, vif);
	if (!ret) {
		wcn36xx_dbg(WCN36XX_DBG_PMC, "Entered BMPS\n");
		vif_priv->pw_state = WCN36XX_BMPS;
		vif_priv->bmps_fail_ct = 0;
		vif->driver_flags |= IEEE80211_VIF_BEACON_FILTER;
	} else {
		/*
		 * One of the reasons why HW will not enter BMPS is because
		 * received just after auth complete
		 */
		wcn36xx_err("Can not enter BMPS!\n");

		if (vif_priv->bmps_fail_ct++ == WCN36XX_BMPS_FAIL_THREHOLD) {
			ieee80211_connection_loss(vif);
			vif_priv->bmps_fail_ct = 0;
		}
	}
	return ret;
}

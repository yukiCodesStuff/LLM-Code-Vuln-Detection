
#include "wcn36xx.h"

int wcn36xx_pmc_enter_bmps_state(struct wcn36xx *wcn,
				 struct ieee80211_vif *vif)
{
	int ret = 0;
	struct wcn36xx_vif *vif_priv = wcn36xx_vif_to_priv(vif);

	if (!vif_priv->allow_bmps)
		return -ENOTSUPP;

	ret = wcn36xx_smd_enter_bmps(wcn, vif);
	if (!ret) {
		wcn36xx_dbg(WCN36XX_DBG_PMC, "Entered BMPS\n");
		vif_priv->pw_state = WCN36XX_BMPS;
		vif->driver_flags |= IEEE80211_VIF_BEACON_FILTER;
	} else {
		/*
		 * One of the reasons why HW will not enter BMPS is because
		 * received just after auth complete
		 */
		wcn36xx_err("Can not enter BMPS!\n");
	}
	return ret;
}

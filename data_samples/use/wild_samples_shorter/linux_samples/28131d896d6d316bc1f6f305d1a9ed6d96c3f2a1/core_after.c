	list_for_each_entry_safe(rtwtxq, tmp, &rtwdev->ba_list, list) {
		struct ieee80211_txq *txq = rtw89_txq_to_txq(rtwtxq);
		struct ieee80211_sta *sta = txq->sta;
		struct rtw89_sta *rtwsta = sta ? (struct rtw89_sta *)sta->drv_priv : NULL;
		u8 tid = txq->tid;

		if (!sta) {
			rtw89_warn(rtwdev, "cannot start BA without sta\n");
	struct ieee80211_hw *hw = rtwdev->hw;
	struct ieee80211_txq *txq = rtw89_txq_to_txq(rtwtxq);
	struct ieee80211_sta *sta = txq->sta;
	struct rtw89_sta *rtwsta = sta ? (struct rtw89_sta *)sta->drv_priv : NULL;

	if (unlikely(skb_get_queue_mapping(skb) == IEEE80211_AC_VO))
		return;

{
	struct rtw89_txq *rtwtxq = (struct rtw89_txq *)txq->drv_priv;
	struct ieee80211_sta *sta = txq->sta;
	struct rtw89_sta *rtwsta = sta ? (struct rtw89_sta *)sta->drv_priv : NULL;

	if (!sta || rtwsta->max_agg_wait <= 0)
		return false;

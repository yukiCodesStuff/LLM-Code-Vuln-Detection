	list_for_each_entry_safe(rtwtxq, tmp, &rtwdev->ba_list, list) {
		struct ieee80211_txq *txq = rtw89_txq_to_txq(rtwtxq);
		struct ieee80211_sta *sta = txq->sta;
		struct rtw89_sta *rtwsta = (struct rtw89_sta *)sta->drv_priv;
		u8 tid = txq->tid;

		if (!sta) {
			rtw89_warn(rtwdev, "cannot start BA without sta\n");
			goto skip_ba_work;
		}
{
	struct ieee80211_hw *hw = rtwdev->hw;
	struct ieee80211_txq *txq = rtw89_txq_to_txq(rtwtxq);
	struct ieee80211_sta *sta = txq->sta;
	struct rtw89_sta *rtwsta = (struct rtw89_sta *)sta->drv_priv;

	if (unlikely(skb_get_queue_mapping(skb) == IEEE80211_AC_VO))
		return;

	if (unlikely(skb->protocol == cpu_to_be16(ETH_P_PAE)))
		return;

	if (unlikely(!sta))
		return;

	if (unlikely(test_bit(RTW89_TXQ_F_BLOCK_BA, &rtwtxq->flags)))
		return;

	if (test_bit(RTW89_TXQ_F_AMPDU, &rtwtxq->flags)) {
		IEEE80211_SKB_CB(skb)->flags |= IEEE80211_TX_CTL_AMPDU;
		return;
	}
{
	struct rtw89_txq *rtwtxq = (struct rtw89_txq *)txq->drv_priv;
	struct ieee80211_sta *sta = txq->sta;
	struct rtw89_sta *rtwsta = (struct rtw89_sta *)sta->drv_priv;

	if (!sta || rtwsta->max_agg_wait <= 0)
		return false;

	if (rtwdev->stats.tx_tfc_lv <= RTW89_TFC_MID)
		return false;

	if (*frame_cnt > 1) {
		*frame_cnt -= 1;
		*sched_txq = true;
		*reinvoke = true;
		rtwtxq->wait_cnt = 1;
		return false;
	}
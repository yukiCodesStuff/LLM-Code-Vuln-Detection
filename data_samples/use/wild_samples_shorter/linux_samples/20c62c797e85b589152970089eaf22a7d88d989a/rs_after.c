	    (lq_sta->tx_agg_tid_en & BIT(tid)) &&
	    (tid_data->tx_count_last >= IWL_MVM_RS_AGG_START_THRESHOLD)) {
		IWL_DEBUG_RATE(mvm, "try to aggregate tid %d\n", tid);
		if (rs_tl_turn_on_agg_for_tid(mvm, lq_sta, tid, sta) == 0)
			tid_data->state = IWL_AGG_QUEUED;
	}
}

static inline int get_num_of_ant_from_rate(u32 rate_n_flags)
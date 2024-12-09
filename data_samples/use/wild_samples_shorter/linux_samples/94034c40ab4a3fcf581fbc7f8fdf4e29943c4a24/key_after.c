		       struct ieee80211_sub_if_data *sdata,
		       struct sta_info *sta)
{
	static atomic_t key_color = ATOMIC_INIT(0);
	struct ieee80211_key *old_key;
	int idx = key->conf.keyidx;
	bool pairwise = key->conf.flags & IEEE80211_KEY_FLAG_PAIRWISE;
	/*
	key->sdata = sdata;
	key->sta = sta;

	/*
	 * Assign a unique ID to every key so we can easily prevent mixed
	 * key and fragment cache attacks.
	 */
	key->color = atomic_inc_return(&key_color);

	increment_tailroom_need_count(sdata);

	ret = ieee80211_key_replace(sdata, sta, pairwise, old_key, key);

	struct {
		struct dentry *stalink;
		struct dentry *dir;
		int cnt;
	} debugfs;
#endif

	/*
	 * key config, must be last because it contains key
	 * material as variable length member
	 */
	struct ieee80211_key_conf conf;
};

struct ieee80211_key *
ieee80211_key_alloc(u32 cipher, int idx, size_t key_len,
		    const u8 *key_data,
		    size_t seq_len, const u8 *seq,
		    const struct ieee80211_cipher_scheme *cs);
/*
 * Insert a key into data structures (sdata, sta if necessary)
 * to make it used, free old key. On failure, also free the new key.
 */
int ieee80211_key_link(struct ieee80211_key *key,
		       struct ieee80211_sub_if_data *sdata,
		       struct sta_info *sta);
int ieee80211_set_tx_key(struct ieee80211_key *key);
void ieee80211_key_free(struct ieee80211_key *key, bool delay_tailroom);
void ieee80211_key_free_unused(struct ieee80211_key *key);
void ieee80211_set_default_key(struct ieee80211_sub_if_data *sdata, int idx,
			       bool uni, bool multi);
void ieee80211_set_default_mgmt_key(struct ieee80211_sub_if_data *sdata,
				    int idx);
void ieee80211_set_default_beacon_key(struct ieee80211_sub_if_data *sdata,
				      int idx);
void ieee80211_free_keys(struct ieee80211_sub_if_data *sdata,
			 bool force_synchronize);
void ieee80211_free_sta_keys(struct ieee80211_local *local,
			     struct sta_info *sta);
void ieee80211_reenable_keys(struct ieee80211_sub_if_data *sdata);

#define key_mtx_dereference(local, ref) \
	rcu_dereference_protected(ref, lockdep_is_held(&((local)->key_mtx)))

void ieee80211_delayed_tailroom_dec(struct work_struct *wk);

#endif /* IEEE80211_KEY_H */
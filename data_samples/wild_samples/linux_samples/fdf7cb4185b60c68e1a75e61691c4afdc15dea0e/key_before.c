/*
 * Copyright 2002-2005, Instant802 Networks, Inc.
 * Copyright 2005-2006, Devicescape Software, Inc.
 * Copyright 2006-2007	Jiri Benc <jbenc@suse.cz>
 * Copyright 2007-2008	Johannes Berg <johannes@sipsolutions.net>
 * Copyright 2013-2014  Intel Mobile Communications GmbH
 * Copyright 2015	Intel Deutschland GmbH
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include <linux/if_ether.h>
#include <linux/etherdevice.h>
#include <linux/list.h>
#include <linux/rcupdate.h>
#include <linux/rtnetlink.h>
#include <linux/slab.h>
#include <linux/export.h>
#include <net/mac80211.h>
#include <asm/unaligned.h>
#include "ieee80211_i.h"
#include "driver-ops.h"
#include "debugfs_key.h"
#include "aes_ccm.h"
#include "aes_cmac.h"
#include "aes_gmac.h"
#include "aes_gcm.h"


/**
 * DOC: Key handling basics
 *
 * Key handling in mac80211 is done based on per-interface (sub_if_data)
 * keys and per-station keys. Since each station belongs to an interface,
 * each station key also belongs to that interface.
 *
 * Hardware acceleration is done on a best-effort basis for algorithms
 * that are implemented in software,  for each key the hardware is asked
 * to enable that key for offloading but if it cannot do that the key is
 * simply kept for software encryption (unless it is for an algorithm
 * that isn't implemented in software).
 * There is currently no way of knowing whether a key is handled in SW
 * or HW except by looking into debugfs.
 *
 * All key management is internally protected by a mutex. Within all
 * other parts of mac80211, key references are, just as STA structure
 * references, protected by RCU. Note, however, that some things are
 * unprotected, namely the key->sta dereferences within the hardware
 * acceleration functions. This means that sta_info_destroy() must
 * remove the key which waits for an RCU grace period.
 */

static const u8 bcast_addr[ETH_ALEN] = { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF };

static void assert_key_lock(struct ieee80211_local *local)
{
	lockdep_assert_held(&local->key_mtx);
}
{
	struct ieee80211_local *local = sdata->local;
	struct ieee80211_key *old_key;
	int idx, ret;
	bool pairwise;

	pairwise = key->conf.flags & IEEE80211_KEY_FLAG_PAIRWISE;
	idx = key->conf.keyidx;
	key->local = sdata->local;
	key->sdata = sdata;
	key->sta = sta;

	mutex_lock(&sdata->local->key_mtx);

	if (sta && pairwise)
		old_key = key_mtx_dereference(sdata->local, sta->ptk[idx]);
	else if (sta)
		old_key = key_mtx_dereference(sdata->local, sta->gtk[idx]);
	else
		old_key = key_mtx_dereference(sdata->local, sdata->keys[idx]);

	increment_tailroom_need_count(sdata);

	ieee80211_key_replace(sdata, sta, pairwise, old_key, key);
	ieee80211_key_destroy(old_key, true);

	ieee80211_debugfs_key_add(key);

	if (!local->wowlan) {
		ret = ieee80211_key_enable_hw_accel(key);
		if (ret)
			ieee80211_key_free(key, true);
	} else {
		ret = 0;
	}

	mutex_unlock(&sdata->local->key_mtx);

	return ret;
}
{
	struct ieee80211_local *local = sdata->local;
	struct ieee80211_key *old_key;
	int idx, ret;
	bool pairwise;

	pairwise = key->conf.flags & IEEE80211_KEY_FLAG_PAIRWISE;
	idx = key->conf.keyidx;
	key->local = sdata->local;
	key->sdata = sdata;
	key->sta = sta;

	mutex_lock(&sdata->local->key_mtx);

	if (sta && pairwise)
		old_key = key_mtx_dereference(sdata->local, sta->ptk[idx]);
	else if (sta)
		old_key = key_mtx_dereference(sdata->local, sta->gtk[idx]);
	else
		old_key = key_mtx_dereference(sdata->local, sdata->keys[idx]);

	increment_tailroom_need_count(sdata);

	ieee80211_key_replace(sdata, sta, pairwise, old_key, key);
	ieee80211_key_destroy(old_key, true);

	ieee80211_debugfs_key_add(key);

	if (!local->wowlan) {
		ret = ieee80211_key_enable_hw_accel(key);
		if (ret)
			ieee80211_key_free(key, true);
	} else {
		ret = 0;
	}

	mutex_unlock(&sdata->local->key_mtx);

	return ret;
}
	} else {
		ret = 0;
	}

	mutex_unlock(&sdata->local->key_mtx);

	return ret;
}

void ieee80211_key_free(struct ieee80211_key *key, bool delay_tailroom)
{
	if (!key)
		return;

	/*
	 * Replace key with nothingness if it was ever used.
	 */
	if (key->sdata)
		ieee80211_key_replace(key->sdata, key->sta,
				key->conf.flags & IEEE80211_KEY_FLAG_PAIRWISE,
				key, NULL);
	ieee80211_key_destroy(key, delay_tailroom);
}

void ieee80211_enable_keys(struct ieee80211_sub_if_data *sdata)
{
	struct ieee80211_key *key;
	struct ieee80211_sub_if_data *vlan;

	ASSERT_RTNL();

	if (WARN_ON(!ieee80211_sdata_running(sdata)))
		return;

	mutex_lock(&sdata->local->key_mtx);

	WARN_ON_ONCE(sdata->crypto_tx_tailroom_needed_cnt ||
		     sdata->crypto_tx_tailroom_pending_dec);

	if (sdata->vif.type == NL80211_IFTYPE_AP) {
		list_for_each_entry(vlan, &sdata->u.ap.vlans, u.vlan.list)
			WARN_ON_ONCE(vlan->crypto_tx_tailroom_needed_cnt ||
				     vlan->crypto_tx_tailroom_pending_dec);
	}

	list_for_each_entry(key, &sdata->key_list, list) {
		increment_tailroom_need_count(sdata);
		ieee80211_key_enable_hw_accel(key);
	}

	mutex_unlock(&sdata->local->key_mtx);
}

void ieee80211_reset_crypto_tx_tailroom(struct ieee80211_sub_if_data *sdata)
{
	struct ieee80211_sub_if_data *vlan;

	mutex_lock(&sdata->local->key_mtx);

	sdata->crypto_tx_tailroom_needed_cnt = 0;

	if (sdata->vif.type == NL80211_IFTYPE_AP) {
		list_for_each_entry(vlan, &sdata->u.ap.vlans, u.vlan.list)
			vlan->crypto_tx_tailroom_needed_cnt = 0;
	}

	mutex_unlock(&sdata->local->key_mtx);
}

void ieee80211_iter_keys(struct ieee80211_hw *hw,
			 struct ieee80211_vif *vif,
			 void (*iter)(struct ieee80211_hw *hw,
				      struct ieee80211_vif *vif,
				      struct ieee80211_sta *sta,
				      struct ieee80211_key_conf *key,
				      void *data),
			 void *iter_data)
{
	struct ieee80211_local *local = hw_to_local(hw);
	struct ieee80211_key *key, *tmp;
	struct ieee80211_sub_if_data *sdata;

	ASSERT_RTNL();

	mutex_lock(&local->key_mtx);
	if (vif) {
		sdata = vif_to_sdata(vif);
		list_for_each_entry_safe(key, tmp, &sdata->key_list, list)
			iter(hw, &sdata->vif,
			     key->sta ? &key->sta->sta : NULL,
			     &key->conf, iter_data);
	} else {
		list_for_each_entry(sdata, &local->interfaces, list)
			list_for_each_entry_safe(key, tmp,
						 &sdata->key_list, list)
				iter(hw, &sdata->vif,
				     key->sta ? &key->sta->sta : NULL,
				     &key->conf, iter_data);
	}
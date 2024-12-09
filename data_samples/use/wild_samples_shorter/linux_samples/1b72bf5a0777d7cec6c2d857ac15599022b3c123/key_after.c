 * Copyright 2006-2007	Jiri Benc <jbenc@suse.cz>
 * Copyright 2007-2008	Johannes Berg <johannes@sipsolutions.net>
 * Copyright 2013-2014  Intel Mobile Communications GmbH
 * Copyright 2015-2017	Intel Deutschland GmbH
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.

	pairwise = key->conf.flags & IEEE80211_KEY_FLAG_PAIRWISE;
	idx = key->conf.keyidx;

	mutex_lock(&sdata->local->key_mtx);

	if (sta && pairwise)
	else
		old_key = key_mtx_dereference(sdata->local, sdata->keys[idx]);

	/*
	 * Silently accept key re-installation without really installing the
	 * new version of the key to avoid nonce reuse or replay issues.
	 */
	if (old_key && key->conf.keylen == old_key->conf.keylen &&
	    !memcmp(key->conf.key, old_key->conf.key, key->conf.keylen)) {
		ieee80211_key_free_unused(key);
		ret = 0;
		goto out;
	}

	key->local = sdata->local;
	key->sdata = sdata;
	key->sta = sta;

	increment_tailroom_need_count(sdata);

	ieee80211_key_replace(sdata, sta, pairwise, old_key, key);
	ieee80211_key_destroy(old_key, true);
		ret = 0;
	}

 out:
	mutex_unlock(&sdata->local->key_mtx);

	return ret;
}
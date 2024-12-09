	lockdep_assert_held(&rdev->bss_lock);

	bss->refcount++;
	if (bss->pub.hidden_beacon_bss) {
		bss = container_of(bss->pub.hidden_beacon_bss,
				   struct cfg80211_internal_bss,
				   pub);
		bss->refcount++;
	}
	if (bss->pub.transmitted_bss) {
		bss = container_of(bss->pub.transmitted_bss,
				   struct cfg80211_internal_bss,
				   pub);
		bss->refcount++;
	}
}

static inline void bss_ref_put(struct cfg80211_registered_device *rdev,
			       struct cfg80211_internal_bss *bss)
		new->refcount = 1;
		INIT_LIST_HEAD(&new->hidden_list);
		INIT_LIST_HEAD(&new->pub.nontrans_list);

		if (rcu_access_pointer(tmp->pub.proberesp_ies)) {
			hidden = rb_find_bss(rdev, tmp, BSS_CMP_HIDE_ZLEN);
			if (!hidden)
		spin_lock_bh(&rdev->bss_lock);
		if (cfg80211_add_nontrans_list(non_tx_data->tx_bss,
					       &res->pub)) {
			if (__cfg80211_unlink_bss(rdev, res))
				rdev->bss_generation++;
		}
		spin_unlock_bh(&rdev->bss_lock);
	}

	trace_cfg80211_return_bss(&res->pub);
	/* cfg80211_bss_update gives us a referenced result */
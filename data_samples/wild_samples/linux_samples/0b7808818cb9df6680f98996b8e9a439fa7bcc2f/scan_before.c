{
	lockdep_assert_held(&rdev->bss_lock);

	bss->refcount++;
	if (bss->pub.hidden_beacon_bss) {
		bss = container_of(bss->pub.hidden_beacon_bss,
				   struct cfg80211_internal_bss,
				   pub);
		bss->refcount++;
	}
		if (!new) {
			ies = (void *)rcu_dereference(tmp->pub.beacon_ies);
			if (ies)
				kfree_rcu(ies, rcu_head);
			ies = (void *)rcu_dereference(tmp->pub.proberesp_ies);
			if (ies)
				kfree_rcu(ies, rcu_head);
			goto drop;
		}
		memcpy(new, tmp, sizeof(*new));
		new->refcount = 1;
		INIT_LIST_HEAD(&new->hidden_list);
		INIT_LIST_HEAD(&new->pub.nontrans_list);

		if (rcu_access_pointer(tmp->pub.proberesp_ies)) {
			hidden = rb_find_bss(rdev, tmp, BSS_CMP_HIDE_ZLEN);
			if (!hidden)
				hidden = rb_find_bss(rdev, tmp,
						     BSS_CMP_HIDE_NUL);
			if (hidden) {
				new->pub.hidden_beacon_bss = &hidden->pub;
				list_add(&new->hidden_list,
					 &hidden->hidden_list);
				hidden->refcount++;
				rcu_assign_pointer(new->pub.beacon_ies,
						   hidden->pub.beacon_ies);
			}
		} else {
			/*
			 * Ok so we found a beacon, and don't have an entry. If
			 * it's a beacon with hidden SSID, we might be in for an
			 * expensive search for any probe responses that should
			 * be grouped with this beacon for updates ...
			 */
			if (!cfg80211_combine_bsses(rdev, new)) {
				bss_ref_put(rdev, new);
				goto drop;
			}
		}

		if (rdev->bss_entries >= bss_entries_limit &&
		    !cfg80211_bss_expire_oldest(rdev)) {
			bss_ref_put(rdev, new);
			goto drop;
		}

		/* This must be before the call to bss_ref_get */
		if (tmp->pub.transmitted_bss) {
			struct cfg80211_internal_bss *pbss =
				container_of(tmp->pub.transmitted_bss,
					     struct cfg80211_internal_bss,
					     pub);

			new->pub.transmitted_bss = tmp->pub.transmitted_bss;
			bss_ref_get(rdev, pbss);
		}

		list_add_tail(&new->list, &rdev->bss_list);
		rdev->bss_entries++;
		rb_insert_bss(rdev, new);
		found = new;
	}
	if (non_tx_data) {
		/* this is a nontransmitting bss, we need to add it to
		 * transmitting bss' list if it is not there
		 */
		spin_lock_bh(&rdev->bss_lock);
		if (cfg80211_add_nontrans_list(non_tx_data->tx_bss,
					       &res->pub)) {
			if (__cfg80211_unlink_bss(rdev, res))
				rdev->bss_generation++;
		}
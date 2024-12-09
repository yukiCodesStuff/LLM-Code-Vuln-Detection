			   ssid_elem->datalen)) {
			rcu_read_unlock();
			return 0;
		}
	}

	rcu_read_unlock();

	/* add to the list */
	list_add_tail(&nontrans_bss->nontrans_list, &trans_bss->nontrans_list);
	return 0;
}

static void __cfg80211_bss_expire(struct cfg80211_registered_device *rdev,
				  unsigned long expire_time)
{
	struct cfg80211_internal_bss *bss, *tmp;
	bool expired = false;

	lockdep_assert_held(&rdev->bss_lock);

	list_for_each_entry_safe(bss, tmp, &rdev->bss_list, list) {
		if (atomic_read(&bss->hold))
			continue;
		if (!time_after(expire_time, bss->ts))
			continue;

		if (__cfg80211_unlink_bss(rdev, bss))
			expired = true;
	}
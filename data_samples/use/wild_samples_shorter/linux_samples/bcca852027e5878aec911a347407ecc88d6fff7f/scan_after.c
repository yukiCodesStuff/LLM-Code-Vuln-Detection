
	rcu_read_unlock();

	/*
	 * This is a bit weird - it's not on the list, but already on another
	 * one! The only way that could happen is if there's some BSSID/SSID
	 * shared by multiple APs in their multi-BSSID profiles, potentially
	 * with hidden SSID mixed in ... ignore it.
	 */
	if (!list_empty(&nontrans_bss->nontrans_list))
		return -EINVAL;

	/* add to the list */
	list_add_tail(&nontrans_bss->nontrans_list, &trans_bss->nontrans_list);
	return 0;
}
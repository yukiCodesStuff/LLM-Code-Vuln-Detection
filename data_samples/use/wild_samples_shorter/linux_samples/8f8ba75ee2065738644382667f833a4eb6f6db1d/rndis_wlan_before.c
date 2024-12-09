						struct cfg80211_pmksa *pmksa,
						int max_pmkids)
{
	int i, err, newlen;
	unsigned int count;

	count = le32_to_cpu(pmkids->bssid_info_count);
	/* add new pmkid */
	newlen = sizeof(*pmkids) + (count + 1) * sizeof(pmkids->bssid_info[0]);

	pmkids = krealloc(pmkids, newlen, GFP_KERNEL);
	if (!pmkids) {
		err = -ENOMEM;
		goto error;
	}

	pmkids->length = cpu_to_le32(newlen);
	pmkids->bssid_info_count = cpu_to_le32(count + 1);

						struct cfg80211_pmksa *pmksa,
						int max_pmkids)
{
	struct ndis_80211_pmkid *new_pmkids;
	int i, err, newlen;
	unsigned int count;

	count = le32_to_cpu(pmkids->bssid_info_count);
	/* add new pmkid */
	newlen = sizeof(*pmkids) + (count + 1) * sizeof(pmkids->bssid_info[0]);

	new_pmkids = krealloc(pmkids, newlen, GFP_KERNEL);
	if (!new_pmkids) {
		err = -ENOMEM;
		goto error;
	}
	pmkids = new_pmkids;

	pmkids->length = cpu_to_le32(newlen);
	pmkids->bssid_info_count = cpu_to_le32(count + 1);

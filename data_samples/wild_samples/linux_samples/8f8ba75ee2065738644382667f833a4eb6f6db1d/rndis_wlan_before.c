		netdev_dbg(usbdev->net, "%s(): bssid not found (%pM)\n",
					__func__, pmksa->bssid);
		err = -ENOENT;
		goto error;
	}

	for (; i + 1 < count; i++)
		pmkids->bssid_info[i] = pmkids->bssid_info[i + 1];

	count--;
	newlen = sizeof(*pmkids) + count * sizeof(pmkids->bssid_info[0]);

	pmkids->length = cpu_to_le32(newlen);
	pmkids->bssid_info_count = cpu_to_le32(count);

	return pmkids;
error:
	kfree(pmkids);
	return ERR_PTR(err);
}

static struct ndis_80211_pmkid *update_pmkid(struct usbnet *usbdev,
						struct ndis_80211_pmkid *pmkids,
						struct cfg80211_pmksa *pmksa,
						int max_pmkids)
{
	int i, err, newlen;
	unsigned int count;

	count = le32_to_cpu(pmkids->bssid_info_count);

	if (count > max_pmkids)
		count = max_pmkids;

	/* update with new pmkid */
	for (i = 0; i < count; i++) {
		if (!ether_addr_equal(pmkids->bssid_info[i].bssid,
				      pmksa->bssid))
			continue;

		memcpy(pmkids->bssid_info[i].pmkid, pmksa->pmkid,
								WLAN_PMKID_LEN);

		return pmkids;
	}

	/* out of space, return error */
	if (i == max_pmkids) {
		netdev_dbg(usbdev->net, "%s(): out of space\n", __func__);
		err = -ENOSPC;
		goto error;
	}

	/* add new pmkid */
	newlen = sizeof(*pmkids) + (count + 1) * sizeof(pmkids->bssid_info[0]);

	pmkids = krealloc(pmkids, newlen, GFP_KERNEL);
	if (!pmkids) {
		err = -ENOMEM;
		goto error;
	}

	pmkids->length = cpu_to_le32(newlen);
	pmkids->bssid_info_count = cpu_to_le32(count + 1);

	memcpy(pmkids->bssid_info[count].bssid, pmksa->bssid, ETH_ALEN);
	memcpy(pmkids->bssid_info[count].pmkid, pmksa->pmkid, WLAN_PMKID_LEN);

	return pmkids;
error:
	kfree(pmkids);
	return ERR_PTR(err);
}
		netdev_dbg(usbdev->net, "%s(): out of space\n", __func__);
		err = -ENOSPC;
		goto error;
	}

	/* add new pmkid */
	newlen = sizeof(*pmkids) + (count + 1) * sizeof(pmkids->bssid_info[0]);

	pmkids = krealloc(pmkids, newlen, GFP_KERNEL);
	if (!pmkids) {
		err = -ENOMEM;
		goto error;
	}
{
	int ret;
	u64 val;

	ret = iwl_acpi_get_dsm_integer(dev, rev, func,
				       guid, &val, sizeof(u32));

	if (ret < 0)
		return ret;

	/* cast val (u64) to be u32 */
	*value = (u32)val;
	return 0;
}
IWL_EXPORT_SYMBOL(iwl_acpi_get_dsm_u32);

union acpi_object *iwl_acpi_get_wifi_pkg_range(struct device *dev,
					       union acpi_object *data,
					       int min_data_size,
					       int max_data_size,
					       int *tbl_rev)
{
	int i;
	union acpi_object *wifi_pkg;

	/*
	 * We need at least one entry in the wifi package that
	 * describes the domain, and one more entry, otherwise there's
	 * no point in reading it.
	 */
	if (WARN_ON_ONCE(min_data_size < 2 || min_data_size > max_data_size))
		return ERR_PTR(-EINVAL);

	/*
	 * We need at least two packages, one for the revision and one
	 * for the data itself.  Also check that the revision is valid
	 * (i.e. it is an integer (each caller has to check by itself
	 * if the returned revision is supported)).
	 */
	if (data->type != ACPI_TYPE_PACKAGE ||
	    data->package.count < 2 ||
	    data->package.elements[0].type != ACPI_TYPE_INTEGER) {
		IWL_DEBUG_DEV_RADIO(dev, "Invalid packages structure\n");
		return ERR_PTR(-EINVAL);
	}

	*tbl_rev = data->package.elements[0].integer.value;

	/* loop through all the packages to find the one for WiFi */
	for (i = 1; i < data->package.count; i++) {
		union acpi_object *domain;

		wifi_pkg = &data->package.elements[i];

		/* skip entries that are not a package with the right size */
		if (wifi_pkg->type != ACPI_TYPE_PACKAGE ||
		    wifi_pkg->package.count < min_data_size ||
		    wifi_pkg->package.count > max_data_size)
			continue;

		domain = &wifi_pkg->package.elements[0];
		if (domain->type == ACPI_TYPE_INTEGER &&
		    domain->integer.value == ACPI_WIFI_DOMAIN)
			goto found;
	}

	return ERR_PTR(-ENOENT);

found:
	return wifi_pkg;
}
{
	int i;
	union acpi_object *wifi_pkg;

	/*
	 * We need at least one entry in the wifi package that
	 * describes the domain, and one more entry, otherwise there's
	 * no point in reading it.
	 */
	if (WARN_ON_ONCE(min_data_size < 2 || min_data_size > max_data_size))
		return ERR_PTR(-EINVAL);

	/*
	 * We need at least two packages, one for the revision and one
	 * for the data itself.  Also check that the revision is valid
	 * (i.e. it is an integer (each caller has to check by itself
	 * if the returned revision is supported)).
	 */
	if (data->type != ACPI_TYPE_PACKAGE ||
	    data->package.count < 2 ||
	    data->package.elements[0].type != ACPI_TYPE_INTEGER) {
		IWL_DEBUG_DEV_RADIO(dev, "Invalid packages structure\n");
		return ERR_PTR(-EINVAL);
	}
	for (i = 1; i < data->package.count; i++) {
		union acpi_object *domain;

		wifi_pkg = &data->package.elements[i];

		/* skip entries that are not a package with the right size */
		if (wifi_pkg->type != ACPI_TYPE_PACKAGE ||
		    wifi_pkg->package.count < min_data_size ||
		    wifi_pkg->package.count > max_data_size)
			continue;

		domain = &wifi_pkg->package.elements[0];
		if (domain->type == ACPI_TYPE_INTEGER &&
		    domain->integer.value == ACPI_WIFI_DOMAIN)
			goto found;
	}

	return ERR_PTR(-ENOENT);

found:
	return wifi_pkg;
}
IWL_EXPORT_SYMBOL(iwl_acpi_get_wifi_pkg_range);

int iwl_acpi_get_tas(struct iwl_fw_runtime *fwrt,
		     __le32 *block_list_array,
		     int *block_list_size)
{
	union acpi_object *wifi_pkg, *data;
	int ret, tbl_rev, i;
	bool enabled;

	data = iwl_acpi_get_object(fwrt->dev, ACPI_WTAS_METHOD);
	if (IS_ERR(data))
		return PTR_ERR(data);

	wifi_pkg = iwl_acpi_get_wifi_pkg(fwrt->dev, data,
					 ACPI_WTAS_WIFI_DATA_SIZE,
					 &tbl_rev);
	if (IS_ERR(wifi_pkg)) {
		ret = PTR_ERR(wifi_pkg);
		goto out_free;
	}

	if (wifi_pkg->package.elements[0].type != ACPI_TYPE_INTEGER ||
	    tbl_rev != 0) {
		ret = -EINVAL;
		goto out_free;
	}

	enabled = !!wifi_pkg->package.elements[1].integer.value;

	if (!enabled) {
		*block_list_size = -1;
		IWL_DEBUG_RADIO(fwrt, "TAS not enabled\n");
		ret = 0;
		goto out_free;
	}

	if (wifi_pkg->package.elements[2].type != ACPI_TYPE_INTEGER ||
	    wifi_pkg->package.elements[2].integer.value >
	    APCI_WTAS_BLACK_LIST_MAX) {
		IWL_DEBUG_RADIO(fwrt, "TAS invalid array size %llu\n",
				wifi_pkg->package.elements[1].integer.value);
		ret = -EINVAL;
		goto out_free;
	}
	*block_list_size = wifi_pkg->package.elements[2].integer.value;

	IWL_DEBUG_RADIO(fwrt, "TAS array size %d\n", *block_list_size);
	if (*block_list_size > APCI_WTAS_BLACK_LIST_MAX) {
		IWL_DEBUG_RADIO(fwrt, "TAS invalid array size value %u\n",
				*block_list_size);
		ret = -EINVAL;
		goto out_free;
	}

	for (i = 0; i < *block_list_size; i++) {
		u32 country;

		if (wifi_pkg->package.elements[3 + i].type !=
		    ACPI_TYPE_INTEGER) {
			IWL_DEBUG_RADIO(fwrt,
					"TAS invalid array elem %d\n", 3 + i);
			ret = -EINVAL;
			goto out_free;
		}
	for (i = 1; i < data->package.count; i++) {
		union acpi_object *domain;

		wifi_pkg = &data->package.elements[i];

		/* skip entries that are not a package with the right size */
		if (wifi_pkg->type != ACPI_TYPE_PACKAGE ||
		    wifi_pkg->package.count < min_data_size ||
		    wifi_pkg->package.count > max_data_size)
			continue;

		domain = &wifi_pkg->package.elements[0];
		if (domain->type == ACPI_TYPE_INTEGER &&
		    domain->integer.value == ACPI_WIFI_DOMAIN)
			goto found;
	}

	return ERR_PTR(-ENOENT);

found:
	return wifi_pkg;
}
IWL_EXPORT_SYMBOL(iwl_acpi_get_wifi_pkg_range);

int iwl_acpi_get_tas(struct iwl_fw_runtime *fwrt,
		     __le32 *block_list_array,
		     int *block_list_size)
{
	union acpi_object *wifi_pkg, *data;
	int ret, tbl_rev, i;
	bool enabled;

	data = iwl_acpi_get_object(fwrt->dev, ACPI_WTAS_METHOD);
	if (IS_ERR(data))
		return PTR_ERR(data);

	wifi_pkg = iwl_acpi_get_wifi_pkg(fwrt->dev, data,
					 ACPI_WTAS_WIFI_DATA_SIZE,
					 &tbl_rev);
	if (IS_ERR(wifi_pkg)) {
		ret = PTR_ERR(wifi_pkg);
		goto out_free;
	}
{
	union acpi_object *wifi_pkg, *data;
	int i, j, k, ret, tbl_rev;
	u8 num_bands, num_profiles;
	static const struct {
		u8 revisions;
		u8 bands;
		u8 profiles;
		u8 min_profiles;
	} rev_data[] = {
		{
			.revisions = BIT(3),
			.bands = ACPI_GEO_NUM_BANDS_REV2,
			.profiles = ACPI_NUM_GEO_PROFILES_REV3,
			.min_profiles = 3,
		},
		{
			.revisions = BIT(2),
			.bands = ACPI_GEO_NUM_BANDS_REV2,
			.profiles = ACPI_NUM_GEO_PROFILES,
		},
		{
			.revisions = BIT(0) | BIT(1),
			.bands = ACPI_GEO_NUM_BANDS_REV0,
			.profiles = ACPI_NUM_GEO_PROFILES,
		},
	};
	int idx;
	/* start from one to skip the domain */
	int entry_idx = 1;

	BUILD_BUG_ON(ACPI_NUM_GEO_PROFILES_REV3 != IWL_NUM_GEO_PROFILES_V3);
	BUILD_BUG_ON(ACPI_NUM_GEO_PROFILES != IWL_NUM_GEO_PROFILES);

	data = iwl_acpi_get_object(fwrt->dev, ACPI_WGDS_METHOD);
	if (IS_ERR(data))
		return PTR_ERR(data);

	/* read the highest revision we understand first */
	for (idx = 0; idx < ARRAY_SIZE(rev_data); idx++) {
		/* min_profiles != 0 requires num_profiles header */
		u32 hdr_size = 1 + !!rev_data[idx].min_profiles;
		u32 profile_size = ACPI_GEO_PER_CHAIN_SIZE *
				   rev_data[idx].bands;
		u32 max_size = hdr_size + profile_size * rev_data[idx].profiles;
		u32 min_size;

		if (!rev_data[idx].min_profiles)
			min_size = max_size;
		else
			min_size = hdr_size +
				   profile_size * rev_data[idx].min_profiles;

		wifi_pkg = iwl_acpi_get_wifi_pkg_range(fwrt->dev, data,
						       min_size, max_size,
						       &tbl_rev);
		if (!IS_ERR(wifi_pkg)) {
			if (!(BIT(tbl_rev) & rev_data[idx].revisions))
				continue;

			num_bands = rev_data[idx].bands;
			num_profiles = rev_data[idx].profiles;

			if (rev_data[idx].min_profiles) {
				/* read header that says # of profiles */
				union acpi_object *entry;

				entry = &wifi_pkg->package.elements[entry_idx];
				entry_idx++;
				if (entry->type != ACPI_TYPE_INTEGER ||
				    entry->integer.value > num_profiles) {
					ret = -EINVAL;
					goto out_free;
				}
				num_profiles = entry->integer.value;

				/*
				 * this also validates >= min_profiles since we
				 * otherwise wouldn't have gotten the data when
				 * looking up in ACPI
				 */
				if (wifi_pkg->package.count !=
				    min_size + profile_size * num_profiles) {
					ret = -EINVAL;
					goto out_free;
				}
			}
			goto read_table;
		}
	}

	if (idx < ARRAY_SIZE(rev_data))
		ret = PTR_ERR(wifi_pkg);
	else
		ret = -ENOENT;
	goto out_free;

read_table:
	fwrt->geo_rev = tbl_rev;
	for (i = 0; i < num_profiles; i++) {
		for (j = 0; j < ACPI_GEO_NUM_BANDS_REV2; j++) {
			union acpi_object *entry;

			/*
			 * num_bands is either 2 or 3, if it's only 2 then
			 * fill the third band (6 GHz) with the values from
			 * 5 GHz (second band)
			 */
			if (j >= num_bands) {
				fwrt->geo_profiles[i].bands[j].max =
					fwrt->geo_profiles[i].bands[1].max;
			} else {
				entry = &wifi_pkg->package.elements[entry_idx];
				entry_idx++;
				if (entry->type != ACPI_TYPE_INTEGER ||
				    entry->integer.value > U8_MAX) {
					ret = -EINVAL;
					goto out_free;
				}

				fwrt->geo_profiles[i].bands[j].max =
					entry->integer.value;
			}

			for (k = 0; k < ACPI_GEO_NUM_CHAINS; k++) {
				/* same here as above */
				if (j >= num_bands) {
					fwrt->geo_profiles[i].bands[j].chains[k] =
						fwrt->geo_profiles[i].bands[1].chains[k];
				} else {
					entry = &wifi_pkg->package.elements[entry_idx];
					entry_idx++;
					if (entry->type != ACPI_TYPE_INTEGER ||
					    entry->integer.value > U8_MAX) {
						ret = -EINVAL;
						goto out_free;
					}

					fwrt->geo_profiles[i].bands[j].chains[k] =
						entry->integer.value;
				}
			}
		}
	}

	ret = 0;
out_free:
	kfree(data);
	return ret;
}
IWL_EXPORT_SYMBOL(iwl_sar_get_wgds_table);

bool iwl_sar_geo_support(struct iwl_fw_runtime *fwrt)
{
	/*
	 * The PER_CHAIN_LIMIT_OFFSET_CMD command is not supported on
	 * earlier firmware versions.  Unfortunately, we don't have a
	 * TLV API flag to rely on, so rely on the major version which
	 * is in the first byte of ucode_ver.  This was implemented
	 * initially on version 38 and then backported to 17.  It was
	 * also backported to 29, but only for 7265D devices.  The
	 * intention was to have it in 36 as well, but not all 8000
	 * family got this feature enabled.  The 8000 family is the
	 * only one using version 36, so skip this version entirely.
	 */
	return IWL_UCODE_SERIAL(fwrt->fw->ucode_ver) >= 38 ||
	       IWL_UCODE_SERIAL(fwrt->fw->ucode_ver) == 17 ||
	       (IWL_UCODE_SERIAL(fwrt->fw->ucode_ver) == 29 &&
		((fwrt->trans->hw_rev & CSR_HW_REV_TYPE_MSK) ==
		 CSR_HW_REV_TYPE_7265D));
}
IWL_EXPORT_SYMBOL(iwl_sar_geo_support);

int iwl_sar_geo_init(struct iwl_fw_runtime *fwrt,
		     struct iwl_per_chain_offset *table,
		     u32 n_bands, u32 n_profiles)
{
	int i, j;

	if (!iwl_sar_geo_support(fwrt))
		return -EOPNOTSUPP;

	for (i = 0; i < n_profiles; i++) {
		for (j = 0; j < n_bands; j++) {
			struct iwl_per_chain_offset *chain =
				&table[i * n_bands + j];

			chain->max_tx_power =
				cpu_to_le16(fwrt->geo_profiles[i].bands[j].max);
			chain->chain_a = fwrt->geo_profiles[i].bands[j].chains[0];
			chain->chain_b = fwrt->geo_profiles[i].bands[j].chains[1];
			IWL_DEBUG_RADIO(fwrt,
					"SAR geographic profile[%d] Band[%d]: chain A = %d chain B = %d max_tx_power = %d\n",
					i, j,
					fwrt->geo_profiles[i].bands[j].chains[0],
					fwrt->geo_profiles[i].bands[j].chains[1],
					fwrt->geo_profiles[i].bands[j].max);
		}
	}

	return 0;
}
IWL_EXPORT_SYMBOL(iwl_sar_geo_init);

__le32 iwl_acpi_get_lari_config_bitmap(struct iwl_fw_runtime *fwrt)
{
	int ret;
	u8 value;
	__le32 config_bitmap = 0;

	/*
	 ** Evaluate func 'DSM_FUNC_ENABLE_INDONESIA_5G2'
	 */
	ret = iwl_acpi_get_dsm_u8(fwrt->dev, 0,
				  DSM_FUNC_ENABLE_INDONESIA_5G2,
				  &iwl_guid, &value);

	if (!ret && value == DSM_VALUE_INDONESIA_ENABLE)
		config_bitmap |=
			cpu_to_le32(LARI_CONFIG_ENABLE_5G2_IN_INDONESIA_MSK);

	/*
	 ** Evaluate func 'DSM_FUNC_DISABLE_SRD'
	 */
	ret = iwl_acpi_get_dsm_u8(fwrt->dev, 0,
				  DSM_FUNC_DISABLE_SRD,
				  &iwl_guid, &value);
	if (!ret) {
		if (value == DSM_VALUE_SRD_PASSIVE)
			config_bitmap |=
				cpu_to_le32(LARI_CONFIG_CHANGE_ETSI_TO_PASSIVE_MSK);
		else if (value == DSM_VALUE_SRD_DISABLE)
			config_bitmap |=
				cpu_to_le32(LARI_CONFIG_CHANGE_ETSI_TO_DISABLED_MSK);
	}

	return config_bitmap;
}
IWL_EXPORT_SYMBOL(iwl_acpi_get_lari_config_bitmap);
			if (j >= num_bands) {
				fwrt->geo_profiles[i].bands[j].max =
					fwrt->geo_profiles[i].bands[1].max;
			} else {
				entry = &wifi_pkg->package.elements[entry_idx];
				entry_idx++;
				if (entry->type != ACPI_TYPE_INTEGER ||
				    entry->integer.value > U8_MAX) {
					ret = -EINVAL;
					goto out_free;
				}

				fwrt->geo_profiles[i].bands[j].max =
					entry->integer.value;
			}
				if (j >= num_bands) {
					fwrt->geo_profiles[i].bands[j].chains[k] =
						fwrt->geo_profiles[i].bands[1].chains[k];
				} else {
					entry = &wifi_pkg->package.elements[entry_idx];
					entry_idx++;
					if (entry->type != ACPI_TYPE_INTEGER ||
					    entry->integer.value > U8_MAX) {
						ret = -EINVAL;
						goto out_free;
					}

					fwrt->geo_profiles[i].bands[j].chains[k] =
						entry->integer.value;
				}
{
	/*
	 * The PER_CHAIN_LIMIT_OFFSET_CMD command is not supported on
	 * earlier firmware versions.  Unfortunately, we don't have a
	 * TLV API flag to rely on, so rely on the major version which
	 * is in the first byte of ucode_ver.  This was implemented
	 * initially on version 38 and then backported to 17.  It was
	 * also backported to 29, but only for 7265D devices.  The
	 * intention was to have it in 36 as well, but not all 8000
	 * family got this feature enabled.  The 8000 family is the
	 * only one using version 36, so skip this version entirely.
	 */
	return IWL_UCODE_SERIAL(fwrt->fw->ucode_ver) >= 38 ||
	       IWL_UCODE_SERIAL(fwrt->fw->ucode_ver) == 17 ||
	       (IWL_UCODE_SERIAL(fwrt->fw->ucode_ver) == 29 &&
		((fwrt->trans->hw_rev & CSR_HW_REV_TYPE_MSK) ==
		 CSR_HW_REV_TYPE_7265D));
}
{
	/*
	 * The PER_CHAIN_LIMIT_OFFSET_CMD command is not supported on
	 * earlier firmware versions.  Unfortunately, we don't have a
	 * TLV API flag to rely on, so rely on the major version which
	 * is in the first byte of ucode_ver.  This was implemented
	 * initially on version 38 and then backported to 17.  It was
	 * also backported to 29, but only for 7265D devices.  The
	 * intention was to have it in 36 as well, but not all 8000
	 * family got this feature enabled.  The 8000 family is the
	 * only one using version 36, so skip this version entirely.
	 */
	return IWL_UCODE_SERIAL(fwrt->fw->ucode_ver) >= 38 ||
	       IWL_UCODE_SERIAL(fwrt->fw->ucode_ver) == 17 ||
	       (IWL_UCODE_SERIAL(fwrt->fw->ucode_ver) == 29 &&
		((fwrt->trans->hw_rev & CSR_HW_REV_TYPE_MSK) ==
		 CSR_HW_REV_TYPE_7265D));
}
IWL_EXPORT_SYMBOL(iwl_sar_geo_support);

int iwl_sar_geo_init(struct iwl_fw_runtime *fwrt,
		     struct iwl_per_chain_offset *table,
		     u32 n_bands, u32 n_profiles)
{
	int i, j;

	if (!iwl_sar_geo_support(fwrt))
		return -EOPNOTSUPP;

	for (i = 0; i < n_profiles; i++) {
		for (j = 0; j < n_bands; j++) {
			struct iwl_per_chain_offset *chain =
				&table[i * n_bands + j];

			chain->max_tx_power =
				cpu_to_le16(fwrt->geo_profiles[i].bands[j].max);
			chain->chain_a = fwrt->geo_profiles[i].bands[j].chains[0];
			chain->chain_b = fwrt->geo_profiles[i].bands[j].chains[1];
			IWL_DEBUG_RADIO(fwrt,
					"SAR geographic profile[%d] Band[%d]: chain A = %d chain B = %d max_tx_power = %d\n",
					i, j,
					fwrt->geo_profiles[i].bands[j].chains[0],
					fwrt->geo_profiles[i].bands[j].chains[1],
					fwrt->geo_profiles[i].bands[j].max);
		}
	}

	return 0;
}
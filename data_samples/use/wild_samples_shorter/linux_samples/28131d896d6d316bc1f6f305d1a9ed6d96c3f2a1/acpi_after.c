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

	 * describes the domain, and one more entry, otherwise there's
	 * no point in reading it.
	 */
	if (WARN_ON_ONCE(min_data_size < 2 || min_data_size > max_data_size))
		return ERR_PTR(-EINVAL);

	/*
	 * We need at least two packages, one for the revision and one

		/* skip entries that are not a package with the right size */
		if (wifi_pkg->type != ACPI_TYPE_PACKAGE ||
		    wifi_pkg->package.count < min_data_size ||
		    wifi_pkg->package.count > max_data_size)
			continue;

		domain = &wifi_pkg->package.elements[0];
		if (domain->type == ACPI_TYPE_INTEGER &&
found:
	return wifi_pkg;
}
IWL_EXPORT_SYMBOL(iwl_acpi_get_wifi_pkg_range);

int iwl_acpi_get_tas(struct iwl_fw_runtime *fwrt,
		     __le32 *block_list_array,
		     int *block_list_size)
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
				fwrt->geo_profiles[i].bands[j].max =
					fwrt->geo_profiles[i].bands[1].max;
			} else {
				entry = &wifi_pkg->package.elements[entry_idx];
				entry_idx++;
				if (entry->type != ACPI_TYPE_INTEGER ||
				    entry->integer.value > U8_MAX) {
					ret = -EINVAL;
					goto out_free;
					fwrt->geo_profiles[i].bands[j].chains[k] =
						fwrt->geo_profiles[i].bands[1].chains[k];
				} else {
					entry = &wifi_pkg->package.elements[entry_idx];
					entry_idx++;
					if (entry->type != ACPI_TYPE_INTEGER ||
					    entry->integer.value > U8_MAX) {
						ret = -EINVAL;
						goto out_free;
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

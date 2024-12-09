}
IWL_EXPORT_SYMBOL(iwl_acpi_get_dsm_u32);

union acpi_object *iwl_acpi_get_wifi_pkg(struct device *dev,
					 union acpi_object *data,
					 int data_size, int *tbl_rev)
{
	int i;
	union acpi_object *wifi_pkg;

	 * describes the domain, and one more entry, otherwise there's
	 * no point in reading it.
	 */
	if (WARN_ON_ONCE(data_size < 2))
		return ERR_PTR(-EINVAL);

	/*
	 * We need at least two packages, one for the revision and one

		/* skip entries that are not a package with the right size */
		if (wifi_pkg->type != ACPI_TYPE_PACKAGE ||
		    wifi_pkg->package.count != data_size)
			continue;

		domain = &wifi_pkg->package.elements[0];
		if (domain->type == ACPI_TYPE_INTEGER &&
found:
	return wifi_pkg;
}
IWL_EXPORT_SYMBOL(iwl_acpi_get_wifi_pkg);

int iwl_acpi_get_tas(struct iwl_fw_runtime *fwrt,
		     __le32 *block_list_array,
		     int *block_list_size)
{
	union acpi_object *wifi_pkg, *data;
	int i, j, k, ret, tbl_rev;
	int idx = 1; /* start from one to skip the domain */
	u8 num_bands;

	data = iwl_acpi_get_object(fwrt->dev, ACPI_WGDS_METHOD);
	if (IS_ERR(data))
		return PTR_ERR(data);

	/* start by trying to read revision 2 */
	wifi_pkg = iwl_acpi_get_wifi_pkg(fwrt->dev, data,
					 ACPI_WGDS_WIFI_DATA_SIZE_REV2,
					 &tbl_rev);
	if (!IS_ERR(wifi_pkg)) {
		if (tbl_rev != 2) {
			ret = PTR_ERR(wifi_pkg);
			goto out_free;
		}

		num_bands = ACPI_GEO_NUM_BANDS_REV2;

		goto read_table;
	}

	/* then try revision 0 (which is the same as 1) */
	wifi_pkg = iwl_acpi_get_wifi_pkg(fwrt->dev, data,
					 ACPI_WGDS_WIFI_DATA_SIZE_REV0,
					 &tbl_rev);
	if (!IS_ERR(wifi_pkg)) {
		if (tbl_rev != 0 && tbl_rev != 1) {
			ret = PTR_ERR(wifi_pkg);
			goto out_free;
		}

		num_bands = ACPI_GEO_NUM_BANDS_REV0;

		goto read_table;
	}

	ret = PTR_ERR(wifi_pkg);
	goto out_free;

read_table:
	fwrt->geo_rev = tbl_rev;
	for (i = 0; i < ACPI_NUM_GEO_PROFILES; i++) {
		for (j = 0; j < ACPI_GEO_NUM_BANDS_REV2; j++) {
			union acpi_object *entry;

			/*
				fwrt->geo_profiles[i].bands[j].max =
					fwrt->geo_profiles[i].bands[1].max;
			} else {
				entry = &wifi_pkg->package.elements[idx++];
				if (entry->type != ACPI_TYPE_INTEGER ||
				    entry->integer.value > U8_MAX) {
					ret = -EINVAL;
					goto out_free;
					fwrt->geo_profiles[i].bands[j].chains[k] =
						fwrt->geo_profiles[i].bands[1].chains[k];
				} else {
					entry = &wifi_pkg->package.elements[idx++];
					if (entry->type != ACPI_TYPE_INTEGER ||
					    entry->integer.value > U8_MAX) {
						ret = -EINVAL;
						goto out_free;
bool iwl_sar_geo_support(struct iwl_fw_runtime *fwrt)
{
	/*
	 * The GEO_TX_POWER_LIMIT command is not supported on earlier
	 * firmware versions.  Unfortunately, we don't have a TLV API
	 * flag to rely on, so rely on the major version which is in
	 * the first byte of ucode_ver.  This was implemented
	 * initially on version 38 and then backported to 17.  It was
	 * also backported to 29, but only for 7265D devices.  The
	 * intention was to have it in 36 as well, but not all 8000
	 * family got this feature enabled.  The 8000 family is the
IWL_EXPORT_SYMBOL(iwl_sar_geo_support);

int iwl_sar_geo_init(struct iwl_fw_runtime *fwrt,
		     struct iwl_per_chain_offset *table, u32 n_bands)
{
	int i, j;

	if (!iwl_sar_geo_support(fwrt))
		return -EOPNOTSUPP;

	for (i = 0; i < ACPI_NUM_GEO_PROFILES; i++) {
		for (j = 0; j < n_bands; j++) {
			struct iwl_per_chain_offset *chain =
				&table[i * n_bands + j];

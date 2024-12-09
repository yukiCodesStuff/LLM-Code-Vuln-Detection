#define ACPI_SAR_PROFILE_NUM		4

#define ACPI_NUM_GEO_PROFILES		3
#define ACPI_NUM_GEO_PROFILES_REV3	8
#define ACPI_GEO_PER_CHAIN_SIZE		3

#define ACPI_SAR_NUM_CHAINS_REV0	2
#define ACPI_SAR_NUM_CHAINS_REV1	2
#define ACPI_GEO_NUM_BANDS_REV2		3
#define ACPI_GEO_NUM_CHAINS		2

#define ACPI_WRDD_WIFI_DATA_SIZE	2
#define ACPI_SPLC_WIFI_DATA_SIZE	2
#define ACPI_ECKV_WIFI_DATA_SIZE	2

	DSM_FUNC_QUERY = 0,
	DSM_FUNC_DISABLE_SRD = 1,
	DSM_FUNC_ENABLE_INDONESIA_5G2 = 2,
	DSM_FUNC_ENABLE_6E = 3,
	DSM_FUNC_11AX_ENABLEMENT = 6,
	DSM_FUNC_ENABLE_UNII4_CHAN = 7,
	DSM_FUNC_ACTIVATE_CHANNEL = 8
};

enum iwl_dsm_values_srd {
	DSM_VALUE_SRD_ACTIVE,
int iwl_acpi_get_dsm_u32(struct device *dev, int rev, int func,
			 const guid_t *guid, u32 *value);

union acpi_object *iwl_acpi_get_wifi_pkg_range(struct device *dev,
					       union acpi_object *data,
					       int min_data_size,
					       int max_data_size,
					       int *tbl_rev);
/**
 * iwl_acpi_get_mcc - read MCC from ACPI, if available
 *
 * @dev: the struct device
bool iwl_sar_geo_support(struct iwl_fw_runtime *fwrt);

int iwl_sar_geo_init(struct iwl_fw_runtime *fwrt,
		     struct iwl_per_chain_offset *table,
		     u32 n_bands, u32 n_profiles);

int iwl_acpi_get_tas(struct iwl_fw_runtime *fwrt, __le32 *block_list_array,
		     int *block_list_size);

	return -ENOENT;
}

static inline union acpi_object *
iwl_acpi_get_wifi_pkg_range(struct device *dev,
			    union acpi_object *data,
			    int min_data_size, int max_data_size,
			    int *tbl_rev)
{
	return ERR_PTR(-ENOENT);
}

}

#endif /* CONFIG_ACPI */

static inline union acpi_object *
iwl_acpi_get_wifi_pkg(struct device *dev,
		      union acpi_object *data,
		      int data_size, int *tbl_rev)
{
	return iwl_acpi_get_wifi_pkg_range(dev, data, data_size, data_size,
					   tbl_rev);
}

#endif /* __iwl_fw_acpi__ */
			found = 1;
	}

	if (!found) /* fallback to defaults */
		radeon_legacy_get_primary_dac_info_from_table(rdev, p_dac);

	return p_dac;
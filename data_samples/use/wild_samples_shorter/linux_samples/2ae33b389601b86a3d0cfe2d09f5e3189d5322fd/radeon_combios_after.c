			found = 1;
	}

	/* quirks */
	/* Radeon 9100 (R200) */
	if ((dev->pdev->device == 0x514D) &&
	    (dev->pdev->subsystem_vendor == 0x174B) &&
	    (dev->pdev->subsystem_device == 0x7149)) {
		/* vbios value is bad, use the default */
		found = 0;
	}

	if (!found) /* fallback to defaults */
		radeon_legacy_get_primary_dac_info_from_table(rdev, p_dac);

	return p_dac;
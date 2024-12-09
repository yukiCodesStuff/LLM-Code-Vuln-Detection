{
	int rc;

	rc = tpm1_chip_register(chip);
	if (rc)
		return rc;

	tpm_add_ppi(chip);

	rc = tpm_dev_add_device(chip);
	if (rc)
		goto out_err;

	if (!(chip->flags & TPM_CHIP_FLAG_TPM2)) {
		rc = __compat_only_sysfs_link_entry_to_kobj(&chip->pdev->kobj,
							    &chip->dev.kobj,
							    "ppi");
		if (rc)
			goto out_err;
	}
	if (rc)
		goto out_err;

	if (!(chip->flags & TPM_CHIP_FLAG_TPM2)) {
		rc = __compat_only_sysfs_link_entry_to_kobj(&chip->pdev->kobj,
							    &chip->dev.kobj,
							    "ppi");
		if (rc)
			goto out_err;
	}

	/* Make the chip available. */
	spin_lock(&driver_lock);
	list_add_rcu(&chip->list, &tpm_chip_list);
	spin_unlock(&driver_lock);

	chip->flags |= TPM_CHIP_FLAG_REGISTERED;

	return 0;
out_err:
	tpm1_chip_unregister(chip);
	return rc;
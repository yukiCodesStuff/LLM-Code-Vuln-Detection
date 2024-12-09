
	if (wait_startup(chip, 0) != 0) {
		rc = -ENODEV;
		goto out_err;
	}

	/* Take control of the TPM's interrupt hardware and shut it off */
	rc = tpm_tis_read32(priv, TPM_INT_ENABLE(priv->locality), &intmask);
	if (rc < 0)
		goto out_err;

	intmask |= TPM_INTF_CMD_READY_INT | TPM_INTF_LOCALITY_CHANGE_INT |
		   TPM_INTF_DATA_AVAIL_INT | TPM_INTF_STS_VALID_INT;
	intmask &= ~TPM_GLOBAL_INT_ENABLE;

	rc = tpm_chip_start(chip);
	if (rc)
		goto out_err;
	rc = tpm2_probe(chip);
	tpm_chip_stop(chip);
	if (rc)
		goto out_err;

	rc = tpm_tis_read32(priv, TPM_DID_VID(0), &vendor);
	if (rc < 0)
		goto out_err;

	priv->manufacturer_id = vendor;

	rc = tpm_tis_read8(priv, TPM_RID(0), &rid);
	if (rc < 0)
		goto out_err;

	dev_info(dev, "%s TPM (device-id 0x%X, rev-id %d)\n",
		 (chip->flags & TPM_CHIP_FLAG_TPM2) ? "2.0" : "1.2",
		 vendor >> 16, rid);
	probe = probe_itpm(chip);
	if (probe < 0) {
		rc = -ENODEV;
		goto out_err;
	}

	/* Figure out the capabilities */
	rc = tpm_tis_read32(priv, TPM_INTF_CAPS(priv->locality), &intfcaps);
	if (rc < 0)
		goto out_err;

	dev_dbg(dev, "TPM interface capabilities (0x%x):\n",
		intfcaps);
	if (intfcaps & TPM_INTF_BURST_COUNT_STATIC)
		if (tpm_get_timeouts(chip)) {
			dev_err(dev, "Could not get TPM timeouts and durations\n");
			rc = -ENODEV;
			goto out_err;
		}

		if (irq) {
			tpm_tis_probe_irq_single(chip, intmask, IRQF_SHARED,
						 irq);
			if (!(chip->flags & TPM_CHIP_FLAG_IRQ))
		}
	}

	rc = tpm_chip_register(chip);
	if (rc)
		goto out_err;

	if (chip->ops->clk_enable != NULL)
		chip->ops->clk_enable(chip, false);

	return 0;
out_err:
	if ((chip->ops != NULL) && (chip->ops->clk_enable != NULL))
		chip->ops->clk_enable(chip, false);

	tpm_tis_remove(chip);
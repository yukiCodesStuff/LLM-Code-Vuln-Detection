	if (wait_startup(chip, 0) != 0) {
		rc = -ENODEV;
		goto err_start;
	}
	if (wait_startup(chip, 0) != 0) {
		rc = -ENODEV;
		goto err_start;
	}

	/* Take control of the TPM's interrupt hardware and shut it off */
	rc = tpm_tis_read32(priv, TPM_INT_ENABLE(priv->locality), &intmask);
	if (rc < 0)
		goto err_start;

	intmask |= TPM_INTF_CMD_READY_INT | TPM_INTF_LOCALITY_CHANGE_INT |
		   TPM_INTF_DATA_AVAIL_INT | TPM_INTF_STS_VALID_INT;
	intmask &= ~TPM_GLOBAL_INT_ENABLE;
	tpm_tis_write32(priv, TPM_INT_ENABLE(priv->locality), intmask);

	rc = tpm_chip_start(chip);
	if (rc)
		goto err_start;

	rc = tpm2_probe(chip);
	if (rc)
		goto err_probe;

	rc = tpm_tis_read32(priv, TPM_DID_VID(0), &vendor);
	if (rc < 0)
		goto err_probe;

	priv->manufacturer_id = vendor;

	rc = tpm_tis_read8(priv, TPM_RID(0), &rid);
	if (rc < 0)
		goto err_probe;

	dev_info(dev, "%s TPM (device-id 0x%X, rev-id %d)\n",
		 (chip->flags & TPM_CHIP_FLAG_TPM2) ? "2.0" : "1.2",
		 vendor >> 16, rid);

	probe = probe_itpm(chip);
	if (probe < 0) {
		rc = -ENODEV;
		goto err_probe;
	}
	if (probe < 0) {
		rc = -ENODEV;
		goto err_probe;
	}
		if (tpm_get_timeouts(chip)) {
			dev_err(dev, "Could not get TPM timeouts and durations\n");
			rc = -ENODEV;
			goto err_probe;
		}

		chip->flags |= TPM_CHIP_FLAG_IRQ;
		if (irq) {
			tpm_tis_probe_irq_single(chip, intmask, IRQF_SHARED,
						 irq);
			if (!(chip->flags & TPM_CHIP_FLAG_IRQ))
				dev_err(&chip->dev, FW_BUG
					"TPM interrupt not working, polling instead\n");
		} else {
			tpm_tis_probe_irq(chip, intmask);
		}
		} else {
			tpm_tis_probe_irq(chip, intmask);
		}
	}

	tpm_chip_stop(chip);

	rc = tpm_chip_register(chip);
	if (rc)
		goto err_start;

	return 0;

err_probe:
	tpm_chip_stop(chip);

err_start:
	if ((chip->ops != NULL) && (chip->ops->clk_enable != NULL))
		chip->ops->clk_enable(chip, false);

	tpm_tis_remove(chip);

	return rc;
}
EXPORT_SYMBOL_GPL(tpm_tis_core_init);

#ifdef CONFIG_PM_SLEEP
static void tpm_tis_reenable_interrupts(struct tpm_chip *chip)
{
	struct tpm_tis_data *priv = dev_get_drvdata(&chip->dev);
	u32 intmask;
	int rc;

	if (chip->ops->clk_enable != NULL)
		chip->ops->clk_enable(chip, true);

	/* reenable interrupts that device may have lost or
	 * BIOS/firmware may have disabled
	 */
	rc = tpm_tis_write8(priv, TPM_INT_VECTOR(priv->locality), priv->irq);
	if (rc < 0)
		goto out;

	rc = tpm_tis_read32(priv, TPM_INT_ENABLE(priv->locality), &intmask);
	if (rc < 0)
		goto out;

	intmask |= TPM_INTF_CMD_READY_INT
	    | TPM_INTF_LOCALITY_CHANGE_INT | TPM_INTF_DATA_AVAIL_INT
	    | TPM_INTF_STS_VALID_INT | TPM_GLOBAL_INT_ENABLE;

	tpm_tis_write32(priv, TPM_INT_ENABLE(priv->locality), intmask);

out:
	if (chip->ops->clk_enable != NULL)
		chip->ops->clk_enable(chip, false);

	return;
}

int tpm_tis_resume(struct device *dev)
{
	struct tpm_chip *chip = dev_get_drvdata(dev);
	int ret;

	if (chip->flags & TPM_CHIP_FLAG_IRQ)
		tpm_tis_reenable_interrupts(chip);

	ret = tpm_pm_resume(dev);
	if (ret)
		return ret;

	/* TPM 1.2 requires self-test on resume. This function actually returns
	 * an error code but for unknown reason it isn't handled.
	 */
	if (!(chip->flags & TPM_CHIP_FLAG_TPM2))
		tpm1_do_selftest(chip);

	return 0;
}
EXPORT_SYMBOL_GPL(tpm_tis_resume);
#endif

MODULE_AUTHOR("Leendert van Doorn (leendert@watson.ibm.com)");
MODULE_DESCRIPTION("TPM Driver");
MODULE_VERSION("2.0");
MODULE_LICENSE("GPL");
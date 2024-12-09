{
	struct pci_dev *pdev = to_pci_dev(device);
	struct ieee80211_hw *hw = pci_get_drvdata(pdev);
	struct ath_softc *sc = hw->priv;

	if (sc->wow_enabled)
		return 0;

	/* The device has to be moved to FULLSLEEP forcibly.
	 * Otherwise the chip never moved to full sleep,
	 * when no interface is up.
	 */
	ath9k_hw_disable(sc->sc_ah);
	ath9k_hw_setpower(sc->sc_ah, ATH9K_PM_FULL_SLEEP);

	return 0;
}

static int ath_pci_resume(struct device *device)
{
	struct pci_dev *pdev = to_pci_dev(device);
	u32 val;

	/*
	 * Suspend/Resume resets the PCI configuration space, so we have to
	 * re-disable the RETRY_TIMEOUT register (0x41) to keep
	 * PCI Tx retries from interfering with C3 CPU state
	 */
	pci_read_config_dword(pdev, 0x40, &val);
	if ((val & 0x0000ff00) != 0)
		pci_write_config_dword(pdev, 0x40, val & 0xffff00ff);

	return 0;
}

static const struct dev_pm_ops ath9k_pm_ops = {
	.suspend = ath_pci_suspend,
	.resume = ath_pci_resume,
	.freeze = ath_pci_suspend,
	.thaw = ath_pci_resume,
	.poweroff = ath_pci_suspend,
	.restore = ath_pci_resume,
};

#define ATH9K_PM_OPS	(&ath9k_pm_ops)

#else /* !CONFIG_PM */

#define ATH9K_PM_OPS	NULL

#endif /* !CONFIG_PM */


MODULE_DEVICE_TABLE(pci, ath_pci_id_table);

static struct pci_driver ath_pci_driver = {
	.name       = "ath9k",
	.id_table   = ath_pci_id_table,
	.probe      = ath_pci_probe,
	.remove     = ath_pci_remove,
	.driver.pm  = ATH9K_PM_OPS,
};

int ath_pci_init(void)
{
	return pci_register_driver(&ath_pci_driver);
}

void ath_pci_exit(void)
{
	pci_unregister_driver(&ath_pci_driver);
}

MODULE_DEVICE_TABLE(pci, ath10k_pci_id_table);

#ifdef CONFIG_PM

static int ath10k_pci_pm_suspend(struct device *dev)
{
	struct ath10k *ar = dev_get_drvdata(dev);
	int ret;

	return ret;
}

static int ath10k_pci_pm_resume(struct device *dev)
{
	struct ath10k *ar = dev_get_drvdata(dev);
	int ret;

static SIMPLE_DEV_PM_OPS(ath10k_pci_pm_ops,
			 ath10k_pci_pm_suspend,
			 ath10k_pci_pm_resume);
#endif

static struct pci_driver ath10k_pci_driver = {
	.name = "ath10k_pci",
	.id_table = ath10k_pci_id_table,
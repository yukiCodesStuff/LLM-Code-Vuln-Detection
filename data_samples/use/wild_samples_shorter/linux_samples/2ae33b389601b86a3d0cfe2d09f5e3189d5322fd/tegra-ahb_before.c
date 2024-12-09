EXPORT_SYMBOL(tegra_ahb_enable_smmu);
#endif

#ifdef CONFIG_PM_SLEEP
static int tegra_ahb_suspend(struct device *dev)
{
	int i;
	struct tegra_ahb *ahb = dev_get_drvdata(dev);
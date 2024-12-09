{
	struct device *dev;
	u32 val;
	struct tegra_ahb *ahb;

	dev = driver_find_device(&tegra_ahb_driver.driver, NULL, dn,
				 tegra_ahb_match_by_smmu);
	if (!dev)
		return -EPROBE_DEFER;
	ahb = dev_get_drvdata(dev);
	val = gizmo_readl(ahb, AHB_ARBITRATION_XBAR_CTRL);
	val |= AHB_ARBITRATION_XBAR_CTRL_SMMU_INIT_DONE;
	gizmo_writel(ahb, val, AHB_ARBITRATION_XBAR_CTRL);
	return 0;
}
EXPORT_SYMBOL(tegra_ahb_enable_smmu);
#endif

#ifdef CONFIG_PM_SLEEP
static int tegra_ahb_suspend(struct device *dev)
{
	int i;
	struct tegra_ahb *ahb = dev_get_drvdata(dev);

	for (i = 0; i < ARRAY_SIZE(tegra_ahb_gizmo); i++)
		ahb->ctx[i] = gizmo_readl(ahb, tegra_ahb_gizmo[i]);
	return 0;
}
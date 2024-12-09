	return 0;
}

#ifdef CONFIG_PM
static int omap_i2c_runtime_suspend(struct device *dev)
{
	struct omap_i2c_dev *omap = dev_get_drvdata(dev);

	omap->iestate = omap_i2c_read_reg(omap, OMAP_I2C_IE_REG);
	return 0;
}

static int omap_i2c_runtime_resume(struct device *dev)
{
	struct omap_i2c_dev *omap = dev_get_drvdata(dev);

	pinctrl_pm_select_default_state(dev);
}

static const struct dev_pm_ops omap_i2c_pm_ops = {
	SET_RUNTIME_PM_OPS(omap_i2c_runtime_suspend,
			   omap_i2c_runtime_resume, NULL)
};
#define OMAP_I2C_PM_OPS (&omap_i2c_pm_ops)
#else
#define OMAP_I2C_PM_OPS NULL
#endif /* CONFIG_PM */

static struct platform_driver omap_i2c_driver = {
	.probe		= omap_i2c_probe,
	.remove		= omap_i2c_remove,
	.driver		= {
		.name	= "omap_i2c",
		.pm	= OMAP_I2C_PM_OPS,
		.of_match_table = of_match_ptr(omap_i2c_of_match),
	},
};

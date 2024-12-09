{
	struct omap_i2c_dev	*omap = platform_get_drvdata(pdev);
	int ret;

	i2c_del_adapter(&omap->adapter);
	ret = pm_runtime_get_sync(&pdev->dev);
	if (ret < 0)
		return ret;

	omap_i2c_write_reg(omap, OMAP_I2C_CON_REG, 0);
	pm_runtime_dont_use_autosuspend(&pdev->dev);
	pm_runtime_put_sync(&pdev->dev);
	pm_runtime_disable(&pdev->dev);
	return 0;
}

#ifdef CONFIG_PM
static int omap_i2c_runtime_suspend(struct device *dev)
{
	struct omap_i2c_dev *omap = dev_get_drvdata(dev);

	omap->iestate = omap_i2c_read_reg(omap, OMAP_I2C_IE_REG);

	if (omap->scheme == OMAP_I2C_SCHEME_0)
		omap_i2c_write_reg(omap, OMAP_I2C_IE_REG, 0);
	else
		omap_i2c_write_reg(omap, OMAP_I2C_IP_V2_IRQENABLE_CLR,
				   OMAP_I2C_IP_V2_INTERRUPTS_MASK);

	if (omap->rev < OMAP_I2C_OMAP1_REV_2) {
		omap_i2c_read_reg(omap, OMAP_I2C_IV_REG); /* Read clears */
	} else {
		omap_i2c_write_reg(omap, OMAP_I2C_STAT_REG, omap->iestate);

		/* Flush posted write */
		omap_i2c_read_reg(omap, OMAP_I2C_STAT_REG);
	}

	pinctrl_pm_select_sleep_state(dev);

	return 0;
}
	} else {
		omap_i2c_write_reg(omap, OMAP_I2C_STAT_REG, omap->iestate);

		/* Flush posted write */
		omap_i2c_read_reg(omap, OMAP_I2C_STAT_REG);
	}

	pinctrl_pm_select_sleep_state(dev);

	return 0;
}

static int omap_i2c_runtime_resume(struct device *dev)
{
	struct omap_i2c_dev *omap = dev_get_drvdata(dev);

	pinctrl_pm_select_default_state(dev);

	if (!omap->regs)
		return 0;

	__omap_i2c_init(omap);

	return 0;
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

/* I2C may be needed to bring up other drivers */
static int __init
omap_i2c_init_driver(void)
{
	return platform_driver_register(&omap_i2c_driver);
}
subsys_initcall(omap_i2c_init_driver);

static void __exit omap_i2c_exit_driver(void)
{
	platform_driver_unregister(&omap_i2c_driver);
}
module_exit(omap_i2c_exit_driver);

MODULE_AUTHOR("MontaVista Software, Inc. (and others)");
MODULE_DESCRIPTION("TI OMAP I2C bus adapter");
MODULE_LICENSE("GPL");
MODULE_ALIAS("platform:omap_i2c");
{
	struct omap_i2c_dev *omap = dev_get_drvdata(dev);

	pinctrl_pm_select_default_state(dev);

	if (!omap->regs)
		return 0;

	__omap_i2c_init(omap);

	return 0;
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

/* I2C may be needed to bring up other drivers */
static int __init
omap_i2c_init_driver(void)
{
	return platform_driver_register(&omap_i2c_driver);
}
subsys_initcall(omap_i2c_init_driver);

static void __exit omap_i2c_exit_driver(void)
{
	platform_driver_unregister(&omap_i2c_driver);
}
module_exit(omap_i2c_exit_driver);

MODULE_AUTHOR("MontaVista Software, Inc. (and others)");
MODULE_DESCRIPTION("TI OMAP I2C bus adapter");
MODULE_LICENSE("GPL");
MODULE_ALIAS("platform:omap_i2c");
{
	struct imx7ulp_wdt_device *wdt = watchdog_get_drvdata(wdog);

	imx7ulp_wdt_enable(wdog, true);
	imx7ulp_wdt_set_timeout(&wdt->wdd, 1);

	/* wait for wdog to fire */
	while (true)
		;

	return NOTIFY_DONE;
}

static const struct watchdog_ops imx7ulp_wdt_ops = {
	.owner = THIS_MODULE,
	.start = imx7ulp_wdt_start,
	.stop  = imx7ulp_wdt_stop,
	.ping  = imx7ulp_wdt_ping,
	.set_timeout = imx7ulp_wdt_set_timeout,
	.restart = imx7ulp_wdt_restart,
};

static const struct watchdog_info imx7ulp_wdt_info = {
	.identity = "i.MX7ULP watchdog timer",
	.options  = WDIOF_SETTIMEOUT | WDIOF_KEEPALIVEPING |
		    WDIOF_MAGICCLOSE,
};

static void imx7ulp_wdt_init(void __iomem *base, unsigned int timeout)
{
	u32 val;

	/* unlock the wdog for reconfiguration */
	writel_relaxed(UNLOCK_SEQ0, base + WDOG_CNT);
	writel_relaxed(UNLOCK_SEQ1, base + WDOG_CNT);

	/* set an initial timeout value in TOVAL */
	writel(timeout, base + WDOG_TOVAL);
	/* enable 32bit command sequence and reconfigure */
	val = WDOG_CS_CMD32EN | WDOG_CS_CLK | WDOG_CS_UPDATE;
	writel(val, base + WDOG_CS);
}

static void imx7ulp_wdt_action(void *data)
{
	clk_disable_unprepare(data);
}

static int imx7ulp_wdt_probe(struct platform_device *pdev)
{
	struct imx7ulp_wdt_device *imx7ulp_wdt;
	struct device *dev = &pdev->dev;
	struct watchdog_device *wdog;
	int ret;

	imx7ulp_wdt = devm_kzalloc(dev, sizeof(*imx7ulp_wdt), GFP_KERNEL);
	if (!imx7ulp_wdt)
		return -ENOMEM;

	platform_set_drvdata(pdev, imx7ulp_wdt);

	imx7ulp_wdt->base = devm_platform_ioremap_resource(pdev, 0);
	if (IS_ERR(imx7ulp_wdt->base))
		return PTR_ERR(imx7ulp_wdt->base);

	imx7ulp_wdt->clk = devm_clk_get(dev, NULL);
	if (IS_ERR(imx7ulp_wdt->clk)) {
		dev_err(dev, "Failed to get watchdog clock\n");
		return PTR_ERR(imx7ulp_wdt->clk);
	}

	ret = clk_prepare_enable(imx7ulp_wdt->clk);
	if (ret)
		return ret;

	ret = devm_add_action_or_reset(dev, imx7ulp_wdt_action, imx7ulp_wdt->clk);
	if (ret)
		return ret;

	wdog = &imx7ulp_wdt->wdd;
	wdog->info = &imx7ulp_wdt_info;
	wdog->ops = &imx7ulp_wdt_ops;
	wdog->min_timeout = 1;
	wdog->max_timeout = MAX_TIMEOUT;
	wdog->parent = dev;
	wdog->timeout = DEFAULT_TIMEOUT;

	watchdog_init_timeout(wdog, 0, dev);
	watchdog_stop_on_reboot(wdog);
	watchdog_stop_on_unregister(wdog);
	watchdog_set_drvdata(wdog, imx7ulp_wdt);
	imx7ulp_wdt_init(imx7ulp_wdt->base, wdog->timeout * WDOG_CLOCK_RATE);

	return devm_watchdog_register_device(dev, wdog);
}

static int __maybe_unused imx7ulp_wdt_suspend(struct device *dev)
{
	struct imx7ulp_wdt_device *imx7ulp_wdt = dev_get_drvdata(dev);

	if (watchdog_active(&imx7ulp_wdt->wdd))
		imx7ulp_wdt_stop(&imx7ulp_wdt->wdd);

	clk_disable_unprepare(imx7ulp_wdt->clk);

	return 0;
}

static int __maybe_unused imx7ulp_wdt_resume(struct device *dev)
{
	struct imx7ulp_wdt_device *imx7ulp_wdt = dev_get_drvdata(dev);
	u32 timeout = imx7ulp_wdt->wdd.timeout * WDOG_CLOCK_RATE;
	int ret;

	ret = clk_prepare_enable(imx7ulp_wdt->clk);
	if (ret)
		return ret;

	if (imx7ulp_wdt_is_enabled(imx7ulp_wdt->base))
		imx7ulp_wdt_init(imx7ulp_wdt->base, timeout);

	if (watchdog_active(&imx7ulp_wdt->wdd))
		imx7ulp_wdt_start(&imx7ulp_wdt->wdd);

	return 0;
}

static SIMPLE_DEV_PM_OPS(imx7ulp_wdt_pm_ops, imx7ulp_wdt_suspend,
			 imx7ulp_wdt_resume);

static const struct of_device_id imx7ulp_wdt_dt_ids[] = {
	{ .compatible = "fsl,imx7ulp-wdt", },
	{ /* sentinel */ }
};
MODULE_DEVICE_TABLE(of, imx7ulp_wdt_dt_ids);

static struct platform_driver imx7ulp_wdt_driver = {
	.probe		= imx7ulp_wdt_probe,
	.driver		= {
		.name	= "imx7ulp-wdt",
		.pm	= &imx7ulp_wdt_pm_ops,
		.of_match_table = imx7ulp_wdt_dt_ids,
	},
};
module_platform_driver(imx7ulp_wdt_driver);

MODULE_AUTHOR("Anson Huang <Anson.Huang@nxp.com>");
MODULE_DESCRIPTION("Freescale i.MX7ULP watchdog driver");
MODULE_LICENSE("GPL v2");
{
	struct imx7ulp_wdt_device *wdt = watchdog_get_drvdata(wdog);

	imx7ulp_wdt_enable(wdt->base, true);
	imx7ulp_wdt_set_timeout(&wdt->wdd, 1);

	/* wait for wdog to fire */
	while (true)
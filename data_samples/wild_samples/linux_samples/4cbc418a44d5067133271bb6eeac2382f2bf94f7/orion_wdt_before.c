	if (ret) {
		dev_err(&pdev->dev, "cannot initialize clock\n");
		return ret;
	}

	wdt_max_duration = WDT_MAX_CYCLE_COUNT / dev->clk_rate;

	dev->wdt.timeout = wdt_max_duration;
	dev->wdt.max_timeout = wdt_max_duration;
	dev->wdt.parent = &pdev->dev;
	watchdog_init_timeout(&dev->wdt, heartbeat, &pdev->dev);

	platform_set_drvdata(pdev, &dev->wdt);
	watchdog_set_drvdata(&dev->wdt, dev);

	/*
	 * Let's make sure the watchdog is fully stopped, unless it's
	 * explicitly enabled. This may be the case if the module was
	 * removed and re-inserted, or if the bootloader explicitly
	 * set a running watchdog before booting the kernel.
	 */
	if (!orion_wdt_enabled(&dev->wdt))
		orion_wdt_stop(&dev->wdt);
	else
		set_bit(WDOG_HW_RUNNING, &dev->wdt.status);

	/* Request the IRQ only after the watchdog is disabled */
	irq = platform_get_irq(pdev, 0);
	if (irq > 0) {
		/*
		 * Not all supported platforms specify an interrupt for the
		 * watchdog, so let's make it optional.
		 */
		ret = devm_request_irq(&pdev->dev, irq, orion_wdt_irq, 0,
				       pdev->name, dev);
		if (ret < 0) {
			dev_err(&pdev->dev, "failed to request IRQ\n");
			goto disable_clk;
		}
	}
		if (ret < 0) {
			dev_err(&pdev->dev, "failed to request IRQ\n");
			goto disable_clk;
		}
	}

	/* Optional 2nd interrupt for pretimeout */
	irq = platform_get_irq(pdev, 1);
	if (irq > 0) {
		orion_wdt_info.options |= WDIOF_PRETIMEOUT;
		ret = devm_request_irq(&pdev->dev, irq, orion_wdt_pre_irq,
				       0, pdev->name, dev);
		if (ret < 0) {
			dev_err(&pdev->dev, "failed to request IRQ\n");
			goto disable_clk;
		}
	}
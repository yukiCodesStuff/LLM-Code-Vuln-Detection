		set_bit(WDOG_HW_RUNNING, &dev->wdt.status);

	/* Request the IRQ only after the watchdog is disabled */
	irq = platform_get_irq(pdev, 0);
	if (irq > 0) {
		/*
		 * Not all supported platforms specify an interrupt for the
		 * watchdog, so let's make it optional.
	}

	/* Optional 2nd interrupt for pretimeout */
	irq = platform_get_irq(pdev, 1);
	if (irq > 0) {
		orion_wdt_info.options |= WDIOF_PRETIMEOUT;
		ret = devm_request_irq(&pdev->dev, irq, orion_wdt_pre_irq,
				       0, pdev->name, dev);
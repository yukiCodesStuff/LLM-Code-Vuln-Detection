	err = gpiochip_add(gc);
	if (err) {
		dev_err(&ofdev->dev, "Could not add gpiochip\n");
		if (priv->domain)
			irq_domain_remove(priv->domain);
		return err;
	}

	dev_info(&ofdev->dev, "regs=0x%p, base=%d, ngpio=%d, irqs=%s\n",
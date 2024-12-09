	if (err) {
		dev_err(&ofdev->dev, "Could not add gpiochip\n");
		if (priv->domain)
			irq_domain_remove(priv->domain);
		return err;
	}
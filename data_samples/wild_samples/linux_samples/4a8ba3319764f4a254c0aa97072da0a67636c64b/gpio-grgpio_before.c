	if (err) {
		dev_err(&ofdev->dev, "Could not add gpiochip\n");
		irq_domain_remove(priv->domain);
		return err;
	}
	dev_dbg(&dev->dev, "PCI INT %c disabled\n", pin_name(pin));
	if (gsi >= 0) {
		acpi_unregister_gsi(gsi);
		dev->irq = 0;
		dev->irq_managed = 0;
	}
}
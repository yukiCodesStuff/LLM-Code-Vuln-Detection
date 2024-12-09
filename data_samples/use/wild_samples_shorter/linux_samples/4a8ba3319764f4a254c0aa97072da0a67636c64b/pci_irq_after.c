	dev_dbg(&dev->dev, "PCI INT %c disabled\n", pin_name(pin));
	if (gsi >= 0) {
		acpi_unregister_gsi(gsi);
		dev->irq_managed = 0;
	}
}
	if (gsi >= 0) {
		acpi_unregister_gsi(gsi);
		dev->irq_managed = 0;
	}
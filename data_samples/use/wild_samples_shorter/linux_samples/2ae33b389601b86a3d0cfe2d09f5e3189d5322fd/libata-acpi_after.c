		return -ENODEV;
}

static struct acpi_bus_type ata_acpi_bus = {
	.name = "ATA",
	.find_device = ata_acpi_find_device,
};

int ata_acpi_register(void)
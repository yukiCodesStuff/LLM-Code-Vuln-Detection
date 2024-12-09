	else if (scsi_is_sdev_device(dev)) {
		struct scsi_device *sdev = to_scsi_device(dev);

		return ata_acpi_bind_device(ap, sdev, handle);
	} else
		return -ENODEV;
}

static int ata_acpi_find_dummy(struct device *dev, acpi_handle *handle)
{
	return -ENODEV;
}

static struct acpi_bus_type ata_acpi_bus = {
	.find_bridge = ata_acpi_find_dummy,
	.find_device = ata_acpi_find_device,
};

int ata_acpi_register(void)
{
	return scsi_register_acpi_bus_type(&ata_acpi_bus);
}
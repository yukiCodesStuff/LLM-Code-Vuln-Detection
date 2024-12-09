
extern int (*__acpi_register_gsi)(struct device *dev, u32 gsi,
				  int trigger, int polarity);

static inline void disable_acpi(void)
{
	acpi_disabled = 1;
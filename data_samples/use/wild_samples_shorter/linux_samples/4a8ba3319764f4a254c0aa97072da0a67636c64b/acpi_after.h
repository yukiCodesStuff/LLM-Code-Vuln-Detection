
extern int (*__acpi_register_gsi)(struct device *dev, u32 gsi,
				  int trigger, int polarity);
extern void (*__acpi_unregister_gsi)(u32 gsi);

static inline void disable_acpi(void)
{
	acpi_disabled = 1;
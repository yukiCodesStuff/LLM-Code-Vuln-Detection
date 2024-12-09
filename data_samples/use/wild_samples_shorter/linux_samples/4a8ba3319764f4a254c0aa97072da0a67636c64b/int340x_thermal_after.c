
#include "internal.h"

#define INT3401_DEVICE 0X01
static const struct acpi_device_id int340x_thermal_device_ids[] = {
	{"INT3400"},
	{"INT3401", INT3401_DEVICE},
	{"INT3402"},
	{"INT3403"},
	{"INT3404"},
	{"INT3406"},
					const struct acpi_device_id *id)
{
#if defined(CONFIG_INT340X_THERMAL) || defined(CONFIG_INT340X_THERMAL_MODULE)
	acpi_create_platform_device(adev);
#elif defined(INTEL_SOC_DTS_THERMAL) || defined(INTEL_SOC_DTS_THERMAL_MODULE)
	/* Intel SoC DTS thermal driver needs INT3401 to set IRQ descriptor */
	if (id->driver_data == INT3401_DEVICE)
		acpi_create_platform_device(adev);
#endif
	return 1;
}
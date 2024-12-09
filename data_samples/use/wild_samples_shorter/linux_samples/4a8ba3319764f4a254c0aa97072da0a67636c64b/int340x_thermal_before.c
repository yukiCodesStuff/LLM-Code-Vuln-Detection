
#include "internal.h"

#define DO_ENUMERATION 0x01
static const struct acpi_device_id int340x_thermal_device_ids[] = {
	{"INT3400", DO_ENUMERATION },
	{"INT3401"},
	{"INT3402"},
	{"INT3403"},
	{"INT3404"},
	{"INT3406"},
					const struct acpi_device_id *id)
{
#if defined(CONFIG_INT340X_THERMAL) || defined(CONFIG_INT340X_THERMAL_MODULE)
	if (id->driver_data == DO_ENUMERATION)
		acpi_create_platform_device(adev);
#endif
	return 1;
}
{
#ifndef CONFIG_LEFI_FIRMWARE_INTERFACE
	mach_prepare_shutdown();
	unreachable();
#else
	void (*fw_poweroff)(void) = (void *)loongson_sysconf.poweroff_addr;

	fw_poweroff();
	while (1) {
		if (cpu_wait)
			cpu_wait();
	}
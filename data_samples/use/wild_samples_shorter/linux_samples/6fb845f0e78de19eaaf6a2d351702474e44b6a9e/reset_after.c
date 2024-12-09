{
#ifndef CONFIG_LEFI_FIRMWARE_INTERFACE
	mach_prepare_shutdown();

	/*
	 * It needs a wait loop here, but mips/kernel/reset.c already calls
	 * a generic delay loop, machine_hang(), so simply return.
	 */
	return;
#else
	void (*fw_poweroff)(void) = (void *)loongson_sysconf.poweroff_addr;

	fw_poweroff();

	cpus_read_unlock();

	/* Possibly switch to a new RFI flush type */
	pseries_setup_rfi_flush();

	/* Reinitialise system information for hv-24x7 */
	read_24x7_sys_info();


	cpus_read_unlock();

	/* Possibly switch to a new L1 flush type */
	pseries_setup_security_mitigations();

	/* Reinitialise system information for hv-24x7 */
	read_24x7_sys_info();

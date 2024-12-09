		return 0;
#endif

	BUG_ON(pr->id >= nr_cpu_ids);

	/*
	 * Buggy BIOS check
	 * ACPI id of processors can be reported wrongly by the BIOS.
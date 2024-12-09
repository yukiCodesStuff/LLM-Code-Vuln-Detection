	if (result) {
		/* Processor is physically not present */
		return 0;
	}

#ifdef CONFIG_SMP
	if (pr->id >= setup_max_cpus && pr->id != 0)
		return 0;
#endif

	BUG_ON(pr->id >= nr_cpu_ids);

	/*
	 * Buggy BIOS check
	 * ACPI id of processors can be reported wrongly by the BIOS.
	 * Don't trust it blindly
	 */
	if (per_cpu(processor_device_array, pr->id) != NULL &&
	    per_cpu(processor_device_array, pr->id) != device) {
		dev_warn(&device->dev,
			"BIOS reported wrong ACPI id %d for the processor\n",
			pr->id);
		result = -ENODEV;
		goto err_free_cpumask;
	}
	acpi_status status;
	int ret;

	if (pr->phys_id == -1)
		return -ENODEV;

	status = acpi_evaluate_integer(pr->handle, "_STA", NULL, &sta);
	if (ACPI_FAILURE(status) || !(sta & ACPI_STA_DEVICE_PRESENT))
	cpu_maps_update_begin();
	cpu_hotplug_begin();

	ret = acpi_map_cpu(pr->handle, pr->phys_id, &pr->id);
	if (ret)
		goto out;

	ret = arch_register_cpu(pr->id);
	if (ret) {
		acpi_unmap_cpu(pr->id);
		goto out;
	}

	/*
	union acpi_object object = { 0 };
	struct acpi_buffer buffer = { sizeof(union acpi_object), &object };
	struct acpi_processor *pr = acpi_driver_data(device);
	int phys_id, cpu_index, device_declaration = 0;
	acpi_status status = AE_OK;
	static int cpu0_initialized;
	unsigned long long value;

		pr->acpi_id = value;
	}

	phys_id = acpi_get_phys_id(pr->handle, device_declaration, pr->acpi_id);
	if (phys_id < 0)
		acpi_handle_debug(pr->handle, "failed to get CPU physical ID.\n");
	pr->phys_id = phys_id;

	cpu_index = acpi_map_cpuid(pr->phys_id, pr->acpi_id);
	if (!cpu0_initialized && !acpi_has_cpu_in_madt()) {
		cpu0_initialized = 1;
		/*
		 * Handle UP system running SMP kernel, with no CPU
		 * entry in MADT
		 */
		if ((cpu_index == -1) && (num_online_cpus() == 1))
			cpu_index = 0;
	}
	pr->id = cpu_index;

	/* Remove the CPU. */
	arch_unregister_cpu(pr->id);
	acpi_unmap_cpu(pr->id);

	cpu_hotplug_done();
	cpu_maps_update_done();

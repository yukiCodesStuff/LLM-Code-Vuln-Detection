{
	unsigned long long sta;
	acpi_status status;
	int ret;

	if (pr->apic_id == -1)
		return -ENODEV;

	status = acpi_evaluate_integer(pr->handle, "_STA", NULL, &sta);
	if (ACPI_FAILURE(status) || !(sta & ACPI_STA_DEVICE_PRESENT))
		return -ENODEV;

	cpu_maps_update_begin();
	cpu_hotplug_begin();

	ret = acpi_map_lsapic(pr->handle, pr->apic_id, &pr->id);
	if (ret)
		goto out;

	ret = arch_register_cpu(pr->id);
	if (ret) {
		acpi_unmap_lsapic(pr->id);
		goto out;
	}
{
	unsigned long long sta;
	acpi_status status;
	int ret;

	if (pr->apic_id == -1)
		return -ENODEV;

	status = acpi_evaluate_integer(pr->handle, "_STA", NULL, &sta);
	if (ACPI_FAILURE(status) || !(sta & ACPI_STA_DEVICE_PRESENT))
		return -ENODEV;

	cpu_maps_update_begin();
	cpu_hotplug_begin();

	ret = acpi_map_lsapic(pr->handle, pr->apic_id, &pr->id);
	if (ret)
		goto out;

	ret = arch_register_cpu(pr->id);
	if (ret) {
		acpi_unmap_lsapic(pr->id);
		goto out;
	}
{
	union acpi_object object = { 0 };
	struct acpi_buffer buffer = { sizeof(union acpi_object), &object };
	struct acpi_processor *pr = acpi_driver_data(device);
	int apic_id, cpu_index, device_declaration = 0;
	acpi_status status = AE_OK;
	static int cpu0_initialized;
	unsigned long long value;

	acpi_processor_errata();

	/*
	 * Check to see if we have bus mastering arbitration control.  This
	 * is required for proper C3 usage (to maintain cache coherency).
	 */
	if (acpi_gbl_FADT.pm2_control_block && acpi_gbl_FADT.pm2_control_length) {
		pr->flags.bm_control = 1;
		ACPI_DEBUG_PRINT((ACPI_DB_INFO,
				  "Bus mastering arbitration control present\n"));
	} else
		ACPI_DEBUG_PRINT((ACPI_DB_INFO,
				  "No bus mastering arbitration control\n"));

	if (!strcmp(acpi_device_hid(device), ACPI_PROCESSOR_OBJECT_HID)) {
		/* Declared with "Processor" statement; match ProcessorID */
		status = acpi_evaluate_object(pr->handle, NULL, NULL, &buffer);
		if (ACPI_FAILURE(status)) {
			dev_err(&device->dev,
				"Failed to evaluate processor object (0x%x)\n",
				status);
			return -ENODEV;
		}

		pr->acpi_id = object.processor.proc_id;
	} else {
		/*
		 * Declared with "Device" statement; match _UID.
		 * Note that we don't handle string _UIDs yet.
		 */
		status = acpi_evaluate_integer(pr->handle, METHOD_NAME__UID,
						NULL, &value);
		if (ACPI_FAILURE(status)) {
			dev_err(&device->dev,
				"Failed to evaluate processor _UID (0x%x)\n",
				status);
			return -ENODEV;
		}
		device_declaration = 1;
		pr->acpi_id = value;
	}

	apic_id = acpi_get_apicid(pr->handle, device_declaration, pr->acpi_id);
	if (apic_id < 0)
		acpi_handle_debug(pr->handle, "failed to get CPU APIC ID.\n");
	pr->apic_id = apic_id;

	cpu_index = acpi_map_cpuid(pr->apic_id, pr->acpi_id);
	if (!cpu0_initialized && !acpi_has_cpu_in_madt()) {
		cpu0_initialized = 1;
		/* Handle UP system running SMP kernel, with no LAPIC in MADT */
		if ((cpu_index == -1) && (num_online_cpus() == 1))
			cpu_index = 0;
	}
	pr->id = cpu_index;

	/*
	 *  Extra Processor objects may be enumerated on MP systems with
	 *  less than the max # of CPUs. They should be ignored _iff
	 *  they are physically not present.
	 */
	if (pr->id == -1) {
		int ret = acpi_processor_hotadd_init(pr);
		if (ret)
			return ret;
	}

	/*
	 * On some boxes several processors use the same processor bus id.
	 * But they are located in different scope. For example:
	 * \_SB.SCK0.CPU0
	 * \_SB.SCK1.CPU0
	 * Rename the processor device bus id. And the new bus id will be
	 * generated as the following format:
	 * CPU+CPU ID.
	 */
	sprintf(acpi_device_bid(device), "CPU%X", pr->id);
	ACPI_DEBUG_PRINT((ACPI_DB_INFO, "Processor [%d:%d]\n", pr->id,
			  pr->acpi_id));

	if (!object.processor.pblk_address)
		ACPI_DEBUG_PRINT((ACPI_DB_INFO, "No PBLK (NULL address)\n"));
	else if (object.processor.pblk_length != 6)
		dev_err(&device->dev, "Invalid PBLK length [%d]\n",
			    object.processor.pblk_length);
	else {
		pr->throttling.address = object.processor.pblk_address;
		pr->throttling.duty_offset = acpi_gbl_FADT.duty_offset;
		pr->throttling.duty_width = acpi_gbl_FADT.duty_width;

		pr->pblk = object.processor.pblk_address;

		/*
		 * We don't care about error returns - we just try to mark
		 * these reserved so that nobody else is confused into thinking
		 * that this region might be unused..
		 *
		 * (In particular, allocating the IO range for Cardbus)
		 */
		request_region(pr->throttling.address, 6, "ACPI CPU throttle");
	}

	/*
	 * If ACPI describes a slot number for this CPU, we can use it to
	 * ensure we get the right value in the "physical id" field
	 * of /proc/cpuinfo
	 */
	status = acpi_evaluate_integer(pr->handle, "_SUN", NULL, &value);
	if (ACPI_SUCCESS(status))
		arch_fix_phys_package_id(pr->id, value);

	return 0;
}

/*
 * Do not put anything in here which needs the core to be online.
 * For example MSR access or setting up things which check for cpuinfo_x86
 * (cpu_data(cpu)) values, like CPU feature flags, family, model, etc.
 * Such things have to be put in and set up by the processor driver's .probe().
 */
static DEFINE_PER_CPU(void *, processor_device_array);

static int acpi_processor_add(struct acpi_device *device,
					const struct acpi_device_id *id)
{
	struct acpi_processor *pr;
	struct device *dev;
	int result = 0;

	pr = kzalloc(sizeof(struct acpi_processor), GFP_KERNEL);
	if (!pr)
		return -ENOMEM;

	if (!zalloc_cpumask_var(&pr->throttling.shared_cpu_map, GFP_KERNEL)) {
		result = -ENOMEM;
		goto err_free_pr;
	}

	pr->handle = device->handle;
	strcpy(acpi_device_name(device), ACPI_PROCESSOR_DEVICE_NAME);
	strcpy(acpi_device_class(device), ACPI_PROCESSOR_CLASS);
	device->driver_data = pr;

	result = acpi_processor_get_info(device);
	if (result) /* Processor is not physically present or unavailable */
		return 0;

#ifdef CONFIG_SMP
	if (pr->id >= setup_max_cpus && pr->id != 0)
		return 0;
#endif

	BUG_ON(pr->id >= nr_cpu_ids);

	/*
	 * Buggy BIOS check.
	 * ACPI id of processors can be reported wrongly by the BIOS.
	 * Don't trust it blindly
	 */
	if (per_cpu(processor_device_array, pr->id) != NULL &&
	    per_cpu(processor_device_array, pr->id) != device) {
		dev_warn(&device->dev,
			"BIOS reported wrong ACPI id %d for the processor\n",
			pr->id);
		/* Give up, but do not abort the namespace scan. */
		goto err;
	}
	/*
	 * processor_device_array is not cleared on errors to allow buggy BIOS
	 * checks.
	 */
	per_cpu(processor_device_array, pr->id) = device;
	per_cpu(processors, pr->id) = pr;

	dev = get_cpu_device(pr->id);
	if (!dev) {
		result = -ENODEV;
		goto err;
	}

	result = acpi_bind_one(dev, device);
	if (result)
		goto err;

	pr->dev = dev;

	/* Trigger the processor driver's .probe() if present. */
	if (device_attach(dev) >= 0)
		return 1;

	dev_err(dev, "Processor driver could not be attached\n");
	acpi_unbind_one(dev);

 err:
	free_cpumask_var(pr->throttling.shared_cpu_map);
	device->driver_data = NULL;
	per_cpu(processors, pr->id) = NULL;
 err_free_pr:
	kfree(pr);
	return result;
}

#ifdef CONFIG_ACPI_HOTPLUG_CPU
/* --------------------------------------------------------------------------
                                    Removal
   -------------------------------------------------------------------------- */

static void acpi_processor_remove(struct acpi_device *device)
{
	struct acpi_processor *pr;

	if (!device || !acpi_driver_data(device))
		return;

	pr = acpi_driver_data(device);
	if (pr->id >= nr_cpu_ids)
		goto out;

	/*
	 * The only reason why we ever get here is CPU hot-removal.  The CPU is
	 * already offline and the ACPI device removal locking prevents it from
	 * being put back online at this point.
	 *
	 * Unbind the driver from the processor device and detach it from the
	 * ACPI companion object.
	 */
	device_release_driver(pr->dev);
	acpi_unbind_one(pr->dev);

	/* Clean up. */
	per_cpu(processor_device_array, pr->id) = NULL;
	per_cpu(processors, pr->id) = NULL;

	cpu_maps_update_begin();
	cpu_hotplug_begin();

	/* Remove the CPU. */
	arch_unregister_cpu(pr->id);
	acpi_unmap_lsapic(pr->id);

	cpu_hotplug_done();
	cpu_maps_update_done();

	try_offline_node(cpu_to_node(pr->id));

 out:
	free_cpumask_var(pr->throttling.shared_cpu_map);
	kfree(pr);
}
#endif /* CONFIG_ACPI_HOTPLUG_CPU */

/*
 * The following ACPI IDs are known to be suitable for representing as
 * processor devices.
 */
static const struct acpi_device_id processor_device_ids[] = {

	{ ACPI_PROCESSOR_OBJECT_HID, },
	{ ACPI_PROCESSOR_DEVICE_HID, },

	{ }
};

static struct acpi_scan_handler __refdata processor_handler = {
	.ids = processor_device_ids,
	.attach = acpi_processor_add,
#ifdef CONFIG_ACPI_HOTPLUG_CPU
	.detach = acpi_processor_remove,
#endif
	.hotplug = {
		.enabled = true,
	},
};

void __init acpi_processor_init(void)
{
	acpi_scan_add_handler_with_hotplug(&processor_handler, "processor");
}
		if (ACPI_FAILURE(status)) {
			dev_err(&device->dev,
				"Failed to evaluate processor _UID (0x%x)\n",
				status);
			return -ENODEV;
		}
		device_declaration = 1;
		pr->acpi_id = value;
	}

	apic_id = acpi_get_apicid(pr->handle, device_declaration, pr->acpi_id);
	if (apic_id < 0)
		acpi_handle_debug(pr->handle, "failed to get CPU APIC ID.\n");
	pr->apic_id = apic_id;

	cpu_index = acpi_map_cpuid(pr->apic_id, pr->acpi_id);
	if (!cpu0_initialized && !acpi_has_cpu_in_madt()) {
		cpu0_initialized = 1;
		/* Handle UP system running SMP kernel, with no LAPIC in MADT */
		if ((cpu_index == -1) && (num_online_cpus() == 1))
			cpu_index = 0;
	}
	pr->id = cpu_index;

	/*
	 *  Extra Processor objects may be enumerated on MP systems with
	 *  less than the max # of CPUs. They should be ignored _iff
	 *  they are physically not present.
	 */
	if (pr->id == -1) {
		int ret = acpi_processor_hotadd_init(pr);
		if (ret)
			return ret;
	}

	/*
	 * On some boxes several processors use the same processor bus id.
	 * But they are located in different scope. For example:
	 * \_SB.SCK0.CPU0
	 * \_SB.SCK1.CPU0
	 * Rename the processor device bus id. And the new bus id will be
	 * generated as the following format:
	 * CPU+CPU ID.
	 */
	sprintf(acpi_device_bid(device), "CPU%X", pr->id);
	ACPI_DEBUG_PRINT((ACPI_DB_INFO, "Processor [%d:%d]\n", pr->id,
			  pr->acpi_id));

	if (!object.processor.pblk_address)
		ACPI_DEBUG_PRINT((ACPI_DB_INFO, "No PBLK (NULL address)\n"));
	else if (object.processor.pblk_length != 6)
		dev_err(&device->dev, "Invalid PBLK length [%d]\n",
			    object.processor.pblk_length);
	else {
		pr->throttling.address = object.processor.pblk_address;
		pr->throttling.duty_offset = acpi_gbl_FADT.duty_offset;
		pr->throttling.duty_width = acpi_gbl_FADT.duty_width;

		pr->pblk = object.processor.pblk_address;

		/*
		 * We don't care about error returns - we just try to mark
		 * these reserved so that nobody else is confused into thinking
		 * that this region might be unused..
		 *
		 * (In particular, allocating the IO range for Cardbus)
		 */
		request_region(pr->throttling.address, 6, "ACPI CPU throttle");
	}

	/*
	 * If ACPI describes a slot number for this CPU, we can use it to
	 * ensure we get the right value in the "physical id" field
	 * of /proc/cpuinfo
	 */
	status = acpi_evaluate_integer(pr->handle, "_SUN", NULL, &value);
	if (ACPI_SUCCESS(status))
		arch_fix_phys_package_id(pr->id, value);

	return 0;
}

/*
 * Do not put anything in here which needs the core to be online.
 * For example MSR access or setting up things which check for cpuinfo_x86
 * (cpu_data(cpu)) values, like CPU feature flags, family, model, etc.
 * Such things have to be put in and set up by the processor driver's .probe().
 */
static DEFINE_PER_CPU(void *, processor_device_array);

static int acpi_processor_add(struct acpi_device *device,
					const struct acpi_device_id *id)
{
	struct acpi_processor *pr;
	struct device *dev;
	int result = 0;

	pr = kzalloc(sizeof(struct acpi_processor), GFP_KERNEL);
	if (!pr)
		return -ENOMEM;

	if (!zalloc_cpumask_var(&pr->throttling.shared_cpu_map, GFP_KERNEL)) {
		result = -ENOMEM;
		goto err_free_pr;
	}

	pr->handle = device->handle;
	strcpy(acpi_device_name(device), ACPI_PROCESSOR_DEVICE_NAME);
	strcpy(acpi_device_class(device), ACPI_PROCESSOR_CLASS);
	device->driver_data = pr;

	result = acpi_processor_get_info(device);
	if (result) /* Processor is not physically present or unavailable */
		return 0;

#ifdef CONFIG_SMP
	if (pr->id >= setup_max_cpus && pr->id != 0)
		return 0;
#endif

	BUG_ON(pr->id >= nr_cpu_ids);

	/*
	 * Buggy BIOS check.
	 * ACPI id of processors can be reported wrongly by the BIOS.
	 * Don't trust it blindly
	 */
	if (per_cpu(processor_device_array, pr->id) != NULL &&
	    per_cpu(processor_device_array, pr->id) != device) {
		dev_warn(&device->dev,
			"BIOS reported wrong ACPI id %d for the processor\n",
			pr->id);
		/* Give up, but do not abort the namespace scan. */
		goto err;
	}
	/*
	 * processor_device_array is not cleared on errors to allow buggy BIOS
	 * checks.
	 */
	per_cpu(processor_device_array, pr->id) = device;
	per_cpu(processors, pr->id) = pr;

	dev = get_cpu_device(pr->id);
	if (!dev) {
		result = -ENODEV;
		goto err;
	}

	result = acpi_bind_one(dev, device);
	if (result)
		goto err;

	pr->dev = dev;

	/* Trigger the processor driver's .probe() if present. */
	if (device_attach(dev) >= 0)
		return 1;

	dev_err(dev, "Processor driver could not be attached\n");
	acpi_unbind_one(dev);

 err:
	free_cpumask_var(pr->throttling.shared_cpu_map);
	device->driver_data = NULL;
	per_cpu(processors, pr->id) = NULL;
 err_free_pr:
	kfree(pr);
	return result;
}

#ifdef CONFIG_ACPI_HOTPLUG_CPU
/* --------------------------------------------------------------------------
                                    Removal
   -------------------------------------------------------------------------- */

static void acpi_processor_remove(struct acpi_device *device)
{
	struct acpi_processor *pr;

	if (!device || !acpi_driver_data(device))
		return;

	pr = acpi_driver_data(device);
	if (pr->id >= nr_cpu_ids)
		goto out;

	/*
	 * The only reason why we ever get here is CPU hot-removal.  The CPU is
	 * already offline and the ACPI device removal locking prevents it from
	 * being put back online at this point.
	 *
	 * Unbind the driver from the processor device and detach it from the
	 * ACPI companion object.
	 */
	device_release_driver(pr->dev);
	acpi_unbind_one(pr->dev);

	/* Clean up. */
	per_cpu(processor_device_array, pr->id) = NULL;
	per_cpu(processors, pr->id) = NULL;

	cpu_maps_update_begin();
	cpu_hotplug_begin();

	/* Remove the CPU. */
	arch_unregister_cpu(pr->id);
	acpi_unmap_lsapic(pr->id);

	cpu_hotplug_done();
	cpu_maps_update_done();

	try_offline_node(cpu_to_node(pr->id));

 out:
	free_cpumask_var(pr->throttling.shared_cpu_map);
	kfree(pr);
}
#endif /* CONFIG_ACPI_HOTPLUG_CPU */

/*
 * The following ACPI IDs are known to be suitable for representing as
 * processor devices.
 */
static const struct acpi_device_id processor_device_ids[] = {

	{ ACPI_PROCESSOR_OBJECT_HID, },
	{ ACPI_PROCESSOR_DEVICE_HID, },

	{ }
};

static struct acpi_scan_handler __refdata processor_handler = {
	.ids = processor_device_ids,
	.attach = acpi_processor_add,
#ifdef CONFIG_ACPI_HOTPLUG_CPU
	.detach = acpi_processor_remove,
#endif
	.hotplug = {
		.enabled = true,
	},
};

void __init acpi_processor_init(void)
{
	acpi_scan_add_handler_with_hotplug(&processor_handler, "processor");
}
{
	struct acpi_processor *pr;

	if (!device || !acpi_driver_data(device))
		return;

	pr = acpi_driver_data(device);
	if (pr->id >= nr_cpu_ids)
		goto out;

	/*
	 * The only reason why we ever get here is CPU hot-removal.  The CPU is
	 * already offline and the ACPI device removal locking prevents it from
	 * being put back online at this point.
	 *
	 * Unbind the driver from the processor device and detach it from the
	 * ACPI companion object.
	 */
	device_release_driver(pr->dev);
	acpi_unbind_one(pr->dev);

	/* Clean up. */
	per_cpu(processor_device_array, pr->id) = NULL;
	per_cpu(processors, pr->id) = NULL;

	cpu_maps_update_begin();
	cpu_hotplug_begin();

	/* Remove the CPU. */
	arch_unregister_cpu(pr->id);
	acpi_unmap_lsapic(pr->id);

	cpu_hotplug_done();
	cpu_maps_update_done();

	try_offline_node(cpu_to_node(pr->id));

 out:
	free_cpumask_var(pr->throttling.shared_cpu_map);
	kfree(pr);
}
#endif /* CONFIG_ACPI_HOTPLUG_CPU */

/*
 * The following ACPI IDs are known to be suitable for representing as
 * processor devices.
 */
static const struct acpi_device_id processor_device_ids[] = {

	{ ACPI_PROCESSOR_OBJECT_HID, },
	{ ACPI_PROCESSOR_DEVICE_HID, },

	{ }
};

static struct acpi_scan_handler __refdata processor_handler = {
	.ids = processor_device_ids,
	.attach = acpi_processor_add,
#ifdef CONFIG_ACPI_HOTPLUG_CPU
	.detach = acpi_processor_remove,
#endif
	.hotplug = {
		.enabled = true,
	},
};

void __init acpi_processor_init(void)
{
	acpi_scan_add_handler_with_hotplug(&processor_handler, "processor");
}
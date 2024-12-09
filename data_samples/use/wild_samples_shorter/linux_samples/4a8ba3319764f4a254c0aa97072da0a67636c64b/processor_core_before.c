	unsigned long madt_end, entry;
	static struct acpi_table_madt *madt;
	static int read_madt;
	int apic_id = -1;

	if (!read_madt) {
		if (ACPI_FAILURE(acpi_get_table(ACPI_SIG_MADT, 0,
					(struct acpi_table_header **)&madt)))
	}

	if (!madt)
		return apic_id;

	entry = (unsigned long)madt;
	madt_end = entry + madt->header.length;

		struct acpi_subtable_header *header =
			(struct acpi_subtable_header *)entry;
		if (header->type == ACPI_MADT_TYPE_LOCAL_APIC) {
			if (!map_lapic_id(header, acpi_id, &apic_id))
				break;
		} else if (header->type == ACPI_MADT_TYPE_LOCAL_X2APIC) {
			if (!map_x2apic_id(header, type, acpi_id, &apic_id))
				break;
		} else if (header->type == ACPI_MADT_TYPE_LOCAL_SAPIC) {
			if (!map_lsapic_id(header, type, acpi_id, &apic_id))
				break;
		}
		entry += header->length;
	}
	return apic_id;
}

static int map_mat_entry(acpi_handle handle, int type, u32 acpi_id)
{
	struct acpi_buffer buffer = { ACPI_ALLOCATE_BUFFER, NULL };
	union acpi_object *obj;
	struct acpi_subtable_header *header;
	int apic_id = -1;

	if (ACPI_FAILURE(acpi_evaluate_object(handle, "_MAT", NULL, &buffer)))
		goto exit;


	header = (struct acpi_subtable_header *)obj->buffer.pointer;
	if (header->type == ACPI_MADT_TYPE_LOCAL_APIC)
		map_lapic_id(header, acpi_id, &apic_id);
	else if (header->type == ACPI_MADT_TYPE_LOCAL_SAPIC)
		map_lsapic_id(header, type, acpi_id, &apic_id);
	else if (header->type == ACPI_MADT_TYPE_LOCAL_X2APIC)
		map_x2apic_id(header, type, acpi_id, &apic_id);

exit:
	kfree(buffer.pointer);
	return apic_id;
}

int acpi_get_apicid(acpi_handle handle, int type, u32 acpi_id)
{
	int apic_id;

	apic_id = map_mat_entry(handle, type, acpi_id);
	if (apic_id == -1)
		apic_id = map_madt_entry(type, acpi_id);

	return apic_id;
}

int acpi_map_cpuid(int apic_id, u32 acpi_id)
{
#ifdef CONFIG_SMP
	int i;
#endif

	if (apic_id == -1) {
		/*
		 * On UP processor, there is no _MAT or MADT table.
		 * So above apic_id is always set to -1.
		 *
		 * BIOS may define multiple CPU handles even for UP processor.
		 * For example,
		 *
		 *     Processor (CPU3, 0x03, 0x00000410, 0x06) {}
		 * }
		 *
		 * Ignores apic_id and always returns 0 for the processor
		 * handle with acpi id 0 if nr_cpu_ids is 1.
		 * This should be the case if SMP tables are not found.
		 * Return -1 for other CPU's handle.
		 */
		if (nr_cpu_ids <= 1 && acpi_id == 0)
			return acpi_id;
		else
			return apic_id;
	}

#ifdef CONFIG_SMP
	for_each_possible_cpu(i) {
		if (cpu_physical_id(i) == apic_id)
			return i;
	}
#else
	/* In UP kernel, only processor 0 is valid */
	if (apic_id == 0)
		return apic_id;
#endif
	return -1;
}

int acpi_get_cpuid(acpi_handle handle, int type, u32 acpi_id)
{
	int apic_id;

	apic_id = acpi_get_apicid(handle, type, acpi_id);

	return acpi_map_cpuid(apic_id, acpi_id);
}
EXPORT_SYMBOL_GPL(acpi_get_cpuid);
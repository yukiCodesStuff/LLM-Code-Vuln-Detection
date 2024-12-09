	unsigned long madt_end, entry;
	static struct acpi_table_madt *madt;
	static int read_madt;
	int phys_id = -1;	/* CPU hardware ID */

	if (!read_madt) {
		if (ACPI_FAILURE(acpi_get_table(ACPI_SIG_MADT, 0,
					(struct acpi_table_header **)&madt)))
	}

	if (!madt)
		return phys_id;

	entry = (unsigned long)madt;
	madt_end = entry + madt->header.length;

		struct acpi_subtable_header *header =
			(struct acpi_subtable_header *)entry;
		if (header->type == ACPI_MADT_TYPE_LOCAL_APIC) {
			if (!map_lapic_id(header, acpi_id, &phys_id))
				break;
		} else if (header->type == ACPI_MADT_TYPE_LOCAL_X2APIC) {
			if (!map_x2apic_id(header, type, acpi_id, &phys_id))
				break;
		} else if (header->type == ACPI_MADT_TYPE_LOCAL_SAPIC) {
			if (!map_lsapic_id(header, type, acpi_id, &phys_id))
				break;
		}
		entry += header->length;
	}
	return phys_id;
}

static int map_mat_entry(acpi_handle handle, int type, u32 acpi_id)
{
	struct acpi_buffer buffer = { ACPI_ALLOCATE_BUFFER, NULL };
	union acpi_object *obj;
	struct acpi_subtable_header *header;
	int phys_id = -1;

	if (ACPI_FAILURE(acpi_evaluate_object(handle, "_MAT", NULL, &buffer)))
		goto exit;


	header = (struct acpi_subtable_header *)obj->buffer.pointer;
	if (header->type == ACPI_MADT_TYPE_LOCAL_APIC)
		map_lapic_id(header, acpi_id, &phys_id);
	else if (header->type == ACPI_MADT_TYPE_LOCAL_SAPIC)
		map_lsapic_id(header, type, acpi_id, &phys_id);
	else if (header->type == ACPI_MADT_TYPE_LOCAL_X2APIC)
		map_x2apic_id(header, type, acpi_id, &phys_id);

exit:
	kfree(buffer.pointer);
	return phys_id;
}

int acpi_get_phys_id(acpi_handle handle, int type, u32 acpi_id)
{
	int phys_id;

	phys_id = map_mat_entry(handle, type, acpi_id);
	if (phys_id == -1)
		phys_id = map_madt_entry(type, acpi_id);

	return phys_id;
}

int acpi_map_cpuid(int phys_id, u32 acpi_id)
{
#ifdef CONFIG_SMP
	int i;
#endif

	if (phys_id == -1) {
		/*
		 * On UP processor, there is no _MAT or MADT table.
		 * So above phys_id is always set to -1.
		 *
		 * BIOS may define multiple CPU handles even for UP processor.
		 * For example,
		 *
		 *     Processor (CPU3, 0x03, 0x00000410, 0x06) {}
		 * }
		 *
		 * Ignores phys_id and always returns 0 for the processor
		 * handle with acpi id 0 if nr_cpu_ids is 1.
		 * This should be the case if SMP tables are not found.
		 * Return -1 for other CPU's handle.
		 */
		if (nr_cpu_ids <= 1 && acpi_id == 0)
			return acpi_id;
		else
			return phys_id;
	}

#ifdef CONFIG_SMP
	for_each_possible_cpu(i) {
		if (cpu_physical_id(i) == phys_id)
			return i;
	}
#else
	/* In UP kernel, only processor 0 is valid */
	if (phys_id == 0)
		return phys_id;
#endif
	return -1;
}

int acpi_get_cpuid(acpi_handle handle, int type, u32 acpi_id)
{
	int phys_id;

	phys_id = acpi_get_phys_id(handle, type, acpi_id);

	return acpi_map_cpuid(phys_id, acpi_id);
}
EXPORT_SYMBOL_GPL(acpi_get_cpuid);
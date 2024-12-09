	} else if (header->type == ACPI_MADT_TYPE_LOCAL_SAPIC) {
		map_lsapic_id(header, type, acpi_id, &apic_id);
	}

exit:
	kfree(buffer.pointer);
	return apic_id;
}

int acpi_get_cpuid(acpi_handle handle, int type, u32 acpi_id)
{
#ifdef CONFIG_SMP
	int i;
#endif
	int apic_id = -1;

	apic_id = map_mat_entry(handle, type, acpi_id);
	if (apic_id == -1)
		apic_id = map_madt_entry(type, acpi_id);
	if (apic_id == -1) {
		/*
		 * On UP processor, there is no _MAT or MADT table.
		 * So above apic_id is always set to -1.
		 *
		 * BIOS may define multiple CPU handles even for UP processor.
		 * For example,
		 *
		 * Scope (_PR)
                 * {
		 *     Processor (CPU0, 0x00, 0x00000410, 0x06) {}
		 *     Processor (CPU1, 0x01, 0x00000410, 0x06) {}
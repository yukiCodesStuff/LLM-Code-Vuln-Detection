	}

exit:
	if (buffer.pointer)
		kfree(buffer.pointer);
	return apic_id;
}

int acpi_get_cpuid(acpi_handle handle, int type, u32 acpi_id)
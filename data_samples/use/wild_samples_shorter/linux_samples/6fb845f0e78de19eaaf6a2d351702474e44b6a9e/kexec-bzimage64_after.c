	struct efi_info *current_ei = &boot_params.efi_info;
	struct efi_info *ei = &params->efi_info;

	if (!efi_enabled(EFI_RUNTIME_SERVICES))
		return 0;

	if (!current_ei->efi_memmap_size)
		return 0;

	/*
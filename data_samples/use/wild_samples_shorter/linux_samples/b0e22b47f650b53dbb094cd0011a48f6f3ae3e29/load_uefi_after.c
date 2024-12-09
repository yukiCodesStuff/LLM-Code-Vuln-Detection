static int __init load_uefi_certs(void)
{
	efi_guid_t secure_var = EFI_IMAGE_SECURITY_DATABASE_GUID;
	efi_guid_t mok_var = EFI_SHIM_LOCK_GUID;
	void *db = NULL, *dbx = NULL, *mokx = NULL;
	unsigned long dbsize = 0, dbxsize = 0, mokxsize = 0;
	efi_status_t status;
	int rc = 0;

	if (!efi_rt_services_supported(EFI_RT_SUPPORTED_GET_VARIABLE))
		kfree(dbx);
	}

	mokx = get_cert_list(L"MokListXRT", &mok_var, &mokxsize, &status);
	if (!mokx) {
		if (status == EFI_NOT_FOUND)
			pr_debug("mokx variable wasn't found\n");
		else
			pr_info("Couldn't get mokx list\n");
	} else {
		rc = parse_efi_signature_list("UEFI:MokListXRT",
					      mokx, mokxsize,
					      get_handler_for_dbx);
		if (rc)
			pr_err("Couldn't parse mokx signatures %d\n", rc);
		kfree(mokx);
	}

	/* Load the MokListRT certs */
	rc = load_moklist_certs();

	return rc;
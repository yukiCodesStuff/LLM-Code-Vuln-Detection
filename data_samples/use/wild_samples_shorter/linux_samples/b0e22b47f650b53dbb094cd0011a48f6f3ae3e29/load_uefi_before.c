static int __init load_uefi_certs(void)
{
	efi_guid_t secure_var = EFI_IMAGE_SECURITY_DATABASE_GUID;
	void *db = NULL, *dbx = NULL;
	unsigned long dbsize = 0, dbxsize = 0;
	efi_status_t status;
	int rc = 0;

	if (!efi_rt_services_supported(EFI_RT_SUPPORTED_GET_VARIABLE))
		kfree(dbx);
	}

	/* Load the MokListRT certs */
	rc = load_moklist_certs();

	return rc;
	uefi_blacklist_hash(source, data, len, "bin:", 4);
}

/*
 * Add an X509 cert to the revocation list.
 */
static __init void uefi_revocation_list_x509(const char *source,
					     const void *data, size_t len)
{
	add_key_to_revocation_list(data, len);
}

/*
 * Return the appropriate handler for particular signature list types found in
 * the UEFI db and MokListRT tables.
 */
		return uefi_blacklist_x509_tbs;
	if (efi_guidcmp(*sig_type, efi_cert_sha256_guid) == 0)
		return uefi_blacklist_binary;
	if (efi_guidcmp(*sig_type, efi_cert_x509_guid) == 0)
		return uefi_revocation_list_x509;
	return 0;
}
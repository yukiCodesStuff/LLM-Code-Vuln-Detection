{
	u8 buf[32];
	int offset = 0;

	memcpy_fromio(buf, p, 32);
	if ((buf[5] < 32) && dmi_checksum(buf, buf[5])) {
		dmi_ver = (buf[6] << 8) + buf[7];

		/* Some BIOS report weird SMBIOS version, fix that up */
		switch (dmi_ver) {
		case 0x021F:
		case 0x0221:
			pr_debug("SMBIOS version fixup(2.%d->2.%d)\n",
			       dmi_ver & 0xFF, 3);
			dmi_ver = 0x0203;
			break;
		case 0x0233:
			pr_debug("SMBIOS version fixup(2.%d->2.%d)\n", 51, 6);
			dmi_ver = 0x0206;
			break;
		}
		offset = 16;
	}
	return dmi_present(buf + offset);
}
		switch (dmi_ver) {
		case 0x021F:
		case 0x0221:
			pr_debug("SMBIOS version fixup(2.%d->2.%d)\n",
			       dmi_ver & 0xFF, 3);
			dmi_ver = 0x0203;
			break;
		case 0x0233:
			pr_debug("SMBIOS version fixup(2.%d->2.%d)\n", 51, 6);
			dmi_ver = 0x0206;
			break;
		}
		offset = 16;
	}
	return dmi_present(buf + offset);
}

void __init dmi_scan_machine(void)
{
	char __iomem *p, *q;
	int rc;

	if (efi_enabled(EFI_CONFIG_TABLES)) {
		if (efi.smbios == EFI_INVALID_TABLE_ADDR)
			goto error;

		/* This is called as a core_initcall() because it isn't
		 * needed during early boot.  This also means we can
		 * iounmap the space when we're done with it.
		 */
		p = dmi_ioremap(efi.smbios, 32);
		if (p == NULL)
			goto error;

		rc = smbios_present(p);
		dmi_iounmap(p, 32);
		if (!rc) {
			dmi_available = 1;
			goto out;
		}
	}
	else {
		/*
		 * no iounmap() for that ioremap(); it would be a no-op, but
		 * it's so early in setup that sucker gets confused into doing
		 * what it shouldn't if we actually call it.
		 */
		p = dmi_ioremap(0xF0000, 0x10000);
		if (p == NULL)
			goto error;

		for (q = p; q < p + 0x10000; q += 16) {
			if (memcmp(q, "_SM_", 4) == 0 && q - p <= 0xFFE0)
				rc = smbios_present(q);
			else if (memcmp(q, "_DMI_", 5) == 0)
				rc = dmi_present(q);
			else
				continue;
			if (!rc) {
				dmi_available = 1;
				dmi_iounmap(p, 0x10000);
				goto out;
			}
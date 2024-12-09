	{ "PIONEER DVD-RW  DVR-216D",	NULL,	ATA_HORKAGE_NOSETXFER },

	/* devices that don't properly handle queued TRIM commands */
	{ "Micron_M[56]*",		NULL,	ATA_HORKAGE_NO_NCQ_TRIM |
						ATA_HORKAGE_ZERO_AFTER_TRIM, },
	{ "Crucial_CT*SSD*",		NULL,	ATA_HORKAGE_NO_NCQ_TRIM, },

	/*
	 * As defined, the DRAT (Deterministic Read After Trim) and RZAT
	 * (Return Zero After Trim) flags in the ATA Command Set are
	 * unreliable in the sense that they only define what happens if
	 * the device successfully executed the DSM TRIM command. TRIM
	 * is only advisory, however, and the device is free to silently
	 * ignore all or parts of the request.
	 *
	 * Whitelist drives that are known to reliably return zeroes
	 * after TRIM.
	 */

	/*
	 * The intel 510 drive has buggy DRAT/RZAT. Explicitly exclude
	 * that model before whitelisting all other intel SSDs.
	 */
	{ "INTEL*SSDSC2MH*",		NULL,	0, },

	{ "INTEL*SSD*", 		NULL,	ATA_HORKAGE_ZERO_AFTER_TRIM, },
	{ "SSD*INTEL*",			NULL,	ATA_HORKAGE_ZERO_AFTER_TRIM, },
	{ "Samsung*SSD*",		NULL,	ATA_HORKAGE_ZERO_AFTER_TRIM, },
	{ "SAMSUNG*SSD*",		NULL,	ATA_HORKAGE_ZERO_AFTER_TRIM, },
	{ "ST[1248][0248]0[FH]*",	NULL,	ATA_HORKAGE_ZERO_AFTER_TRIM, },

	/*
	 * Some WD SATA-I drives spin up and down erratically when the link
	 * is put into the slumber mode.  We don't have full list of the
		return NULL;

	for (i = 0, tag = ap->last_tag + 1; i < max_queue; i++, tag++) {
		if (ap->flags & ATA_FLAG_LOWTAG)
			tag = i;
		else
			tag = tag < max_queue ? tag : 0;

		/* the last tag is reserved for internal command. */
		if (tag == ATA_TAG_INTERNAL)
			continue;
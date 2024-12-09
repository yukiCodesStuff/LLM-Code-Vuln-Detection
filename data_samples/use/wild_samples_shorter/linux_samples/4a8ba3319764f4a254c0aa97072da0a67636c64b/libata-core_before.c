	{ "PIONEER DVD-RW  DVR-216D",	NULL,	ATA_HORKAGE_NOSETXFER },

	/* devices that don't properly handle queued TRIM commands */
	{ "Micron_M500*",		NULL,	ATA_HORKAGE_NO_NCQ_TRIM, },
	{ "Crucial_CT???M500SSD*",	NULL,	ATA_HORKAGE_NO_NCQ_TRIM, },
	{ "Micron_M550*",		NULL,	ATA_HORKAGE_NO_NCQ_TRIM, },
	{ "Crucial_CT*M550SSD*",	NULL,	ATA_HORKAGE_NO_NCQ_TRIM, },

	/*
	 * Some WD SATA-I drives spin up and down erratically when the link
	 * is put into the slumber mode.  We don't have full list of the
		return NULL;

	for (i = 0, tag = ap->last_tag + 1; i < max_queue; i++, tag++) {
		tag = tag < max_queue ? tag : 0;

		/* the last tag is reserved for internal command. */
		if (tag == ATA_TAG_INTERNAL)
			continue;
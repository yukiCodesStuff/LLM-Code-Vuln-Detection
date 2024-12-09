static const struct ata_blacklist_entry ata_device_blacklist [] = {
	/* Devices with DMA related problems under Linux */
	{ "WDC AC11000H",	NULL,		ATA_HORKAGE_NODMA },
	{ "WDC AC22100H",	NULL,		ATA_HORKAGE_NODMA },
	{ "WDC AC32500H",	NULL,		ATA_HORKAGE_NODMA },
	{ "WDC AC33100H",	NULL,		ATA_HORKAGE_NODMA },
	{ "WDC AC31600H",	NULL,		ATA_HORKAGE_NODMA },
	{ "WDC AC32100H",	"24.09P07",	ATA_HORKAGE_NODMA },
	{ "WDC AC23200L",	"21.10N21",	ATA_HORKAGE_NODMA },
	{ "Compaq CRD-8241B", 	NULL,		ATA_HORKAGE_NODMA },
	{ "CRD-8400B",		NULL, 		ATA_HORKAGE_NODMA },
	{ "CRD-848[02]B",	NULL,		ATA_HORKAGE_NODMA },
	{ "CRD-84",		NULL,		ATA_HORKAGE_NODMA },
	{ "SanDisk SDP3B",	NULL,		ATA_HORKAGE_NODMA },
	{ "SanDisk SDP3B-64",	NULL,		ATA_HORKAGE_NODMA },
	{ "SANYO CD-ROM CRD",	NULL,		ATA_HORKAGE_NODMA },
	{ "HITACHI CDR-8",	NULL,		ATA_HORKAGE_NODMA },
	{ "HITACHI CDR-8[34]35",NULL,		ATA_HORKAGE_NODMA },
	{ "Toshiba CD-ROM XM-6202B", NULL,	ATA_HORKAGE_NODMA },
	{ "TOSHIBA CD-ROM XM-1702BC", NULL,	ATA_HORKAGE_NODMA },
	{ "CD-532E-A", 		NULL,		ATA_HORKAGE_NODMA },
	{ "E-IDE CD-ROM CR-840",NULL,		ATA_HORKAGE_NODMA },
	{ "CD-ROM Drive/F5A",	NULL,		ATA_HORKAGE_NODMA },
	{ "WPI CDD-820", 	NULL,		ATA_HORKAGE_NODMA },
	{ "SAMSUNG CD-ROM SC-148C", NULL,	ATA_HORKAGE_NODMA },
	{ "SAMSUNG CD-ROM SC",	NULL,		ATA_HORKAGE_NODMA },
	{ "ATAPI CD-ROM DRIVE 40X MAXIMUM",NULL,ATA_HORKAGE_NODMA },
	{ "_NEC DV5800A", 	NULL,		ATA_HORKAGE_NODMA },
	{ "SAMSUNG CD-ROM SN-124", "N001",	ATA_HORKAGE_NODMA },
	{ "Seagate STT20000A", NULL,		ATA_HORKAGE_NODMA },
	{ " 2GB ATA Flash Disk", "ADMA428M",	ATA_HORKAGE_NODMA },
	/* Odd clown on sil3726/4726 PMPs */
	{ "Config  Disk",	NULL,		ATA_HORKAGE_DISABLE },

	/* Weird ATAPI devices */
	{ "TORiSAN DVD-ROM DRD-N216", NULL,	ATA_HORKAGE_MAX_SEC_128 },
	{ "QUANTUM DAT    DAT72-000", NULL,	ATA_HORKAGE_ATAPI_MOD16_DMA },
	{ "Slimtype DVD A  DS8A8SH", NULL,	ATA_HORKAGE_MAX_SEC_LBA48 },
	{ "Slimtype DVD A  DS8A9SH", NULL,	ATA_HORKAGE_MAX_SEC_LBA48 },

	/* Devices we expect to fail diagnostics */

	/* Devices where NCQ should be avoided */
	/* NCQ is slow */
	{ "WDC WD740ADFD-00",	NULL,		ATA_HORKAGE_NONCQ },
	{ "WDC WD740ADFD-00NLR1", NULL,		ATA_HORKAGE_NONCQ, },
	/* http://thread.gmane.org/gmane.linux.ide/14907 */
	{ "FUJITSU MHT2060BH",	NULL,		ATA_HORKAGE_NONCQ },
	/* NCQ is broken */
	{ "Maxtor *",		"BANC*",	ATA_HORKAGE_NONCQ },
	{ "Maxtor 7V300F0",	"VA111630",	ATA_HORKAGE_NONCQ },
	{ "ST380817AS",		"3.42",		ATA_HORKAGE_NONCQ },
	{ "ST3160023AS",	"3.42",		ATA_HORKAGE_NONCQ },
	{ "OCZ CORE_SSD",	"02.10104",	ATA_HORKAGE_NONCQ },

	/* Seagate NCQ + FLUSH CACHE firmware bug */
	{ "ST31500341AS",	"SD1[5-9]",	ATA_HORKAGE_NONCQ |
						ATA_HORKAGE_FIRMWARE_WARN },

	{ "ST31000333AS",	"SD1[5-9]",	ATA_HORKAGE_NONCQ |
						ATA_HORKAGE_FIRMWARE_WARN },

	{ "ST3640[36]23AS",	"SD1[5-9]",	ATA_HORKAGE_NONCQ |
						ATA_HORKAGE_FIRMWARE_WARN },

	{ "ST3320[68]13AS",	"SD1[5-9]",	ATA_HORKAGE_NONCQ |
						ATA_HORKAGE_FIRMWARE_WARN },

	/* Seagate Momentus SpinPoint M8 seem to have FPMDA_AA issues */
	{ "ST1000LM024 HN-M101MBB", "2AR10001",	ATA_HORKAGE_BROKEN_FPDMA_AA },
	{ "ST1000LM024 HN-M101MBB", "2BA30001",	ATA_HORKAGE_BROKEN_FPDMA_AA },

	/* Blacklist entries taken from Silicon Image 3124/3132
	   Windows driver .inf file - also several Linux problem reports */
	{ "HTS541060G9SA00",    "MB3OC60D",     ATA_HORKAGE_NONCQ, },
	{ "HTS541080G9SA00",    "MB4OC60D",     ATA_HORKAGE_NONCQ, },
	{ "HTS541010G9SA00",    "MBZOC60D",     ATA_HORKAGE_NONCQ, },

	/* https://bugzilla.kernel.org/show_bug.cgi?id=15573 */
	{ "C300-CTFDDAC128MAG",	"0001",		ATA_HORKAGE_NONCQ, },

	/* devices which puke on READ_NATIVE_MAX */
	{ "HDS724040KLSA80",	"KFAOA20N",	ATA_HORKAGE_BROKEN_HPA, },
	{ "WDC WD3200JD-00KLB0", "WD-WCAMR1130137", ATA_HORKAGE_BROKEN_HPA },
	{ "WDC WD2500JD-00HBB0", "WD-WMAL71490727", ATA_HORKAGE_BROKEN_HPA },
	{ "MAXTOR 6L080L4",	"A93.0500",	ATA_HORKAGE_BROKEN_HPA },

	/* this one allows HPA unlocking but fails IOs on the area */
	{ "OCZ-VERTEX",		    "1.30",	ATA_HORKAGE_BROKEN_HPA },

	/* Devices which report 1 sector over size HPA */
	{ "ST340823A",		NULL,		ATA_HORKAGE_HPA_SIZE, },
	{ "ST320413A",		NULL,		ATA_HORKAGE_HPA_SIZE, },
	{ "ST310211A",		NULL,		ATA_HORKAGE_HPA_SIZE, },

	/* Devices which get the IVB wrong */
	{ "QUANTUM FIREBALLlct10 05", "A03.0900", ATA_HORKAGE_IVB, },
	/* Maybe we should just blacklist TSSTcorp... */
	{ "TSSTcorp CDDVDW SH-S202[HJN]", "SB0[01]",  ATA_HORKAGE_IVB, },

	/* Devices that do not need bridging limits applied */
	{ "MTRON MSP-SATA*",		NULL,	ATA_HORKAGE_BRIDGE_OK, },
	{ "BUFFALO HD-QSU2/R5",		NULL,	ATA_HORKAGE_BRIDGE_OK, },

	/* Devices which aren't very happy with higher link speeds */
	{ "WD My Book",			NULL,	ATA_HORKAGE_1_5_GBPS, },
	{ "Seagate FreeAgent GoFlex",	NULL,	ATA_HORKAGE_1_5_GBPS, },

	/*
	 * Devices which choke on SETXFER.  Applies only if both the
	 * device and controller are SATA.
	 */
	{ "PIONEER DVD-RW  DVRTD08",	NULL,	ATA_HORKAGE_NOSETXFER },
	{ "PIONEER DVD-RW  DVRTD08A",	NULL,	ATA_HORKAGE_NOSETXFER },
	{ "PIONEER DVD-RW  DVR-215",	NULL,	ATA_HORKAGE_NOSETXFER },
	{ "PIONEER DVD-RW  DVR-212D",	NULL,	ATA_HORKAGE_NOSETXFER },
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
	 * affected devices.  Disable LPM if the device matches one of the
	 * known prefixes and is SATA-1.  As a side effect LPM partial is
	 * lost too.
	 *
	 * https://bugzilla.kernel.org/show_bug.cgi?id=57211
	 */
	{ "WDC WD800JD-*",		NULL,	ATA_HORKAGE_WD_BROKEN_LPM },
	{ "WDC WD1200JD-*",		NULL,	ATA_HORKAGE_WD_BROKEN_LPM },
	{ "WDC WD1600JD-*",		NULL,	ATA_HORKAGE_WD_BROKEN_LPM },
	{ "WDC WD2000JD-*",		NULL,	ATA_HORKAGE_WD_BROKEN_LPM },
	{ "WDC WD2500JD-*",		NULL,	ATA_HORKAGE_WD_BROKEN_LPM },
	{ "WDC WD3000JD-*",		NULL,	ATA_HORKAGE_WD_BROKEN_LPM },
	{ "WDC WD3200JD-*",		NULL,	ATA_HORKAGE_WD_BROKEN_LPM },

	/* End Marker */
	{ }
};

static unsigned long ata_dev_blacklisted(const struct ata_device *dev)
{
	unsigned char model_num[ATA_ID_PROD_LEN + 1];
	unsigned char model_rev[ATA_ID_FW_REV_LEN + 1];
	const struct ata_blacklist_entry *ad = ata_device_blacklist;

	ata_id_c_string(dev->id, model_num, ATA_ID_PROD, sizeof(model_num));
	ata_id_c_string(dev->id, model_rev, ATA_ID_FW_REV, sizeof(model_rev));

	while (ad->model_num) {
		if (glob_match(ad->model_num, model_num)) {
			if (ad->model_rev == NULL)
				return ad->horkage;
			if (glob_match(ad->model_rev, model_rev))
				return ad->horkage;
		}
		ad++;
	}
{
	struct ata_queued_cmd *qc = NULL;
	unsigned int max_queue = ap->host->n_tags;
	unsigned int i, tag;

	/* no command while frozen */
	if (unlikely(ap->pflags & ATA_PFLAG_FROZEN))
		return NULL;

	for (i = 0, tag = ap->last_tag + 1; i < max_queue; i++, tag++) {
		if (ap->flags & ATA_FLAG_LOWTAG)
			tag = i;
		else
			tag = tag < max_queue ? tag : 0;

		/* the last tag is reserved for internal command. */
		if (tag == ATA_TAG_INTERNAL)
			continue;

		if (!test_and_set_bit(tag, &ap->qc_allocated)) {
			qc = __ata_qc_from_tag(ap, tag);
			qc->tag = tag;
			ap->last_tag = tag;
			break;
		}
	}
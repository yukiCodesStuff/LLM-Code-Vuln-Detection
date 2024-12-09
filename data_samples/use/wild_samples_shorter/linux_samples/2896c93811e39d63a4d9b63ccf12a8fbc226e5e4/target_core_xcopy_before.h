	struct se_device *dst_dev;
	unsigned char dst_tid_wwn[XCOPY_NAA_IEEE_REGEX_LEN];
	unsigned char local_dev_wwn[XCOPY_NAA_IEEE_REGEX_LEN];

	sector_t src_lba;
	sector_t dst_lba;
	unsigned short stdi;
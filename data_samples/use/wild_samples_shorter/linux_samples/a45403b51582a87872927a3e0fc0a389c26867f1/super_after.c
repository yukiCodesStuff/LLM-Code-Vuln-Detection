	}

	/* Load the checksum driver */
	sbi->s_chksum_driver = crypto_alloc_shash("crc32c", 0, 0);
	if (IS_ERR(sbi->s_chksum_driver)) {
		ext4_msg(sb, KERN_ERR, "Cannot load crc32c driver.");
		ret = PTR_ERR(sbi->s_chksum_driver);
		sbi->s_chksum_driver = NULL;
		goto failed_mount;
	}

	/* Check superblock checksum */
	if (!ext4_superblock_csum_verify(sb, es)) {
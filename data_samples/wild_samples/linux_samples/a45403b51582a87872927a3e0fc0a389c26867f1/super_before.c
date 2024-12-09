	if (!ext4_verify_csum_type(sb, es)) {
		ext4_msg(sb, KERN_ERR, "VFS: Found ext4 filesystem with "
			 "unknown checksum algorithm.");
		silent = 1;
		goto cantfind_ext4;
	}

	/* Load the checksum driver */
	if (ext4_has_feature_metadata_csum(sb) ||
	    ext4_has_feature_ea_inode(sb)) {
		sbi->s_chksum_driver = crypto_alloc_shash("crc32c", 0, 0);
		if (IS_ERR(sbi->s_chksum_driver)) {
			ext4_msg(sb, KERN_ERR, "Cannot load crc32c driver.");
			ret = PTR_ERR(sbi->s_chksum_driver);
			sbi->s_chksum_driver = NULL;
			goto failed_mount;
		}
	}
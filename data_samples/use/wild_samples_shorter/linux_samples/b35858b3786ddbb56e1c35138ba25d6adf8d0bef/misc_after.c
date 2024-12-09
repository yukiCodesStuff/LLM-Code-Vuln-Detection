			cifs_dbg(VFS, "Length less than smb header size\n");
		}
		return -EIO;
	} else if (total_read < sizeof(*smb) + 2 * smb->WordCount) {
		cifs_dbg(VFS, "%s: can't read BCC due to invalid WordCount(%u)\n",
			 __func__, smb->WordCount);
		return -EIO;
	}

	/* otherwise, there is enough to get to the BCC */
	if (check_smb_hdr(smb))
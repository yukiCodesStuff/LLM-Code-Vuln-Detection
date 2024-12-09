	}

	mid = le64_to_cpu(shdr->MessageId);
	if (len < pdu_size) {
		if ((len >= hdr_size)
		    && (shdr->Status != 0)) {
			pdu->StructureSize2 = 0;
		return 1;
	}

	if (check_smb2_hdr(shdr, mid))
		return 1;

	if (shdr->StructureSize != SMB2_HEADER_STRUCTURE_SIZE) {
		cifs_dbg(VFS, "Invalid structure size %u\n",
			 le16_to_cpu(shdr->StructureSize));
		return 1;
	}

	command = le16_to_cpu(shdr->Command);
	if (command >= NUMBER_OF_SMB2_COMMANDS) {
		cifs_dbg(VFS, "Invalid SMB2 command %d\n", command);
		return 1;
	}

	if (smb2_rsp_struct_sizes[command] != pdu->StructureSize2) {
		if (command != SMB2_OPLOCK_BREAK_HE && (shdr->Status == 0 ||
		    pdu->StructureSize2 != SMB2_ERROR_STRUCTURE_SIZE2_LE)) {
			/* error packets have 9 byte structure size */
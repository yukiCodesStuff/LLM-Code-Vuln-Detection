		if (!ses) {
			cifs_dbg(VFS, "no decryption - session id not found\n");
			return 1;
		}
	}

	mid = le64_to_cpu(shdr->MessageId);
	if (check_smb2_hdr(shdr, mid))
		return 1;

	if (shdr->StructureSize != SMB2_HEADER_STRUCTURE_SIZE) {
		cifs_dbg(VFS, "Invalid structure size %u\n",
			 le16_to_cpu(shdr->StructureSize));
		return 1;
	}
	if (len > CIFSMaxBufSize + MAX_SMB2_HDR_SIZE) {
		cifs_dbg(VFS, "SMB length greater than maximum, mid=%llu\n",
			 mid);
		return 1;
	}

	if (smb2_rsp_struct_sizes[command] != pdu->StructureSize2) {
		if (command != SMB2_OPLOCK_BREAK_HE && (shdr->Status == 0 ||
		    pdu->StructureSize2 != SMB2_ERROR_STRUCTURE_SIZE2_LE)) {
			/* error packets have 9 byte structure size */
			cifs_dbg(VFS, "Invalid response size %u for command %d\n",
				 le16_to_cpu(pdu->StructureSize2), command);
			return 1;
		} else if (command == SMB2_OPLOCK_BREAK_HE
			   && (shdr->Status == 0)
			   && (le16_to_cpu(pdu->StructureSize2) != 44)
			   && (le16_to_cpu(pdu->StructureSize2) != 36)) {
			/* special case for SMB2.1 lease break message */
			cifs_dbg(VFS, "Invalid response size %d for oplock break\n",
				 le16_to_cpu(pdu->StructureSize2));
			return 1;
		}
	}
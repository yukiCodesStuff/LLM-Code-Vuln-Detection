{
#ifdef CONFIG_CIFS_DEBUG2
	struct smb2_hdr *shdr = (struct smb2_hdr *)buf;

	cifs_server_dbg(VFS, "Cmd: %d Err: 0x%x Flags: 0x%x Mid: %llu Pid: %d\n",
		 shdr->Command, shdr->Status, shdr->Flags, shdr->MessageId,
		 shdr->Id.SyncId.ProcessId);
	if (!server->ops->check_message(buf, server->total_read, server)) {
		cifs_server_dbg(VFS, "smb buf %p len %u\n", buf,
				server->ops->calc_smb_size(buf));
	}
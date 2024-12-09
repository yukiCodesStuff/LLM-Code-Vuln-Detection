{
#ifdef CONFIG_CIFS_DEBUG2
	struct smb2_hdr *shdr = (struct smb2_hdr *)buf;

	cifs_server_dbg(VFS, "Cmd: %d Err: 0x%x Flags: 0x%x Mid: %llu Pid: %d\n",
		 shdr->Command, shdr->Status, shdr->Flags, shdr->MessageId,
		 shdr->Id.SyncId.ProcessId);
	cifs_server_dbg(VFS, "smb buf %p len %u\n", buf,
		 server->ops->calc_smb_size(buf));
#endif
}

static bool
smb2_need_neg(struct TCP_Server_Info *server)
{
	return server->max_read == 0;
}

static int
smb2_negotiate(const unsigned int xid,
	       struct cifs_ses *ses,
	       struct TCP_Server_Info *server)
{
	int rc;

	spin_lock(&server->mid_lock);
	server->CurrentMid = 0;
	spin_unlock(&server->mid_lock);
	rc = SMB2_negotiate(xid, ses, server);
	/* BB we probably don't need to retry with modern servers */
	if (rc == -EAGAIN)
		rc = -EHOSTDOWN;
	return rc;
}

static unsigned int
smb2_negotiate_wsize(struct cifs_tcon *tcon, struct smb3_fs_context *ctx)
{
	struct TCP_Server_Info *server = tcon->ses->server;
	unsigned int wsize;

	/* start with specified wsize, or default */
	wsize = ctx->wsize ? ctx->wsize : CIFS_DEFAULT_IOSIZE;
	wsize = min_t(unsigned int, wsize, server->max_write);
	if (!(server->capabilities & SMB2_GLOBAL_CAP_LARGE_MTU))
		wsize = min_t(unsigned int, wsize, SMB2_MAX_BUFFER_SIZE);

	return wsize;
}

static unsigned int
smb3_negotiate_wsize(struct cifs_tcon *tcon, struct smb3_fs_context *ctx)
{
	struct TCP_Server_Info *server = tcon->ses->server;
	unsigned int wsize;

	/* start with specified wsize, or default */
	wsize = ctx->wsize ? ctx->wsize : SMB3_DEFAULT_IOSIZE;
	wsize = min_t(unsigned int, wsize, server->max_write);
#ifdef CONFIG_CIFS_SMB_DIRECT
	if (server->rdma) {
		if (server->sign)
			/*
			 * Account for SMB2 data transfer packet header and
			 * possible encryption header
			 */
			wsize = min_t(unsigned int,
				wsize,
				server->smbd_conn->max_fragmented_send_size -
					SMB2_READWRITE_PDU_HEADER_SIZE -
					sizeof(struct smb2_transform_hdr));
		else
			wsize = min_t(unsigned int,
				wsize, server->smbd_conn->max_readwrite_size);
	}
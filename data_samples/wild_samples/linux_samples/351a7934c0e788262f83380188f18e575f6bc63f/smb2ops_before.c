{
	struct TCP_Server_Info *server = tcon->ses->server;
	unsigned int wsize;

	/* start with specified wsize, or default */
	wsize = volume_info->wsize ? volume_info->wsize : CIFS_DEFAULT_IOSIZE;
	wsize = min_t(unsigned int, wsize, server->max_write);
	/*
	 * limit write size to 2 ** 16, because we don't support multicredit
	 * requests now.
	 */
	wsize = min_t(unsigned int, wsize, 2 << 15);

	return wsize;
}

static unsigned int
smb2_negotiate_rsize(struct cifs_tcon *tcon, struct smb_vol *volume_info)
{
	struct TCP_Server_Info *server = tcon->ses->server;
	unsigned int rsize;

	/* start with specified rsize, or default */
	rsize = volume_info->rsize ? volume_info->rsize : CIFS_DEFAULT_IOSIZE;
	rsize = min_t(unsigned int, rsize, server->max_read);
	/*
	 * limit write size to 2 ** 16, because we don't support multicredit
	 * requests now.
	 */
	rsize = min_t(unsigned int, rsize, 2 << 15);

	return rsize;
}

#ifdef CONFIG_CIFS_STATS2
static int
SMB3_request_interfaces(const unsigned int xid, struct cifs_tcon *tcon)
{
	int rc;
	unsigned int ret_data_len = 0;
	struct network_interface_info_ioctl_rsp *out_buf;

	rc = SMB2_ioctl(xid, tcon, NO_FILE_ID, NO_FILE_ID,
			FSCTL_QUERY_NETWORK_INTERFACE_INFO, true /* is_fsctl */,
			NULL /* no data input */, 0 /* no data input */,
			(char **)&out_buf, &ret_data_len);

	if ((rc == 0)  && (ret_data_len > 0)) {
		/* Dump info on first interface */
		cifs_dbg(FYI, "Adapter Capability 0x%x\t",
			le32_to_cpu(out_buf->Capability));
		cifs_dbg(FYI, "Link Speed %lld\n",
			le64_to_cpu(out_buf->LinkSpeed));
	} else
		cifs_dbg(VFS, "error %d on ioctl to get interface list\n", rc);

	return rc;
}
{
	struct TCP_Server_Info *server = tcon->ses->server;
	unsigned int rsize;

	/* start with specified rsize, or default */
	rsize = volume_info->rsize ? volume_info->rsize : CIFS_DEFAULT_IOSIZE;
	rsize = min_t(unsigned int, rsize, server->max_read);
	/*
	 * limit write size to 2 ** 16, because we don't support multicredit
	 * requests now.
	 */
	rsize = min_t(unsigned int, rsize, 2 << 15);

	return rsize;
}

#ifdef CONFIG_CIFS_STATS2
static int
SMB3_request_interfaces(const unsigned int xid, struct cifs_tcon *tcon)
{
	int rc;
	unsigned int ret_data_len = 0;
	struct network_interface_info_ioctl_rsp *out_buf;

	rc = SMB2_ioctl(xid, tcon, NO_FILE_ID, NO_FILE_ID,
			FSCTL_QUERY_NETWORK_INTERFACE_INFO, true /* is_fsctl */,
			NULL /* no data input */, 0 /* no data input */,
			(char **)&out_buf, &ret_data_len);

	if ((rc == 0)  && (ret_data_len > 0)) {
		/* Dump info on first interface */
		cifs_dbg(FYI, "Adapter Capability 0x%x\t",
			le32_to_cpu(out_buf->Capability));
		cifs_dbg(FYI, "Link Speed %lld\n",
			le64_to_cpu(out_buf->LinkSpeed));
	} else
		cifs_dbg(VFS, "error %d on ioctl to get interface list\n", rc);

	return rc;
}
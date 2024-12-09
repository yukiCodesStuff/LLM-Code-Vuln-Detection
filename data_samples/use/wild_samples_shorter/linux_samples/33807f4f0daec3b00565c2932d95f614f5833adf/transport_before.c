		iov->iov_len = rqst->rq_pagesz;
}

static int
smb_send_rqst(struct TCP_Server_Info *server, struct smb_rqst *rqst)
{
	int rc;
	struct kvec *iov = rqst->rq_iov;
	int n_vec = rqst->rq_nvec;
	unsigned int smb_buf_length = get_rfc1002_length(iov[0].iov_base);
	unsigned int i;
	size_t total_len = 0, sent;
	struct socket *ssocket = server->ssocket;
	int val = 1;
	if (ssocket == NULL)
		return -ENOTSOCK;

	cifs_dbg(FYI, "Sending smb: smb_len=%u\n", smb_buf_length);
	dump_smb(iov[0].iov_base, iov[0].iov_len);

	/* cork the socket */
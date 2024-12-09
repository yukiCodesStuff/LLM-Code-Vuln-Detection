		iov->iov_len = rqst->rq_pagesz;
}

static unsigned long
rqst_len(struct smb_rqst *rqst)
{
	unsigned int i;
	struct kvec *iov = rqst->rq_iov;
	unsigned long buflen = 0;

	/* total up iov array first */
	for (i = 0; i < rqst->rq_nvec; i++)
		buflen += iov[i].iov_len;

	/* add in the page array if there is one */
	if (rqst->rq_npages) {
		buflen += rqst->rq_pagesz * (rqst->rq_npages - 1);
		buflen += rqst->rq_tailsz;
	}

	return buflen;
}

static int
smb_send_rqst(struct TCP_Server_Info *server, struct smb_rqst *rqst)
{
	int rc;
	struct kvec *iov = rqst->rq_iov;
	int n_vec = rqst->rq_nvec;
	unsigned int smb_buf_length = get_rfc1002_length(iov[0].iov_base);
	unsigned long send_length;
	unsigned int i;
	size_t total_len = 0, sent;
	struct socket *ssocket = server->ssocket;
	int val = 1;
	if (ssocket == NULL)
		return -ENOTSOCK;

	/* sanity check send length */
	send_length = rqst_len(rqst);
	if (send_length != smb_buf_length + 4) {
		WARN(1, "Send length mismatch(send_length=%lu smb_buf_length=%u)\n",
			send_length, smb_buf_length);
		return -EIO;
	}

	cifs_dbg(FYI, "Sending smb: smb_len=%u\n", smb_buf_length);
	dump_smb(iov[0].iov_base, iov[0].iov_len);

	/* cork the socket */
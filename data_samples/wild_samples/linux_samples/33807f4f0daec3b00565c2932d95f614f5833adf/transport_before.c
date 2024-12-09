{
	/*
	 * FIXME: We could avoid this kmap altogether if we used
	 * kernel_sendpage instead of kernel_sendmsg. That will only
	 * work if signing is disabled though as sendpage inlines the
	 * page directly into the fraglist. If userspace modifies the
	 * page after we calculate the signature, then the server will
	 * reject it and may break the connection. kernel_sendmsg does
	 * an extra copy of the data and avoids that issue.
	 */
	iov->iov_base = kmap(rqst->rq_pages[idx]);

	/* if last page, don't send beyond this offset into page */
	if (idx == (rqst->rq_npages - 1))
		iov->iov_len = rqst->rq_tailsz;
	else
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
	kernel_setsockopt(ssocket, SOL_TCP, TCP_CORK,
				(char *)&val, sizeof(val));

	rc = smb_send_kvec(server, iov, n_vec, &sent);
	if (rc < 0)
		goto uncork;

	total_len += sent;

	/* now walk the page array and send each page in it */
	for (i = 0; i < rqst->rq_npages; i++) {
		struct kvec p_iov;

		cifs_rqst_page_to_kvec(rqst, i, &p_iov);
		rc = smb_send_kvec(server, &p_iov, 1, &sent);
		kunmap(rqst->rq_pages[i]);
		if (rc < 0)
			break;

		total_len += sent;
	}
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
	kernel_setsockopt(ssocket, SOL_TCP, TCP_CORK,
				(char *)&val, sizeof(val));

	rc = smb_send_kvec(server, iov, n_vec, &sent);
	if (rc < 0)
		goto uncork;

	total_len += sent;

	/* now walk the page array and send each page in it */
	for (i = 0; i < rqst->rq_npages; i++) {
		struct kvec p_iov;

		cifs_rqst_page_to_kvec(rqst, i, &p_iov);
		rc = smb_send_kvec(server, &p_iov, 1, &sent);
		kunmap(rqst->rq_pages[i]);
		if (rc < 0)
			break;

		total_len += sent;
	}
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
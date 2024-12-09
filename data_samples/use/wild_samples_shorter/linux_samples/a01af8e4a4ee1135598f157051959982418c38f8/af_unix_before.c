	sock_wfree(skb);
}

static int unix_attach_fds(struct scm_cookie *scm, struct sk_buff *skb)
{
	int i;

	/*
	 * Need to duplicate file references for the sake of garbage
	 * collection.  Otherwise a socket in the fps might become a
	if (!UNIXCB(skb).fp)
		return -ENOMEM;

	for (i = scm->fp->count-1; i >= 0; i--)
		unix_inflight(scm->fp->fp[i]);
	return 0;
}

static int unix_scm_to_skb(struct scm_cookie *scm, struct sk_buff *skb, bool send_fds)
{
	struct sk_buff *skb;
	long timeo;
	struct scm_cookie tmp_scm;

	if (NULL == siocb->scm)
		siocb->scm = &tmp_scm;
	wait_for_unix_gc();
		goto out;

	err = unix_scm_to_skb(siocb->scm, skb, true);
	if (err)
		goto out_free;
	unix_get_secdata(siocb->scm, skb);

	skb_reset_transport_header(skb);
	err = memcpy_fromiovec(skb_put(skb, len), msg->msg_iov, len);
	if (sock_flag(other, SOCK_RCVTSTAMP))
		__net_timestamp(skb);
	skb_queue_tail(&other->sk_receive_queue, skb);
	unix_state_unlock(other);
	other->sk_data_ready(other, len);
	sock_put(other);
	scm_destroy(siocb->scm);
	int sent = 0;
	struct scm_cookie tmp_scm;
	bool fds_sent = false;

	if (NULL == siocb->scm)
		siocb->scm = &tmp_scm;
	wait_for_unix_gc();

		/* Only send the fds in the first buffer */
		err = unix_scm_to_skb(siocb->scm, skb, !fds_sent);
		if (err) {
			kfree_skb(skb);
			goto out_err;
		}
		fds_sent = true;

		err = memcpy_fromiovec(skb_put(skb, size), msg->msg_iov, size);
		if (err) {
			goto pipe_err_free;

		skb_queue_tail(&other->sk_receive_queue, skb);
		unix_state_unlock(other);
		other->sk_data_ready(other, size);
		sent += size;
	}
		unix_state_lock(sk);
		skb = skb_dequeue(&sk->sk_receive_queue);
		if (skb == NULL) {
			if (copied >= target)
				goto unlock;

			/*
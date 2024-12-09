	sock_wfree(skb);
}

#define MAX_RECURSION_LEVEL 4

static int unix_attach_fds(struct scm_cookie *scm, struct sk_buff *skb)
{
	int i;
	unsigned char max_level = 0;
	int unix_sock_count = 0;

	for (i = scm->fp->count - 1; i >= 0; i--) {
		struct sock *sk = unix_get_socket(scm->fp->fp[i]);

		if (sk) {
			unix_sock_count++;
			max_level = max(max_level,
					unix_sk(sk)->recursion_level);
		}
	}
	if (unlikely(max_level > MAX_RECURSION_LEVEL))
		return -ETOOMANYREFS;

	/*
	 * Need to duplicate file references for the sake of garbage
	 * collection.  Otherwise a socket in the fps might become a
	if (!UNIXCB(skb).fp)
		return -ENOMEM;

	if (unix_sock_count) {
		for (i = scm->fp->count - 1; i >= 0; i--)
			unix_inflight(scm->fp->fp[i]);
	}
	return max_level;
}

static int unix_scm_to_skb(struct scm_cookie *scm, struct sk_buff *skb, bool send_fds)
{
	struct sk_buff *skb;
	long timeo;
	struct scm_cookie tmp_scm;
	int max_level;

	if (NULL == siocb->scm)
		siocb->scm = &tmp_scm;
	wait_for_unix_gc();
		goto out;

	err = unix_scm_to_skb(siocb->scm, skb, true);
	if (err < 0)
		goto out_free;
	max_level = err + 1;
	unix_get_secdata(siocb->scm, skb);

	skb_reset_transport_header(skb);
	err = memcpy_fromiovec(skb_put(skb, len), msg->msg_iov, len);
	if (sock_flag(other, SOCK_RCVTSTAMP))
		__net_timestamp(skb);
	skb_queue_tail(&other->sk_receive_queue, skb);
	if (max_level > unix_sk(other)->recursion_level)
		unix_sk(other)->recursion_level = max_level;
	unix_state_unlock(other);
	other->sk_data_ready(other, len);
	sock_put(other);
	scm_destroy(siocb->scm);
	int sent = 0;
	struct scm_cookie tmp_scm;
	bool fds_sent = false;
	int max_level;

	if (NULL == siocb->scm)
		siocb->scm = &tmp_scm;
	wait_for_unix_gc();

		/* Only send the fds in the first buffer */
		err = unix_scm_to_skb(siocb->scm, skb, !fds_sent);
		if (err < 0) {
			kfree_skb(skb);
			goto out_err;
		}
		max_level = err + 1;
		fds_sent = true;

		err = memcpy_fromiovec(skb_put(skb, size), msg->msg_iov, size);
		if (err) {
			goto pipe_err_free;

		skb_queue_tail(&other->sk_receive_queue, skb);
		if (max_level > unix_sk(other)->recursion_level)
			unix_sk(other)->recursion_level = max_level;
		unix_state_unlock(other);
		other->sk_data_ready(other, size);
		sent += size;
	}
		unix_state_lock(sk);
		skb = skb_dequeue(&sk->sk_receive_queue);
		if (skb == NULL) {
			unix_sk(sk)->recursion_level = 0;
			if (copied >= target)
				goto unlock;

			/*
	sock_wfree(skb);
}

#define MAX_RECURSION_LEVEL 4

static int unix_attach_fds(struct scm_cookie *scm, struct sk_buff *skb)
{
	unsigned char max_level = 0;
	int unix_sock_count = 0;

	for (i = scm->fp->count - 1; i >= 0; i--) {
		struct sock *sk = unix_get_socket(scm->fp->fp[i]);

		if (sk) {
	if (!UNIXCB(skb).fp)
		return -ENOMEM;

	if (unix_sock_count) {
		for (i = scm->fp->count - 1; i >= 0; i--)
			unix_inflight(scm->fp->fp[i]);
	}
	return max_level;
}

static int unix_scm_to_skb(struct scm_cookie *scm, struct sk_buff *skb, bool send_fds)
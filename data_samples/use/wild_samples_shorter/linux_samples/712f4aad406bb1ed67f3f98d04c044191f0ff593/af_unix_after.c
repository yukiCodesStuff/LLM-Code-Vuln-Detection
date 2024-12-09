	sock_wfree(skb);
}

/*
 * The "user->unix_inflight" variable is protected by the garbage
 * collection lock, and we just read it locklessly here. If you go
 * over the limit, there might be a tiny race in actually noticing
 * it across threads. Tough.
 */
static inline bool too_many_unix_fds(struct task_struct *p)
{
	struct user_struct *user = current_user();

	if (unlikely(user->unix_inflight > task_rlimit(p, RLIMIT_NOFILE)))
		return !capable(CAP_SYS_RESOURCE) && !capable(CAP_SYS_ADMIN);
	return false;
}

#define MAX_RECURSION_LEVEL 4

static int unix_attach_fds(struct scm_cookie *scm, struct sk_buff *skb)
{
	unsigned char max_level = 0;
	int unix_sock_count = 0;

	if (too_many_unix_fds(current))
		return -ETOOMANYREFS;

	for (i = scm->fp->count - 1; i >= 0; i--) {
		struct sock *sk = unix_get_socket(scm->fp->fp[i]);

		if (sk) {
	if (!UNIXCB(skb).fp)
		return -ENOMEM;

	for (i = scm->fp->count - 1; i >= 0; i--)
		unix_inflight(scm->fp->fp[i]);
	return max_level;
}

static int unix_scm_to_skb(struct scm_cookie *scm, struct sk_buff *skb, bool send_fds)
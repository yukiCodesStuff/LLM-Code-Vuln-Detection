{
	struct scm_cookie scm;
	memset(&scm, 0, sizeof(scm));
	scm.pid  = UNIXCB(skb).pid;
	if (UNIXCB(skb).fp)
		unix_detach_fds(&scm, skb);

	/* Alas, it calls VFS */
	/* So fscking what? fput() had been SMP-safe since the last Summer */
	scm_destroy(&scm);
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
	int i;
	unsigned char max_level = 0;
	int unix_sock_count = 0;

	if (too_many_unix_fds(current))
		return -ETOOMANYREFS;

	for (i = scm->fp->count - 1; i >= 0; i--) {
		struct sock *sk = unix_get_socket(scm->fp->fp[i]);

		if (sk) {
			unix_sock_count++;
			max_level = max(max_level,
					unix_sk(sk)->recursion_level);
		}
	}
{
	int i;
	unsigned char max_level = 0;
	int unix_sock_count = 0;

	if (too_many_unix_fds(current))
		return -ETOOMANYREFS;

	for (i = scm->fp->count - 1; i >= 0; i--) {
		struct sock *sk = unix_get_socket(scm->fp->fp[i]);

		if (sk) {
			unix_sock_count++;
			max_level = max(max_level,
					unix_sk(sk)->recursion_level);
		}
	}
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
	 * candidate for GC while the skb is not yet queued.
	 */
	UNIXCB(skb).fp = scm_fp_dup(scm->fp);
	if (!UNIXCB(skb).fp)
		return -ENOMEM;

	for (i = scm->fp->count - 1; i >= 0; i--)
		unix_inflight(scm->fp->fp[i]);
	return max_level;
}

static int unix_scm_to_skb(struct scm_cookie *scm, struct sk_buff *skb, bool send_fds)
{
	int err = 0;

	UNIXCB(skb).pid  = get_pid(scm->pid);
	UNIXCB(skb).uid = scm->creds.uid;
	UNIXCB(skb).gid = scm->creds.gid;
	UNIXCB(skb).fp = NULL;
	unix_get_secdata(scm, skb);
	if (scm->fp && send_fds)
		err = unix_attach_fds(scm, skb);

	skb->destructor = unix_destruct_scm;
	return err;
}

static bool unix_passcred_enabled(const struct socket *sock,
				  const struct sock *other)
{
	return test_bit(SOCK_PASSCRED, &sock->flags) ||
	       !other->sk_socket ||
	       test_bit(SOCK_PASSCRED, &other->sk_socket->flags);
}

/*
 * Some apps rely on write() giving SCM_CREDENTIALS
 * We include credentials if source or destination socket
 * asserted SOCK_PASSCRED.
 */
static void maybe_add_creds(struct sk_buff *skb, const struct socket *sock,
			    const struct sock *other)
{
	if (UNIXCB(skb).pid)
		return;
	if (unix_passcred_enabled(sock, other)) {
		UNIXCB(skb).pid  = get_pid(task_tgid(current));
		current_uid_gid(&UNIXCB(skb).uid, &UNIXCB(skb).gid);
	}
{
	struct scm_cookie scm;
	memset(&scm, 0, sizeof(scm));
	scm.pid  = UNIXCB(skb).pid;
	scm.cred = UNIXCB(skb).cred;
	if (UNIXCB(skb).fp)
		unix_detach_fds(&scm, skb);

	/* Alas, it calls VFS */
	/* So fscking what? fput() had been SMP-safe since the last Summer */
	scm_destroy(&scm);
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

	if (unix_sock_count) {
		for (i = scm->fp->count - 1; i >= 0; i--)
			unix_inflight(scm->fp->fp[i]);
	}
{
	struct sock_iocb *siocb = kiocb_to_siocb(kiocb);
	struct sock *sk = sock->sk;
	struct net *net = sock_net(sk);
	struct unix_sock *u = unix_sk(sk);
	struct sockaddr_un *sunaddr = msg->msg_name;
	struct sock *other = NULL;
	int namelen = 0; /* fake GCC */
	int err;
	unsigned hash;
	struct sk_buff *skb;
	long timeo;
	struct scm_cookie tmp_scm;
	int max_level;

	if (NULL == siocb->scm)
		siocb->scm = &tmp_scm;
	wait_for_unix_gc();
	err = scm_send(sock, msg, siocb->scm);
	if (err < 0)
		return err;

	err = -EOPNOTSUPP;
	if (msg->msg_flags&MSG_OOB)
		goto out;

	if (msg->msg_namelen) {
		err = unix_mkname(sunaddr, msg->msg_namelen, &hash);
		if (err < 0)
			goto out;
		namelen = err;
	} else {
		sunaddr = NULL;
		err = -ENOTCONN;
		other = unix_peer_get(sk);
		if (!other)
			goto out;
	}

	if (test_bit(SOCK_PASSCRED, &sock->flags) && !u->addr
	    && (err = unix_autobind(sock)) != 0)
		goto out;

	err = -EMSGSIZE;
	if (len > sk->sk_sndbuf - 32)
		goto out;

	skb = sock_alloc_send_skb(sk, len, msg->msg_flags&MSG_DONTWAIT, &err);
	if (skb == NULL)
		goto out;

	err = unix_scm_to_skb(siocb->scm, skb, true);
	if (err < 0)
		goto out_free;
	max_level = err + 1;
	unix_get_secdata(siocb->scm, skb);

	skb_reset_transport_header(skb);
	err = memcpy_fromiovec(skb_put(skb, len), msg->msg_iov, len);
	if (err)
		goto out_free;

	timeo = sock_sndtimeo(sk, msg->msg_flags & MSG_DONTWAIT);

restart:
	if (!other) {
		err = -ECONNRESET;
		if (sunaddr == NULL)
			goto out_free;

		other = unix_find_other(net, sunaddr, namelen, sk->sk_type,
					hash, &err);
		if (other == NULL)
			goto out_free;
	}

	unix_state_lock(other);
	err = -EPERM;
	if (!unix_may_send(sk, other))
		goto out_unlock;

	if (sock_flag(other, SOCK_DEAD)) {
		/*
		 *	Check with 1003.1g - what should
		 *	datagram error
		 */
		unix_state_unlock(other);
		sock_put(other);

		err = 0;
		unix_state_lock(sk);
		if (unix_peer(sk) == other) {
			unix_peer(sk) = NULL;
			unix_state_unlock(sk);

			unix_dgram_disconnected(sk, other);
			sock_put(other);
			err = -ECONNREFUSED;
		} else {
			unix_state_unlock(sk);
		}

		other = NULL;
		if (err)
			goto out_free;
		goto restart;
	}

	err = -EPIPE;
	if (other->sk_shutdown & RCV_SHUTDOWN)
		goto out_unlock;

	if (sk->sk_type != SOCK_SEQPACKET) {
		err = security_unix_may_send(sk->sk_socket, other->sk_socket);
		if (err)
			goto out_unlock;
	}

	if (unix_peer(other) != sk && unix_recvq_full(other)) {
		if (!timeo) {
			err = -EAGAIN;
			goto out_unlock;
		}

		timeo = unix_wait_for_peer(other, timeo);

		err = sock_intr_errno(timeo);
		if (signal_pending(current))
			goto out_free;

		goto restart;
	}

	if (sock_flag(other, SOCK_RCVTSTAMP))
		__net_timestamp(skb);
	skb_queue_tail(&other->sk_receive_queue, skb);
	if (max_level > unix_sk(other)->recursion_level)
		unix_sk(other)->recursion_level = max_level;
	unix_state_unlock(other);
	other->sk_data_ready(other, len);
	sock_put(other);
	scm_destroy(siocb->scm);
	return len;

out_unlock:
	unix_state_unlock(other);
out_free:
	kfree_skb(skb);
out:
	if (other)
		sock_put(other);
	scm_destroy(siocb->scm);
	return err;
}


static int unix_stream_sendmsg(struct kiocb *kiocb, struct socket *sock,
			       struct msghdr *msg, size_t len)
{
	struct sock_iocb *siocb = kiocb_to_siocb(kiocb);
	struct sock *sk = sock->sk;
	struct sock *other = NULL;
	struct sockaddr_un *sunaddr = msg->msg_name;
	int err, size;
	struct sk_buff *skb;
	int sent = 0;
	struct scm_cookie tmp_scm;
	bool fds_sent = false;
	int max_level;

	if (NULL == siocb->scm)
		siocb->scm = &tmp_scm;
	wait_for_unix_gc();
	err = scm_send(sock, msg, siocb->scm);
	if (err < 0)
		return err;

	err = -EOPNOTSUPP;
	if (msg->msg_flags&MSG_OOB)
		goto out_err;

	if (msg->msg_namelen) {
		err = sk->sk_state == TCP_ESTABLISHED ? -EISCONN : -EOPNOTSUPP;
		goto out_err;
	} else {
		sunaddr = NULL;
		err = -ENOTCONN;
		other = unix_peer(sk);
		if (!other)
			goto out_err;
	}

	if (sk->sk_shutdown & SEND_SHUTDOWN)
		goto pipe_err;

	while (sent < len) {
		/*
		 *	Optimisation for the fact that under 0.01% of X
		 *	messages typically need breaking up.
		 */

		size = len-sent;

		/* Keep two messages in the pipe so it schedules better */
		if (size > ((sk->sk_sndbuf >> 1) - 64))
			size = (sk->sk_sndbuf >> 1) - 64;

		if (size > SKB_MAX_ALLOC)
			size = SKB_MAX_ALLOC;

		/*
		 *	Grab a buffer
		 */

		skb = sock_alloc_send_skb(sk, size, msg->msg_flags&MSG_DONTWAIT,
					  &err);

		if (skb == NULL)
			goto out_err;

		/*
		 *	If you pass two values to the sock_alloc_send_skb
		 *	it tries to grab the large buffer with GFP_NOFS
		 *	(which can fail easily), and if it fails grab the
		 *	fallback size buffer which is under a page and will
		 *	succeed. [Alan]
		 */
		size = min_t(int, size, skb_tailroom(skb));


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
			kfree_skb(skb);
			goto out_err;
		}

		unix_state_lock(other);

		if (sock_flag(other, SOCK_DEAD) ||
		    (other->sk_shutdown & RCV_SHUTDOWN))
			goto pipe_err_free;

		skb_queue_tail(&other->sk_receive_queue, skb);
		if (max_level > unix_sk(other)->recursion_level)
			unix_sk(other)->recursion_level = max_level;
		unix_state_unlock(other);
		other->sk_data_ready(other, size);
		sent += size;
	}

	scm_destroy(siocb->scm);
	siocb->scm = NULL;

	return sent;

pipe_err_free:
	unix_state_unlock(other);
	kfree_skb(skb);
pipe_err:
	if (sent == 0 && !(msg->msg_flags&MSG_NOSIGNAL))
		send_sig(SIGPIPE, current, 0);
	err = -EPIPE;
out_err:
	scm_destroy(siocb->scm);
	siocb->scm = NULL;
	return sent ? : err;
}

static int unix_seqpacket_sendmsg(struct kiocb *kiocb, struct socket *sock,
				  struct msghdr *msg, size_t len)
{
	int err;
	struct sock *sk = sock->sk;

	err = sock_error(sk);
	if (err)
		return err;

	if (sk->sk_state != TCP_ESTABLISHED)
		return -ENOTCONN;

	if (msg->msg_namelen)
		msg->msg_namelen = 0;

	return unix_dgram_sendmsg(kiocb, sock, msg, len);
}

static void unix_copy_addr(struct msghdr *msg, struct sock *sk)
{
	struct unix_sock *u = unix_sk(sk);

	msg->msg_namelen = 0;
	if (u->addr) {
		msg->msg_namelen = u->addr->len;
		memcpy(msg->msg_name, u->addr->name, u->addr->len);
	}
}

static int unix_dgram_recvmsg(struct kiocb *iocb, struct socket *sock,
			      struct msghdr *msg, size_t size,
			      int flags)
{
	struct sock_iocb *siocb = kiocb_to_siocb(iocb);
	struct scm_cookie tmp_scm;
	struct sock *sk = sock->sk;
	struct unix_sock *u = unix_sk(sk);
	int noblock = flags & MSG_DONTWAIT;
	struct sk_buff *skb;
	int err;

	err = -EOPNOTSUPP;
	if (flags&MSG_OOB)
		goto out;

	msg->msg_namelen = 0;

	mutex_lock(&u->readlock);

	skb = skb_recv_datagram(sk, flags, noblock, &err);
	if (!skb) {
		unix_state_lock(sk);
		/* Signal EOF on disconnected non-blocking SEQPACKET socket. */
		if (sk->sk_type == SOCK_SEQPACKET && err == -EAGAIN &&
		    (sk->sk_shutdown & RCV_SHUTDOWN))
			err = 0;
		unix_state_unlock(sk);
		goto out_unlock;
	}

	wake_up_interruptible_sync(&u->peer_wait);

	if (msg->msg_name)
		unix_copy_addr(msg, skb->sk);

	if (size > skb->len)
		size = skb->len;
	else if (size < skb->len)
		msg->msg_flags |= MSG_TRUNC;

	err = skb_copy_datagram_iovec(skb, 0, msg->msg_iov, size);
	if (err)
		goto out_free;

	if (sock_flag(sk, SOCK_RCVTSTAMP))
		__sock_recv_timestamp(msg, sk, skb);

	if (!siocb->scm) {
		siocb->scm = &tmp_scm;
		memset(&tmp_scm, 0, sizeof(tmp_scm));
	}
	scm_set_cred(siocb->scm, UNIXCB(skb).pid, UNIXCB(skb).cred);
	unix_set_secdata(siocb->scm, skb);

	if (!(flags & MSG_PEEK)) {
		if (UNIXCB(skb).fp)
			unix_detach_fds(siocb->scm, skb);
	} else {
		/* It is questionable: on PEEK we could:
		   - do not return fds - good, but too simple 8)
		   - return fds, and do not return them on read (old strategy,
		     apparently wrong)
		   - clone fds (I chose it for now, it is the most universal
		     solution)

		   POSIX 1003.1g does not actually define this clearly
		   at all. POSIX 1003.1g doesn't define a lot of things
		   clearly however!

		*/
		if (UNIXCB(skb).fp)
			siocb->scm->fp = scm_fp_dup(UNIXCB(skb).fp);
	}
	err = size;

	scm_recv(sock, msg, siocb->scm, flags);

out_free:
	skb_free_datagram(sk, skb);
out_unlock:
	mutex_unlock(&u->readlock);
out:
	return err;
}

/*
 *	Sleep until data has arrive. But check for races..
 */

static long unix_stream_data_wait(struct sock *sk, long timeo)
{
	DEFINE_WAIT(wait);

	unix_state_lock(sk);

	for (;;) {
		prepare_to_wait(sk_sleep(sk), &wait, TASK_INTERRUPTIBLE);

		if (!skb_queue_empty(&sk->sk_receive_queue) ||
		    sk->sk_err ||
		    (sk->sk_shutdown & RCV_SHUTDOWN) ||
		    signal_pending(current) ||
		    !timeo)
			break;

		set_bit(SOCK_ASYNC_WAITDATA, &sk->sk_socket->flags);
		unix_state_unlock(sk);
		timeo = schedule_timeout(timeo);
		unix_state_lock(sk);
		clear_bit(SOCK_ASYNC_WAITDATA, &sk->sk_socket->flags);
	}

	finish_wait(sk_sleep(sk), &wait);
	unix_state_unlock(sk);
	return timeo;
}



static int unix_stream_recvmsg(struct kiocb *iocb, struct socket *sock,
			       struct msghdr *msg, size_t size,
			       int flags)
{
	struct sock_iocb *siocb = kiocb_to_siocb(iocb);
	struct scm_cookie tmp_scm;
	struct sock *sk = sock->sk;
	struct unix_sock *u = unix_sk(sk);
	struct sockaddr_un *sunaddr = msg->msg_name;
	int copied = 0;
	int check_creds = 0;
	int target;
	int err = 0;
	long timeo;

	err = -EINVAL;
	if (sk->sk_state != TCP_ESTABLISHED)
		goto out;

	err = -EOPNOTSUPP;
	if (flags&MSG_OOB)
		goto out;

	target = sock_rcvlowat(sk, flags&MSG_WAITALL, size);
	timeo = sock_rcvtimeo(sk, flags&MSG_DONTWAIT);

	msg->msg_namelen = 0;

	/* Lock the socket to prevent queue disordering
	 * while sleeps in memcpy_tomsg
	 */

	if (!siocb->scm) {
		siocb->scm = &tmp_scm;
		memset(&tmp_scm, 0, sizeof(tmp_scm));
	}

	mutex_lock(&u->readlock);

	do {
		int chunk;
		struct sk_buff *skb;

		unix_state_lock(sk);
		skb = skb_dequeue(&sk->sk_receive_queue);
		if (skb == NULL) {
			unix_sk(sk)->recursion_level = 0;
			if (copied >= target)
				goto unlock;

			/*
			 *	POSIX 1003.1g mandates this order.
			 */

			err = sock_error(sk);
			if (err)
				goto unlock;
			if (sk->sk_shutdown & RCV_SHUTDOWN)
				goto unlock;

			unix_state_unlock(sk);
			err = -EAGAIN;
			if (!timeo)
				break;
			mutex_unlock(&u->readlock);

			timeo = unix_stream_data_wait(sk, timeo);

			if (signal_pending(current)) {
				err = sock_intr_errno(timeo);
				goto out;
			}
			mutex_lock(&u->readlock);
			continue;
 unlock:
			unix_state_unlock(sk);
			break;
		}
		unix_state_unlock(sk);

		if (check_creds) {
			/* Never glue messages from different writers */
			if ((UNIXCB(skb).pid  != siocb->scm->pid) ||
			    (UNIXCB(skb).cred != siocb->scm->cred)) {
				skb_queue_head(&sk->sk_receive_queue, skb);
				break;
			}
		} else {
			/* Copy credentials */
			scm_set_cred(siocb->scm, UNIXCB(skb).pid, UNIXCB(skb).cred);
			check_creds = 1;
		}

		/* Copy address just once */
		if (sunaddr) {
			unix_copy_addr(msg, skb->sk);
			sunaddr = NULL;
		}

		chunk = min_t(unsigned int, skb->len, size);
		if (memcpy_toiovec(msg->msg_iov, skb->data, chunk)) {
			skb_queue_head(&sk->sk_receive_queue, skb);
			if (copied == 0)
				copied = -EFAULT;
			break;
		}
		copied += chunk;
		size -= chunk;

		/* Mark read part of skb as used */
		if (!(flags & MSG_PEEK)) {
			skb_pull(skb, chunk);

			if (UNIXCB(skb).fp)
				unix_detach_fds(siocb->scm, skb);

			/* put the skb back if we didn't use it up.. */
			if (skb->len) {
				skb_queue_head(&sk->sk_receive_queue, skb);
				break;
			}

			consume_skb(skb);

			if (siocb->scm->fp)
				break;
		} else {
			/* It is questionable, see note in unix_dgram_recvmsg.
			 */
			if (UNIXCB(skb).fp)
				siocb->scm->fp = scm_fp_dup(UNIXCB(skb).fp);

			/* put message back and return */
			skb_queue_head(&sk->sk_receive_queue, skb);
			break;
		}
	} while (size);

	mutex_unlock(&u->readlock);
	scm_recv(sock, msg, siocb->scm, flags);
out:
	return copied ? : err;
}

static int unix_shutdown(struct socket *sock, int mode)
{
	struct sock *sk = sock->sk;
	struct sock *other;

	mode = (mode+1)&(RCV_SHUTDOWN|SEND_SHUTDOWN);

	if (mode) {
		unix_state_lock(sk);
		sk->sk_shutdown |= mode;
		other = unix_peer(sk);
		if (other)
			sock_hold(other);
		unix_state_unlock(sk);
		sk->sk_state_change(sk);

		if (other &&
			(sk->sk_type == SOCK_STREAM || sk->sk_type == SOCK_SEQPACKET)) {

			int peer_mode = 0;

			if (mode&RCV_SHUTDOWN)
				peer_mode |= SEND_SHUTDOWN;
			if (mode&SEND_SHUTDOWN)
				peer_mode |= RCV_SHUTDOWN;
			unix_state_lock(other);
			other->sk_shutdown |= peer_mode;
			unix_state_unlock(other);
			other->sk_state_change(other);
			if (peer_mode == SHUTDOWN_MASK)
				sk_wake_async(other, SOCK_WAKE_WAITD, POLL_HUP);
			else if (peer_mode & RCV_SHUTDOWN)
				sk_wake_async(other, SOCK_WAKE_WAITD, POLL_IN);
		}
		if (other)
			sock_put(other);
	}
	return 0;
}

static int unix_ioctl(struct socket *sock, unsigned int cmd, unsigned long arg)
{
	struct sock *sk = sock->sk;
	long amount = 0;
	int err;

	switch (cmd) {
	case SIOCOUTQ:
		amount = sk_wmem_alloc_get(sk);
		err = put_user(amount, (int __user *)arg);
		break;
	case SIOCINQ:
		{
			struct sk_buff *skb;

			if (sk->sk_state == TCP_LISTEN) {
				err = -EINVAL;
				break;
			}

			spin_lock(&sk->sk_receive_queue.lock);
			if (sk->sk_type == SOCK_STREAM ||
			    sk->sk_type == SOCK_SEQPACKET) {
				skb_queue_walk(&sk->sk_receive_queue, skb)
					amount += skb->len;
			} else {
				skb = skb_peek(&sk->sk_receive_queue);
				if (skb)
					amount = skb->len;
			}
			spin_unlock(&sk->sk_receive_queue.lock);
			err = put_user(amount, (int __user *)arg);
			break;
		}

	default:
		err = -ENOIOCTLCMD;
		break;
	}
	return err;
}

static unsigned int unix_poll(struct file *file, struct socket *sock, poll_table *wait)
{
	struct sock *sk = sock->sk;
	unsigned int mask;

	sock_poll_wait(file, sk_sleep(sk), wait);
	mask = 0;

	/* exceptional events? */
	if (sk->sk_err)
		mask |= POLLERR;
	if (sk->sk_shutdown == SHUTDOWN_MASK)
		mask |= POLLHUP;
	if (sk->sk_shutdown & RCV_SHUTDOWN)
		mask |= POLLRDHUP | POLLIN | POLLRDNORM;

	/* readable? */
	if (!skb_queue_empty(&sk->sk_receive_queue))
		mask |= POLLIN | POLLRDNORM;

	/* Connection-based need to check for termination and startup */
	if ((sk->sk_type == SOCK_STREAM || sk->sk_type == SOCK_SEQPACKET) &&
	    sk->sk_state == TCP_CLOSE)
		mask |= POLLHUP;

	/*
	 * we set writable also when the other side has shut down the
	 * connection. This prevents stuck sockets.
	 */
	if (unix_writable(sk))
		mask |= POLLOUT | POLLWRNORM | POLLWRBAND;

	return mask;
}

static unsigned int unix_dgram_poll(struct file *file, struct socket *sock,
				    poll_table *wait)
{
	struct sock *sk = sock->sk, *other;
	unsigned int mask, writable;

	sock_poll_wait(file, sk_sleep(sk), wait);
	mask = 0;

	/* exceptional events? */
	if (sk->sk_err || !skb_queue_empty(&sk->sk_error_queue))
		mask |= POLLERR;
	if (sk->sk_shutdown & RCV_SHUTDOWN)
		mask |= POLLRDHUP;
	if (sk->sk_shutdown == SHUTDOWN_MASK)
		mask |= POLLHUP;

	/* readable? */
	if (!skb_queue_empty(&sk->sk_receive_queue) ||
	    (sk->sk_shutdown & RCV_SHUTDOWN))
		mask |= POLLIN | POLLRDNORM;

	/* Connection-based need to check for termination and startup */
	if (sk->sk_type == SOCK_SEQPACKET) {
		if (sk->sk_state == TCP_CLOSE)
			mask |= POLLHUP;
		/* connection hasn't started yet? */
		if (sk->sk_state == TCP_SYN_SENT)
			return mask;
	}

	/* writable? */
	writable = unix_writable(sk);
	if (writable) {
		other = unix_peer_get(sk);
		if (other) {
			if (unix_peer(other) != sk) {
				sock_poll_wait(file, &unix_sk(other)->peer_wait,
					  wait);
				if (unix_recvq_full(other))
					writable = 0;
			}

			sock_put(other);
		}
	}

	if (writable)
		mask |= POLLOUT | POLLWRNORM | POLLWRBAND;
	else
		set_bit(SOCK_ASYNC_NOSPACE, &sk->sk_socket->flags);

	return mask;
}

#ifdef CONFIG_PROC_FS
static struct sock *first_unix_socket(int *i)
{
	for (*i = 0; *i <= UNIX_HASH_SIZE; (*i)++) {
		if (!hlist_empty(&unix_socket_table[*i]))
			return __sk_head(&unix_socket_table[*i]);
	}
	return NULL;
}

static struct sock *next_unix_socket(int *i, struct sock *s)
{
	struct sock *next = sk_next(s);
	/* More in this chain? */
	if (next)
		return next;
	/* Look for next non-empty chain. */
	for ((*i)++; *i <= UNIX_HASH_SIZE; (*i)++) {
		if (!hlist_empty(&unix_socket_table[*i]))
			return __sk_head(&unix_socket_table[*i]);
	}
	return NULL;
}

struct unix_iter_state {
	struct seq_net_private p;
	int i;
};

static struct sock *unix_seq_idx(struct seq_file *seq, loff_t pos)
{
	struct unix_iter_state *iter = seq->private;
	loff_t off = 0;
	struct sock *s;

	for (s = first_unix_socket(&iter->i); s; s = next_unix_socket(&iter->i, s)) {
		if (sock_net(s) != seq_file_net(seq))
			continue;
		if (off == pos)
			return s;
		++off;
	}
	return NULL;
}

static void *unix_seq_start(struct seq_file *seq, loff_t *pos)
	__acquires(unix_table_lock)
{
	spin_lock(&unix_table_lock);
	return *pos ? unix_seq_idx(seq, *pos - 1) : SEQ_START_TOKEN;
}

static void *unix_seq_next(struct seq_file *seq, void *v, loff_t *pos)
{
	struct unix_iter_state *iter = seq->private;
	struct sock *sk = v;
	++*pos;

	if (v == SEQ_START_TOKEN)
		sk = first_unix_socket(&iter->i);
	else
		sk = next_unix_socket(&iter->i, sk);
	while (sk && (sock_net(sk) != seq_file_net(seq)))
		sk = next_unix_socket(&iter->i, sk);
	return sk;
}

static void unix_seq_stop(struct seq_file *seq, void *v)
	__releases(unix_table_lock)
{
	spin_unlock(&unix_table_lock);
}

static int unix_seq_show(struct seq_file *seq, void *v)
{

	if (v == SEQ_START_TOKEN)
		seq_puts(seq, "Num       RefCount Protocol Flags    Type St "
			 "Inode Path\n");
	else {
		struct sock *s = v;
		struct unix_sock *u = unix_sk(s);
		unix_state_lock(s);

		seq_printf(seq, "%p: %08X %08X %08X %04X %02X %5lu",
			s,
			atomic_read(&s->sk_refcnt),
			0,
			s->sk_state == TCP_LISTEN ? __SO_ACCEPTCON : 0,
			s->sk_type,
			s->sk_socket ?
			(s->sk_state == TCP_ESTABLISHED ? SS_CONNECTED : SS_UNCONNECTED) :
			(s->sk_state == TCP_ESTABLISHED ? SS_CONNECTING : SS_DISCONNECTING),
			sock_i_ino(s));

		if (u->addr) {
			int i, len;
			seq_putc(seq, ' ');

			i = 0;
			len = u->addr->len - sizeof(short);
			if (!UNIX_ABSTRACT(s))
				len--;
			else {
				seq_putc(seq, '@');
				i++;
			}
			for ( ; i < len; i++)
				seq_putc(seq, u->addr->name->sun_path[i]);
		}
		unix_state_unlock(s);
		seq_putc(seq, '\n');
	}

	return 0;
}

static const struct seq_operations unix_seq_ops = {
	.start  = unix_seq_start,
	.next   = unix_seq_next,
	.stop   = unix_seq_stop,
	.show   = unix_seq_show,
};

static int unix_seq_open(struct inode *inode, struct file *file)
{
	return seq_open_net(inode, file, &unix_seq_ops,
			    sizeof(struct unix_iter_state));
}

static const struct file_operations unix_seq_fops = {
	.owner		= THIS_MODULE,
	.open		= unix_seq_open,
	.read		= seq_read,
	.llseek		= seq_lseek,
	.release	= seq_release_net,
};

#endif

static const struct net_proto_family unix_family_ops = {
	.family = PF_UNIX,
	.create = unix_create,
	.owner	= THIS_MODULE,
};


static int __net_init unix_net_init(struct net *net)
{
	int error = -ENOMEM;

	net->unx.sysctl_max_dgram_qlen = 10;
	if (unix_sysctl_register(net))
		goto out;

#ifdef CONFIG_PROC_FS
	if (!proc_net_fops_create(net, "unix", 0, &unix_seq_fops)) {
		unix_sysctl_unregister(net);
		goto out;
	}
#endif
	error = 0;
out:
	return error;
}

static void __net_exit unix_net_exit(struct net *net)
{
	unix_sysctl_unregister(net);
	proc_net_remove(net, "unix");
}

static struct pernet_operations unix_net_ops = {
	.init = unix_net_init,
	.exit = unix_net_exit,
};

static int __init af_unix_init(void)
{
	int rc = -1;
	struct sk_buff *dummy_skb;

	BUILD_BUG_ON(sizeof(struct unix_skb_parms) > sizeof(dummy_skb->cb));

	rc = proto_register(&unix_proto, 1);
	if (rc != 0) {
		printk(KERN_CRIT "%s: Cannot create unix_sock SLAB cache!\n",
		       __func__);
		goto out;
	}

	sock_register(&unix_family_ops);
	register_pernet_subsys(&unix_net_ops);
out:
	return rc;
}

static void __exit af_unix_exit(void)
{
	sock_unregister(PF_UNIX);
	proto_unregister(&unix_proto);
	unregister_pernet_subsys(&unix_net_ops);
}

/* Earlier than device_initcall() so that other drivers invoking
   request_module() don't end up in a loop when modprobe tries
   to use a UNIX socket. But later than subsys_initcall() because
   we depend on stuff initialised there */
fs_initcall(af_unix_init);
module_exit(af_unix_exit);

MODULE_LICENSE("GPL");
MODULE_ALIAS_NETPROTO(PF_UNIX);
	} else {
		sunaddr = NULL;
		err = -ENOTCONN;
		other = unix_peer_get(sk);
		if (!other)
			goto out;
	}

	if (test_bit(SOCK_PASSCRED, &sock->flags) && !u->addr
	    && (err = unix_autobind(sock)) != 0)
		goto out;

	err = -EMSGSIZE;
	if (len > sk->sk_sndbuf - 32)
		goto out;

	skb = sock_alloc_send_skb(sk, len, msg->msg_flags&MSG_DONTWAIT, &err);
	if (skb == NULL)
		goto out;

	err = unix_scm_to_skb(siocb->scm, skb, true);
	if (err < 0)
		goto out_free;
	max_level = err + 1;
	unix_get_secdata(siocb->scm, skb);

	skb_reset_transport_header(skb);
	err = memcpy_fromiovec(skb_put(skb, len), msg->msg_iov, len);
	if (err)
		goto out_free;

	timeo = sock_sndtimeo(sk, msg->msg_flags & MSG_DONTWAIT);

restart:
	if (!other) {
		err = -ECONNRESET;
		if (sunaddr == NULL)
			goto out_free;

		other = unix_find_other(net, sunaddr, namelen, sk->sk_type,
					hash, &err);
		if (other == NULL)
			goto out_free;
	}
		if (!timeo) {
			err = -EAGAIN;
			goto out_unlock;
		}

		timeo = unix_wait_for_peer(other, timeo);

		err = sock_intr_errno(timeo);
		if (signal_pending(current))
			goto out_free;

		goto restart;
	}

	if (sock_flag(other, SOCK_RCVTSTAMP))
		__net_timestamp(skb);
	skb_queue_tail(&other->sk_receive_queue, skb);
	if (max_level > unix_sk(other)->recursion_level)
		unix_sk(other)->recursion_level = max_level;
	unix_state_unlock(other);
	other->sk_data_ready(other, len);
	sock_put(other);
	scm_destroy(siocb->scm);
	return len;

out_unlock:
	unix_state_unlock(other);
out_free:
	kfree_skb(skb);
out:
	if (other)
		sock_put(other);
	scm_destroy(siocb->scm);
	return err;
}


static int unix_stream_sendmsg(struct kiocb *kiocb, struct socket *sock,
			       struct msghdr *msg, size_t len)
{
	struct sock_iocb *siocb = kiocb_to_siocb(kiocb);
	struct sock *sk = sock->sk;
	struct sock *other = NULL;
	struct sockaddr_un *sunaddr = msg->msg_name;
	int err, size;
	struct sk_buff *skb;
	int sent = 0;
	struct scm_cookie tmp_scm;
	bool fds_sent = false;
	int max_level;

	if (NULL == siocb->scm)
		siocb->scm = &tmp_scm;
	wait_for_unix_gc();
	err = scm_send(sock, msg, siocb->scm);
	if (err < 0)
		return err;

	err = -EOPNOTSUPP;
	if (msg->msg_flags&MSG_OOB)
		goto out_err;

	if (msg->msg_namelen) {
		err = sk->sk_state == TCP_ESTABLISHED ? -EISCONN : -EOPNOTSUPP;
		goto out_err;
	} else {
		sunaddr = NULL;
		err = -ENOTCONN;
		other = unix_peer(sk);
		if (!other)
			goto out_err;
	}

	if (sk->sk_shutdown & SEND_SHUTDOWN)
		goto pipe_err;

	while (sent < len) {
		/*
		 *	Optimisation for the fact that under 0.01% of X
		 *	messages typically need breaking up.
		 */

		size = len-sent;

		/* Keep two messages in the pipe so it schedules better */
		if (size > ((sk->sk_sndbuf >> 1) - 64))
			size = (sk->sk_sndbuf >> 1) - 64;

		if (size > SKB_MAX_ALLOC)
			size = SKB_MAX_ALLOC;

		/*
		 *	Grab a buffer
		 */

		skb = sock_alloc_send_skb(sk, size, msg->msg_flags&MSG_DONTWAIT,
					  &err);

		if (skb == NULL)
			goto out_err;

		/*
		 *	If you pass two values to the sock_alloc_send_skb
		 *	it tries to grab the large buffer with GFP_NOFS
		 *	(which can fail easily), and if it fails grab the
		 *	fallback size buffer which is under a page and will
		 *	succeed. [Alan]
		 */
		size = min_t(int, size, skb_tailroom(skb));


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
			kfree_skb(skb);
			goto out_err;
		}

		unix_state_lock(other);

		if (sock_flag(other, SOCK_DEAD) ||
		    (other->sk_shutdown & RCV_SHUTDOWN))
			goto pipe_err_free;

		skb_queue_tail(&other->sk_receive_queue, skb);
		if (max_level > unix_sk(other)->recursion_level)
			unix_sk(other)->recursion_level = max_level;
		unix_state_unlock(other);
		other->sk_data_ready(other, size);
		sent += size;
	}

	scm_destroy(siocb->scm);
	siocb->scm = NULL;

	return sent;

pipe_err_free:
	unix_state_unlock(other);
	kfree_skb(skb);
pipe_err:
	if (sent == 0 && !(msg->msg_flags&MSG_NOSIGNAL))
		send_sig(SIGPIPE, current, 0);
	err = -EPIPE;
out_err:
	scm_destroy(siocb->scm);
	siocb->scm = NULL;
	return sent ? : err;
}
{
	struct sock_iocb *siocb = kiocb_to_siocb(kiocb);
	struct sock *sk = sock->sk;
	struct sock *other = NULL;
	struct sockaddr_un *sunaddr = msg->msg_name;
	int err, size;
	struct sk_buff *skb;
	int sent = 0;
	struct scm_cookie tmp_scm;
	bool fds_sent = false;
	int max_level;

	if (NULL == siocb->scm)
		siocb->scm = &tmp_scm;
	wait_for_unix_gc();
	err = scm_send(sock, msg, siocb->scm);
	if (err < 0)
		return err;

	err = -EOPNOTSUPP;
	if (msg->msg_flags&MSG_OOB)
		goto out_err;

	if (msg->msg_namelen) {
		err = sk->sk_state == TCP_ESTABLISHED ? -EISCONN : -EOPNOTSUPP;
		goto out_err;
	} else {
		sunaddr = NULL;
		err = -ENOTCONN;
		other = unix_peer(sk);
		if (!other)
			goto out_err;
	}

	if (sk->sk_shutdown & SEND_SHUTDOWN)
		goto pipe_err;

	while (sent < len) {
		/*
		 *	Optimisation for the fact that under 0.01% of X
		 *	messages typically need breaking up.
		 */

		size = len-sent;

		/* Keep two messages in the pipe so it schedules better */
		if (size > ((sk->sk_sndbuf >> 1) - 64))
			size = (sk->sk_sndbuf >> 1) - 64;

		if (size > SKB_MAX_ALLOC)
			size = SKB_MAX_ALLOC;

		/*
		 *	Grab a buffer
		 */

		skb = sock_alloc_send_skb(sk, size, msg->msg_flags&MSG_DONTWAIT,
					  &err);

		if (skb == NULL)
			goto out_err;

		/*
		 *	If you pass two values to the sock_alloc_send_skb
		 *	it tries to grab the large buffer with GFP_NOFS
		 *	(which can fail easily), and if it fails grab the
		 *	fallback size buffer which is under a page and will
		 *	succeed. [Alan]
		 */
		size = min_t(int, size, skb_tailroom(skb));


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
			kfree_skb(skb);
			goto out_err;
		}

		unix_state_lock(other);

		if (sock_flag(other, SOCK_DEAD) ||
		    (other->sk_shutdown & RCV_SHUTDOWN))
			goto pipe_err_free;

		skb_queue_tail(&other->sk_receive_queue, skb);
		if (max_level > unix_sk(other)->recursion_level)
			unix_sk(other)->recursion_level = max_level;
		unix_state_unlock(other);
		other->sk_data_ready(other, size);
		sent += size;
	}

	scm_destroy(siocb->scm);
	siocb->scm = NULL;

	return sent;

pipe_err_free:
	unix_state_unlock(other);
	kfree_skb(skb);
pipe_err:
	if (sent == 0 && !(msg->msg_flags&MSG_NOSIGNAL))
		send_sig(SIGPIPE, current, 0);
	err = -EPIPE;
out_err:
	scm_destroy(siocb->scm);
	siocb->scm = NULL;
	return sent ? : err;
}
	while (sent < len) {
		/*
		 *	Optimisation for the fact that under 0.01% of X
		 *	messages typically need breaking up.
		 */

		size = len-sent;

		/* Keep two messages in the pipe so it schedules better */
		if (size > ((sk->sk_sndbuf >> 1) - 64))
			size = (sk->sk_sndbuf >> 1) - 64;

		if (size > SKB_MAX_ALLOC)
			size = SKB_MAX_ALLOC;

		/*
		 *	Grab a buffer
		 */

		skb = sock_alloc_send_skb(sk, size, msg->msg_flags&MSG_DONTWAIT,
					  &err);

		if (skb == NULL)
			goto out_err;

		/*
		 *	If you pass two values to the sock_alloc_send_skb
		 *	it tries to grab the large buffer with GFP_NOFS
		 *	(which can fail easily), and if it fails grab the
		 *	fallback size buffer which is under a page and will
		 *	succeed. [Alan]
		 */
		size = min_t(int, size, skb_tailroom(skb));


		/* Only send the fds in the first buffer */
		err = unix_scm_to_skb(siocb->scm, skb, !fds_sent);
		if (err < 0) {
			kfree_skb(skb);
			goto out_err;
		}
		if (err) {
			kfree_skb(skb);
			goto out_err;
		}

		unix_state_lock(other);

		if (sock_flag(other, SOCK_DEAD) ||
		    (other->sk_shutdown & RCV_SHUTDOWN))
			goto pipe_err_free;

		skb_queue_tail(&other->sk_receive_queue, skb);
		if (max_level > unix_sk(other)->recursion_level)
			unix_sk(other)->recursion_level = max_level;
		unix_state_unlock(other);
		other->sk_data_ready(other, size);
		sent += size;
	}

	scm_destroy(siocb->scm);
	siocb->scm = NULL;

	return sent;

pipe_err_free:
	unix_state_unlock(other);
	kfree_skb(skb);
pipe_err:
	if (sent == 0 && !(msg->msg_flags&MSG_NOSIGNAL))
		send_sig(SIGPIPE, current, 0);
	err = -EPIPE;
out_err:
	scm_destroy(siocb->scm);
	siocb->scm = NULL;
	return sent ? : err;
}

static int unix_seqpacket_sendmsg(struct kiocb *kiocb, struct socket *sock,
				  struct msghdr *msg, size_t len)
{
	int err;
	struct sock *sk = sock->sk;

	err = sock_error(sk);
	if (err)
		return err;

	if (sk->sk_state != TCP_ESTABLISHED)
		return -ENOTCONN;

	if (msg->msg_namelen)
		msg->msg_namelen = 0;

	return unix_dgram_sendmsg(kiocb, sock, msg, len);
}

static void unix_copy_addr(struct msghdr *msg, struct sock *sk)
{
	struct unix_sock *u = unix_sk(sk);

	msg->msg_namelen = 0;
	if (u->addr) {
		msg->msg_namelen = u->addr->len;
		memcpy(msg->msg_name, u->addr->name, u->addr->len);
	}
}

static int unix_dgram_recvmsg(struct kiocb *iocb, struct socket *sock,
			      struct msghdr *msg, size_t size,
			      int flags)
{
	struct sock_iocb *siocb = kiocb_to_siocb(iocb);
	struct scm_cookie tmp_scm;
	struct sock *sk = sock->sk;
	struct unix_sock *u = unix_sk(sk);
	int noblock = flags & MSG_DONTWAIT;
	struct sk_buff *skb;
	int err;

	err = -EOPNOTSUPP;
	if (flags&MSG_OOB)
		goto out;

	msg->msg_namelen = 0;

	mutex_lock(&u->readlock);

	skb = skb_recv_datagram(sk, flags, noblock, &err);
	if (!skb) {
		unix_state_lock(sk);
		/* Signal EOF on disconnected non-blocking SEQPACKET socket. */
		if (sk->sk_type == SOCK_SEQPACKET && err == -EAGAIN &&
		    (sk->sk_shutdown & RCV_SHUTDOWN))
			err = 0;
		unix_state_unlock(sk);
		goto out_unlock;
	}

	wake_up_interruptible_sync(&u->peer_wait);

	if (msg->msg_name)
		unix_copy_addr(msg, skb->sk);

	if (size > skb->len)
		size = skb->len;
	else if (size < skb->len)
		msg->msg_flags |= MSG_TRUNC;

	err = skb_copy_datagram_iovec(skb, 0, msg->msg_iov, size);
	if (err)
		goto out_free;

	if (sock_flag(sk, SOCK_RCVTSTAMP))
		__sock_recv_timestamp(msg, sk, skb);

	if (!siocb->scm) {
		siocb->scm = &tmp_scm;
		memset(&tmp_scm, 0, sizeof(tmp_scm));
	}
	scm_set_cred(siocb->scm, UNIXCB(skb).pid, UNIXCB(skb).cred);
	unix_set_secdata(siocb->scm, skb);

	if (!(flags & MSG_PEEK)) {
		if (UNIXCB(skb).fp)
			unix_detach_fds(siocb->scm, skb);
	} else {
		/* It is questionable: on PEEK we could:
		   - do not return fds - good, but too simple 8)
		   - return fds, and do not return them on read (old strategy,
		     apparently wrong)
		   - clone fds (I chose it for now, it is the most universal
		     solution)

		   POSIX 1003.1g does not actually define this clearly
		   at all. POSIX 1003.1g doesn't define a lot of things
		   clearly however!

		*/
		if (UNIXCB(skb).fp)
			siocb->scm->fp = scm_fp_dup(UNIXCB(skb).fp);
	}
	do {
		int chunk;
		struct sk_buff *skb;

		unix_state_lock(sk);
		skb = skb_dequeue(&sk->sk_receive_queue);
		if (skb == NULL) {
			unix_sk(sk)->recursion_level = 0;
			if (copied >= target)
				goto unlock;

			/*
			 *	POSIX 1003.1g mandates this order.
			 */

			err = sock_error(sk);
			if (err)
				goto unlock;
			if (sk->sk_shutdown & RCV_SHUTDOWN)
				goto unlock;

			unix_state_unlock(sk);
			err = -EAGAIN;
			if (!timeo)
				break;
			mutex_unlock(&u->readlock);

			timeo = unix_stream_data_wait(sk, timeo);

			if (signal_pending(current)) {
				err = sock_intr_errno(timeo);
				goto out;
			}
			mutex_lock(&u->readlock);
			continue;
 unlock:
			unix_state_unlock(sk);
			break;
		}
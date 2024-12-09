		    (sock_flag(sk, SOCK_LINGER) && !sk->sk_lingertime)) {
			struct sctp_chunk *chunk;

			chunk = sctp_make_abort_user(asoc, NULL, 0);
			sctp_primitive_ABORT(net, asoc, chunk);
		} else
			sctp_primitive_SHUTDOWN(net, asoc, NULL);
	}

	/* On a TCP-style socket, block for at most linger_time if set. */
	if (sctp_style(sk, TCP) && timeout)
		sctp_wait_for_close(sk, timeout);

	/* This will run the backlog queue.  */
	release_sock(sk);

	/* Supposedly, no process has access to the socket, but
	 * the net layers still may.
	 * Also, sctp_destroy_sock() needs to be called with addr_wq_lock
	 * held and that should be grabbed before socket lock.
	 */
	spin_lock_bh(&net->sctp.addr_wq_lock);
	bh_lock_sock_nested(sk);

	/* Hold the sock, since sk_common_release() will put sock_put()
	 * and we have just a little more cleanup.
	 */
	sock_hold(sk);
	sk_common_release(sk);

	bh_unlock_sock(sk);
	spin_unlock_bh(&net->sctp.addr_wq_lock);

	sock_put(sk);

	SCTP_DBG_OBJCNT_DEC(sock);
}

/* Handle EPIPE error. */
static int sctp_error(struct sock *sk, int flags, int err)
{
	if (err == -EPIPE)
		err = sock_error(sk) ? : -EPIPE;
	if (err == -EPIPE && !(flags & MSG_NOSIGNAL))
		send_sig(SIGPIPE, current, 0);
	return err;
}

/* API 3.1.3 sendmsg() - UDP Style Syntax
 *
 * An application uses sendmsg() and recvmsg() calls to transmit data to
 * and receive data from its peer.
 *
 *  ssize_t sendmsg(int socket, const struct msghdr *message,
 *                  int flags);
 *
 *  socket  - the socket descriptor of the endpoint.
 *  message - pointer to the msghdr structure which contains a single
 *            user message and possibly some ancillary data.
 *
 *            See Section 5 for complete description of the data
 *            structures.
 *
 *  flags   - flags sent or received with the user message, see Section
 *            5 for complete description of the flags.
 *
 * Note:  This function could use a rewrite especially when explicit
 * connect support comes in.
 */
/* BUG:  We do not implement the equivalent of sk_stream_wait_memory(). */

static int sctp_msghdr_parse(const struct msghdr *msg,
			     struct sctp_cmsgs *cmsgs);

static int sctp_sendmsg_parse(struct sock *sk, struct sctp_cmsgs *cmsgs,
			      struct sctp_sndrcvinfo *srinfo,
			      const struct msghdr *msg, size_t msg_len)
{
	__u16 sflags;
	int err;

	if (sctp_sstate(sk, LISTENING) && sctp_style(sk, TCP))
		return -EPIPE;

	if (msg_len > sk->sk_sndbuf)
		return -EMSGSIZE;

	memset(cmsgs, 0, sizeof(*cmsgs));
	err = sctp_msghdr_parse(msg, cmsgs);
	if (err) {
		pr_debug("%s: msghdr parse err:%x\n", __func__, err);
		return err;
	}
		    (sock_flag(sk, SOCK_LINGER) && !sk->sk_lingertime)) {
			struct sctp_chunk *chunk;

			chunk = sctp_make_abort_user(asoc, NULL, 0);
			sctp_primitive_ABORT(net, asoc, chunk);
		} else
			sctp_primitive_SHUTDOWN(net, asoc, NULL);
	}

	/* On a TCP-style socket, block for at most linger_time if set. */
	if (sctp_style(sk, TCP) && timeout)
		sctp_wait_for_close(sk, timeout);

	/* This will run the backlog queue.  */
	release_sock(sk);

	/* Supposedly, no process has access to the socket, but
	 * the net layers still may.
	 * Also, sctp_destroy_sock() needs to be called with addr_wq_lock
	 * held and that should be grabbed before socket lock.
	 */
	spin_lock_bh(&net->sctp.addr_wq_lock);
	bh_lock_sock_nested(sk);

	/* Hold the sock, since sk_common_release() will put sock_put()
	 * and we have just a little more cleanup.
	 */
	sock_hold(sk);
	sk_common_release(sk);

	bh_unlock_sock(sk);
	spin_unlock_bh(&net->sctp.addr_wq_lock);

	sock_put(sk);

	SCTP_DBG_OBJCNT_DEC(sock);
}

/* Handle EPIPE error. */
static int sctp_error(struct sock *sk, int flags, int err)
{
	if (err == -EPIPE)
		err = sock_error(sk) ? : -EPIPE;
	if (err == -EPIPE && !(flags & MSG_NOSIGNAL))
		send_sig(SIGPIPE, current, 0);
	return err;
}

/* API 3.1.3 sendmsg() - UDP Style Syntax
 *
 * An application uses sendmsg() and recvmsg() calls to transmit data to
 * and receive data from its peer.
 *
 *  ssize_t sendmsg(int socket, const struct msghdr *message,
 *                  int flags);
 *
 *  socket  - the socket descriptor of the endpoint.
 *  message - pointer to the msghdr structure which contains a single
 *            user message and possibly some ancillary data.
 *
 *            See Section 5 for complete description of the data
 *            structures.
 *
 *  flags   - flags sent or received with the user message, see Section
 *            5 for complete description of the flags.
 *
 * Note:  This function could use a rewrite especially when explicit
 * connect support comes in.
 */
/* BUG:  We do not implement the equivalent of sk_stream_wait_memory(). */

static int sctp_msghdr_parse(const struct msghdr *msg,
			     struct sctp_cmsgs *cmsgs);

static int sctp_sendmsg_parse(struct sock *sk, struct sctp_cmsgs *cmsgs,
			      struct sctp_sndrcvinfo *srinfo,
			      const struct msghdr *msg, size_t msg_len)
{
	__u16 sflags;
	int err;

	if (sctp_sstate(sk, LISTENING) && sctp_style(sk, TCP))
		return -EPIPE;

	if (msg_len > sk->sk_sndbuf)
		return -EMSGSIZE;

	memset(cmsgs, 0, sizeof(*cmsgs));
	err = sctp_msghdr_parse(msg, cmsgs);
	if (err) {
		pr_debug("%s: msghdr parse err:%x\n", __func__, err);
		return err;
	}
	switch (sk->sk_type) {
	case SOCK_SEQPACKET:
		sp->type = SCTP_SOCKET_UDP;
		break;
	case SOCK_STREAM:
		sp->type = SCTP_SOCKET_TCP;
		break;
	default:
		return -ESOCKTNOSUPPORT;
	}

	sk->sk_gso_type = SKB_GSO_SCTP;

	/* Initialize default send parameters. These parameters can be
	 * modified with the SCTP_DEFAULT_SEND_PARAM socket option.
	 */
	sp->default_stream = 0;
	sp->default_ppid = 0;
	sp->default_flags = 0;
	sp->default_context = 0;
	sp->default_timetolive = 0;

	sp->default_rcv_context = 0;
	sp->max_burst = net->sctp.max_burst;

	sp->sctp_hmac_alg = net->sctp.sctp_hmac_alg;

	/* Initialize default setup parameters. These parameters
	 * can be modified with the SCTP_INITMSG socket option or
	 * overridden by the SCTP_INIT CMSG.
	 */
	sp->initmsg.sinit_num_ostreams   = sctp_max_outstreams;
	sp->initmsg.sinit_max_instreams  = sctp_max_instreams;
	sp->initmsg.sinit_max_attempts   = net->sctp.max_retrans_init;
	sp->initmsg.sinit_max_init_timeo = net->sctp.rto_max;

	/* Initialize default RTO related parameters.  These parameters can
	 * be modified for with the SCTP_RTOINFO socket option.
	 */
	sp->rtoinfo.srto_initial = net->sctp.rto_initial;
	sp->rtoinfo.srto_max     = net->sctp.rto_max;
	sp->rtoinfo.srto_min     = net->sctp.rto_min;

	/* Initialize default association related parameters. These parameters
	 * can be modified with the SCTP_ASSOCINFO socket option.
	 */
	sp->assocparams.sasoc_asocmaxrxt = net->sctp.max_retrans_association;
	sp->assocparams.sasoc_number_peer_destinations = 0;
	sp->assocparams.sasoc_peer_rwnd = 0;
	sp->assocparams.sasoc_local_rwnd = 0;
	sp->assocparams.sasoc_cookie_life = net->sctp.valid_cookie_life;

	/* Initialize default event subscriptions. By default, all the
	 * options are off.
	 */
	sp->subscribe = 0;

	/* Default Peer Address Parameters.  These defaults can
	 * be modified via SCTP_PEER_ADDR_PARAMS
	 */
	sp->hbinterval  = net->sctp.hb_interval;
	sp->udp_port    = htons(net->sctp.udp_port);
	sp->encap_port  = htons(net->sctp.encap_port);
	sp->pathmaxrxt  = net->sctp.max_retrans_path;
	sp->pf_retrans  = net->sctp.pf_retrans;
	sp->ps_retrans  = net->sctp.ps_retrans;
	sp->pf_expose   = net->sctp.pf_expose;
	sp->pathmtu     = 0; /* allow default discovery */
	sp->sackdelay   = net->sctp.sack_timeout;
	sp->sackfreq	= 2;
	sp->param_flags = SPP_HB_ENABLE |
			  SPP_PMTUD_ENABLE |
			  SPP_SACKDELAY_ENABLE;
	sp->default_ss = SCTP_SS_DEFAULT;

	/* If enabled no SCTP message fragmentation will be performed.
	 * Configure through SCTP_DISABLE_FRAGMENTS socket option.
	 */
	sp->disable_fragments = 0;

	/* Enable Nagle algorithm by default.  */
	sp->nodelay           = 0;

	sp->recvrcvinfo = 0;
	sp->recvnxtinfo = 0;

	/* Enable by default. */
	sp->v4mapped          = 1;

	/* Auto-close idle associations after the configured
	 * number of seconds.  A value of 0 disables this
	 * feature.  Configure through the SCTP_AUTOCLOSE socket option,
	 * for UDP-style sockets only.
	 */
	sp->autoclose         = 0;

	/* User specified fragmentation limit. */
	sp->user_frag         = 0;

	sp->adaptation_ind = 0;

	sp->pf = sctp_get_pf_specific(sk->sk_family);

	/* Control variables for partial data delivery. */
	atomic_set(&sp->pd_mode, 0);
	skb_queue_head_init(&sp->pd_lobby);
	sp->frag_interleave = 0;

	/* Create a per socket endpoint structure.  Even if we
	 * change the data structure relationships, this may still
	 * be useful for storing pre-connect address information.
	 */
	sp->ep = sctp_endpoint_new(sk, GFP_KERNEL);
	if (!sp->ep)
		return -ENOMEM;

	sp->hmac = NULL;

	sk->sk_destruct = sctp_destruct_sock;

	SCTP_DBG_OBJCNT_INC(sock);

	local_bh_disable();
	sk_sockets_allocated_inc(sk);
	sock_prot_inuse_add(net, sk->sk_prot, 1);

	/* Nothing can fail after this block, otherwise
	 * sctp_destroy_sock() will be called without addr_wq_lock held
	 */
	if (net->sctp.default_auto_asconf) {
		spin_lock(&sock_net(sk)->sctp.addr_wq_lock);
		list_add_tail(&sp->auto_asconf_list,
		    &net->sctp.auto_asconf_splist);
		sp->do_auto_asconf = 1;
		spin_unlock(&sock_net(sk)->sctp.addr_wq_lock);
	} else {
		sp->do_auto_asconf = 0;
	}

	local_bh_enable();

	return 0;
}
	if (sp->do_auto_asconf) {
		sp->do_auto_asconf = 0;
		list_del(&sp->auto_asconf_list);
	}
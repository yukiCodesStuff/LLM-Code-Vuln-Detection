
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
	sk_common_release(sk);

	bh_unlock_sock(sk);
	spin_unlock_bh(&net->sctp.addr_wq_lock);

	sock_put(sk);

	SCTP_DBG_OBJCNT_DEC(sock);
	sk_sockets_allocated_inc(sk);
	sock_prot_inuse_add(net, sk->sk_prot, 1);

	/* Nothing can fail after this block, otherwise
	 * sctp_destroy_sock() will be called without addr_wq_lock held
	 */
	if (net->sctp.default_auto_asconf) {
		spin_lock(&sock_net(sk)->sctp.addr_wq_lock);
		list_add_tail(&sp->auto_asconf_list,
		    &net->sctp.auto_asconf_splist);

	if (sp->do_auto_asconf) {
		sp->do_auto_asconf = 0;
		list_del(&sp->auto_asconf_list);
	}
	sctp_endpoint_free(sp->ep);
	local_bh_disable();
	sk_sockets_allocated_dec(sk);
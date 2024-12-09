	struct sockaddr_llc sllc;
	struct sock *sk = sock->sk;
	struct llc_sock *llc = llc_sk(sk);
	int rc = -EBADF;

	memset(&sllc, 0, sizeof(sllc));
	lock_sock(sk);
	if (sock_flag(sk, SOCK_ZAPPED))
		goto out;
	*uaddrlen = sizeof(sllc);
	if (peer) {
		rc = -ENOTCONN;
		if (sk->sk_state != TCP_ESTABLISHED)
			goto out;
	rc = llc_proc_init();
	if (rc != 0) {
		printk(llc_proc_err_msg);
		goto out_station;
	}
	rc = llc_sysctl_init();
	if (rc) {
		printk(llc_sysctl_err_msg);
	llc_sysctl_exit();
out_proc:
	llc_proc_exit();
out_station:
	llc_station_exit();
	proto_unregister(&llc_proto);
	goto out;
}

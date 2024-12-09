{
	struct sockaddr_llc sllc;
	struct sock *sk = sock->sk;
	struct llc_sock *llc = llc_sk(sk);
	int rc = 0;

	memset(&sllc, 0, sizeof(sllc));
	lock_sock(sk);
	if (sock_flag(sk, SOCK_ZAPPED))
		goto out;
	*uaddrlen = sizeof(sllc);
	memset(uaddr, 0, *uaddrlen);
	if (peer) {
		rc = -ENOTCONN;
		if (sk->sk_state != TCP_ESTABLISHED)
			goto out;
		if(llc->dev)
			sllc.sllc_arphrd = llc->dev->type;
		sllc.sllc_sap = llc->daddr.lsap;
		memcpy(&sllc.sllc_mac, &llc->daddr.mac, IFHWADDRLEN);
	} else {
		rc = -EINVAL;
		if (!llc->sap)
			goto out;
		sllc.sllc_sap = llc->sap->laddr.lsap;

		if (llc->dev) {
			sllc.sllc_arphrd = llc->dev->type;
			memcpy(&sllc.sllc_mac, llc->dev->dev_addr,
			       IFHWADDRLEN);
		}
	}
	rc = 0;
	sllc.sllc_family = AF_LLC;
	memcpy(uaddr, &sllc, sizeof(sllc));
out:
	release_sock(sk);
	return rc;
}
	if (rc != 0) {
		printk(llc_proc_err_msg);
		goto out_unregister_llc_proto;
	}
	if (rc) {
		printk(llc_sock_err_msg);
		goto out_sysctl;
	}
	llc_add_pack(LLC_DEST_SAP, llc_sap_handler);
	llc_add_pack(LLC_DEST_CONN, llc_conn_handler);
out:
	return rc;
out_sysctl:
	llc_sysctl_exit();
out_proc:
	llc_proc_exit();
out_unregister_llc_proto:
	proto_unregister(&llc_proto);
	goto out;
}

static void __exit llc2_exit(void)
{
	llc_station_exit();
	llc_remove_pack(LLC_DEST_SAP);
	llc_remove_pack(LLC_DEST_CONN);
	sock_unregister(PF_LLC);
	llc_proc_exit();
	llc_sysctl_exit();
	proto_unregister(&llc_proto);
}

module_init(llc2_init);
module_exit(llc2_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Procom 1997, Jay Schullist 2001, Arnaldo C. Melo 2001-2003");
MODULE_DESCRIPTION("IEEE 802.2 PF_LLC support");
MODULE_ALIAS_NETPROTO(PF_LLC);
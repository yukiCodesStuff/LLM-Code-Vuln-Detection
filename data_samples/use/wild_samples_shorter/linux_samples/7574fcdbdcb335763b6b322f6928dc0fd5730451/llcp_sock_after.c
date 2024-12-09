
sock_unlink:
	nfc_llcp_sock_unlink(&local->connecting_sockets, sk);
	kfree(llcp_sock->service_name);
	llcp_sock->service_name = NULL;

sock_llcp_release:
	nfc_llcp_put_ssap(local, llcp_sock->ssap);
	nfc_llcp_local_put(llcp_sock->local);
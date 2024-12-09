
sock_unlink:
	nfc_llcp_sock_unlink(&local->connecting_sockets, sk);

sock_llcp_release:
	nfc_llcp_put_ssap(local, llcp_sock->ssap);
	nfc_llcp_local_put(llcp_sock->local);
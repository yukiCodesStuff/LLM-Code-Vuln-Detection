	llcp_sock->local = nfc_llcp_local_get(local);
	llcp_sock->ssap = nfc_llcp_get_local_ssap(local);
	if (llcp_sock->ssap == LLCP_SAP_MAX) {
		ret = -ENOMEM;
		goto put_dev;
	}


sock_llcp_release:
	nfc_llcp_put_ssap(local, llcp_sock->ssap);

put_dev:
	nfc_put_device(dev);
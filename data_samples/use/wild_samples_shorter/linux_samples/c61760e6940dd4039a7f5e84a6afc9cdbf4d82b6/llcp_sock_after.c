					  GFP_KERNEL);
	if (!llcp_sock->service_name) {
		nfc_llcp_local_put(llcp_sock->local);
		llcp_sock->local = NULL;
		ret = -ENOMEM;
		goto put_dev;
	}
	llcp_sock->ssap = nfc_llcp_get_sdp_ssap(local, llcp_sock);
	if (llcp_sock->ssap == LLCP_SAP_MAX) {
		nfc_llcp_local_put(llcp_sock->local);
		llcp_sock->local = NULL;
		kfree(llcp_sock->service_name);
		llcp_sock->service_name = NULL;
		ret = -EADDRINUSE;
		goto put_dev;
	llcp_sock->ssap = nfc_llcp_get_local_ssap(local);
	if (llcp_sock->ssap == LLCP_SAP_MAX) {
		nfc_llcp_local_put(llcp_sock->local);
		llcp_sock->local = NULL;
		ret = -ENOMEM;
		goto put_dev;
	}

sock_llcp_release:
	nfc_llcp_put_ssap(local, llcp_sock->ssap);
	nfc_llcp_local_put(llcp_sock->local);
	llcp_sock->local = NULL;

put_dev:
	nfc_put_device(dev);

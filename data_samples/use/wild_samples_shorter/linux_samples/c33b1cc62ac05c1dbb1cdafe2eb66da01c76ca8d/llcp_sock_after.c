					  llcp_sock->service_name_len,
					  GFP_KERNEL);
	if (!llcp_sock->service_name) {
		nfc_llcp_local_put(llcp_sock->local);
		ret = -ENOMEM;
		goto put_dev;
	}
	llcp_sock->ssap = nfc_llcp_get_sdp_ssap(local, llcp_sock);
	if (llcp_sock->ssap == LLCP_SAP_MAX) {
		nfc_llcp_local_put(llcp_sock->local);
		kfree(llcp_sock->service_name);
		llcp_sock->service_name = NULL;
		ret = -EADDRINUSE;
		goto put_dev;
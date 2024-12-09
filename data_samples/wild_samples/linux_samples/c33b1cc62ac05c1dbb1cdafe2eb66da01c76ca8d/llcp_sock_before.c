	if (local == NULL) {
		ret = -ENODEV;
		goto put_dev;
	}

	llcp_sock->dev = dev;
	llcp_sock->local = nfc_llcp_local_get(local);
	llcp_sock->nfc_protocol = llcp_addr.nfc_protocol;
	llcp_sock->service_name_len = min_t(unsigned int,
					    llcp_addr.service_name_len,
					    NFC_LLCP_MAX_SERVICE_NAME);
	llcp_sock->service_name = kmemdup(llcp_addr.service_name,
					  llcp_sock->service_name_len,
					  GFP_KERNEL);
	if (!llcp_sock->service_name) {
		ret = -ENOMEM;
		goto put_dev;
	}

	BT_DBG("hu %p", hu);

	if (!hci_uart_has_flow_control(hu))
		return -EOPNOTSUPP;

	intel = kzalloc(sizeof(*intel), GFP_KERNEL);
	if (!intel)
		return -ENOMEM;

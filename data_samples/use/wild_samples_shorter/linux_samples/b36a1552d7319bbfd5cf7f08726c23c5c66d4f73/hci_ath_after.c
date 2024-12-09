
	BT_DBG("hu %p", hu);

	if (!hci_uart_has_flow_control(hu))
		return -EOPNOTSUPP;

	ath = kzalloc(sizeof(*ath), GFP_KERNEL);
	if (!ath)
		return -ENOMEM;

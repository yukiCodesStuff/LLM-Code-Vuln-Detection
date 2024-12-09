
	BT_DBG("hu %p qca_open", hu);

	if (!hci_uart_has_flow_control(hu))
		return -EOPNOTSUPP;

	qca = kzalloc(sizeof(struct qca_data), GFP_KERNEL);
	if (!qca)
		return -ENOMEM;

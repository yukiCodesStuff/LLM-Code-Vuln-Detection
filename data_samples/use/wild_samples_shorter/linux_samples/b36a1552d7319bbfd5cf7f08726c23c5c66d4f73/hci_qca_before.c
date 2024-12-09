
	BT_DBG("hu %p qca_open", hu);

	qca = kzalloc(sizeof(struct qca_data), GFP_KERNEL);
	if (!qca)
		return -ENOMEM;

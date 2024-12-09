{
	struct qca_serdev *qcadev;
	struct qca_data *qca;
	int ret;

	BT_DBG("hu %p qca_open", hu);

	qca = kzalloc(sizeof(struct qca_data), GFP_KERNEL);
	if (!qca)
		return -ENOMEM;

	skb_queue_head_init(&qca->txq);
	skb_queue_head_init(&qca->tx_wait_q);
	spin_lock_init(&qca->hci_ibs_lock);
	qca->workqueue = alloc_ordered_workqueue("qca_wq", 0);
	if (!qca->workqueue) {
		BT_ERR("QCA Workqueue not initialized properly");
		kfree(qca);
		return -ENOMEM;
	}
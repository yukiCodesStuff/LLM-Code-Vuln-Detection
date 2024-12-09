{
	struct bcm_data *bcm;
	struct list_head *p;
	int err;

	bt_dev_dbg(hu->hdev, "hu %p", hu);

	if (!hci_uart_has_flow_control(hu))
		return -EOPNOTSUPP;

	bcm = kzalloc(sizeof(*bcm), GFP_KERNEL);
	if (!bcm)
		return -ENOMEM;

	skb_queue_head_init(&bcm->txq);

	hu->priv = bcm;

	mutex_lock(&bcm_device_lock);

	if (hu->serdev) {
		bcm->dev = serdev_device_get_drvdata(hu->serdev);
		goto out;
	}
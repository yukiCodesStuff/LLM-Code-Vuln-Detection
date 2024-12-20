{
	struct mrvl_data *mrvl;
	int ret;

	BT_DBG("hu %p", hu);

	mrvl = kzalloc(sizeof(*mrvl), GFP_KERNEL);
	if (!mrvl)
		return -ENOMEM;

	skb_queue_head_init(&mrvl->txq);
	skb_queue_head_init(&mrvl->rawq);

	set_bit(STATE_CHIP_VER_PENDING, &mrvl->flags);

	hu->priv = mrvl;

	if (hu->serdev) {
		ret = serdev_device_open(hu->serdev);
		if (ret)
			goto err;
	}
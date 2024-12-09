{
	struct ksdazzle_cb *kingsun = netdev_priv(netdev);
	int err = -ENOMEM;
	char hwname[16];

	/* At this point, urbs are NULL, and skb is NULL (see ksdazzle_probe) */
	kingsun->receiving = 0;

	/* Initialize for SIR to copy data directly into skb.  */
	kingsun->rx_unwrap_buff.in_frame = FALSE;
	kingsun->rx_unwrap_buff.state = OUTSIDE_FRAME;
	kingsun->rx_unwrap_buff.truesize = IRDA_SKB_MAX_MTU;
	kingsun->rx_unwrap_buff.skb = dev_alloc_skb(IRDA_SKB_MAX_MTU);
	if (!kingsun->rx_unwrap_buff.skb)
		goto free_mem;

	skb_reserve(kingsun->rx_unwrap_buff.skb, 1);
	kingsun->rx_unwrap_buff.head = kingsun->rx_unwrap_buff.skb->data;

	kingsun->rx_urb = usb_alloc_urb(0, GFP_KERNEL);
	if (!kingsun->rx_urb)
		goto free_mem;

	kingsun->tx_urb = usb_alloc_urb(0, GFP_KERNEL);
	if (!kingsun->tx_urb)
		goto free_mem;

	kingsun->speed_urb = usb_alloc_urb(0, GFP_KERNEL);
	if (!kingsun->speed_urb)
		goto free_mem;

	/* Initialize speed for dongle */
	kingsun->new_speed = 9600;
	err = ksdazzle_change_speed(kingsun, 9600);
	if (err < 0)
		goto free_mem;

	/*
	 * Now that everything should be initialized properly,
	 * Open new IrLAP layer instance to take care of us...
	 */
	sprintf(hwname, "usb#%d", kingsun->usbdev->devnum);
	kingsun->irlap = irlap_open(netdev, &kingsun->qos, hwname);
	if (!kingsun->irlap) {
		dev_err(&kingsun->usbdev->dev, "irlap_open failed\n");
		goto free_mem;
	}
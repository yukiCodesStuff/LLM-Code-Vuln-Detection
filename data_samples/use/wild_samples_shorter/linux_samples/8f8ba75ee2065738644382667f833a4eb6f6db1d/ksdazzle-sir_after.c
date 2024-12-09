	sprintf(hwname, "usb#%d", kingsun->usbdev->devnum);
	kingsun->irlap = irlap_open(netdev, &kingsun->qos, hwname);
	if (!kingsun->irlap) {
		err = -ENOMEM;
		dev_err(&kingsun->usbdev->dev, "irlap_open failed\n");
		goto free_mem;
	}


dealloc:
	if_usb_free(cardp);
	kfree(cardp);
error:
lbtf_deb_leave(LBTF_DEB_MAIN);
	return -ENOMEM;
}

	/* Unlink and free urb */
	if_usb_free(cardp);
	kfree(cardp);

	usb_set_intfdata(intf, NULL);
	usb_put_dev(interface_to_usbdev(intf));

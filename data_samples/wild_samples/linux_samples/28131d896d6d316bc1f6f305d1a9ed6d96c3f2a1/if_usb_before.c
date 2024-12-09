	if (!cardp->ep_out_buf) {
		lbtf_deb_usbd(&udev->dev, "Could not allocate buffer\n");
		goto dealloc;
	}

	cardp->boot2_version = udev->descriptor.bcdDevice;
	priv = lbtf_add_card(cardp, &udev->dev, &if_usb_ops);
	if (!priv)
		goto dealloc;

	usb_get_dev(udev);
	usb_set_intfdata(intf, cardp);

	return 0;

dealloc:
	if_usb_free(cardp);
error:
lbtf_deb_leave(LBTF_DEB_MAIN);
	return -ENOMEM;
}

/**
 *  if_usb_disconnect -  free resource and cleanup
 *
 *  @intf:	USB interface structure
 */
static void if_usb_disconnect(struct usb_interface *intf)
{
	struct if_usb_card *cardp = usb_get_intfdata(intf);
	struct lbtf_private *priv = cardp->priv;

	lbtf_deb_enter(LBTF_DEB_MAIN);

	if (priv) {
		if_usb_reset_device(priv);
		lbtf_remove_card(priv);
	}
	if (priv) {
		if_usb_reset_device(priv);
		lbtf_remove_card(priv);
	}

	/* Unlink and free urb */
	if_usb_free(cardp);

	usb_set_intfdata(intf, NULL);
	usb_put_dev(interface_to_usbdev(intf));

	lbtf_deb_leave(LBTF_DEB_MAIN);
}

/**
 *  if_usb_send_fw_pkt -  This function downloads the FW
 *
 *  @cardp:	pointer if_usb_card
 *
 *  Returns: 0
 */
static int if_usb_send_fw_pkt(struct if_usb_card *cardp)
{
	struct fwdata *fwdata = cardp->ep_out_buf;
	u8 *firmware = (u8 *) cardp->fw->data;

	lbtf_deb_enter(LBTF_DEB_FW);

	/* If we got a CRC failure on the last block, back
	   up and retry it */
	if (!cardp->CRC_OK) {
		cardp->totalbytes = cardp->fwlastblksent;
		cardp->fwseqnum--;
	}
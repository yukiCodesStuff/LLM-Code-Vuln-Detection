			    USB_CTRL_SET_TIMEOUT) != 0x11) {
		kfree(config_data);
		return -EIO;
	}

	/* check if we have a valid interface */
	if (if_num > 16) {
		kfree(config_data);
		return -EINVAL;
	}

	switch (config_data[if_num]) {
	case 0x0:
		result = 0;
		break;
	case 0x1:
		result = HSO_PORT_DIAG;
		break;
	case 0x2:
		result = HSO_PORT_GPS;
		break;
	case 0x3:
		result = HSO_PORT_GPS_CONTROL;
		break;
	case 0x4:
		result = HSO_PORT_APP;
		break;
	case 0x5:
		result = HSO_PORT_APP2;
		break;
	case 0x6:
		result = HSO_PORT_CONTROL;
		break;
	case 0x7:
		result = HSO_PORT_NETWORK;
		break;
	case 0x8:
		result = HSO_PORT_MODEM;
		break;
	case 0x9:
		result = HSO_PORT_MSD;
		break;
	case 0xa:
		result = HSO_PORT_PCSC;
		break;
	case 0xb:
		result = HSO_PORT_VOICE;
		break;
	default:
		result = 0;
	}

	if (result)
		result |= HSO_INTF_BULK;

	if (config_data[16] & 0x1)
		result |= HSO_INFO_CRC_BUG;

	kfree(config_data);
	return result;
}

/* called once for each interface upon device insertion */
static int hso_probe(struct usb_interface *interface,
		     const struct usb_device_id *id)
{
	int mux, i, if_num, port_spec;
	unsigned char port_mask;
	struct hso_device *hso_dev = NULL;
	struct hso_shared_int *shared_int;
	struct hso_device *tmp_dev = NULL;

	if (interface->cur_altsetting->desc.bInterfaceClass != 0xFF) {
		dev_err(&interface->dev, "Not our interface\n");
		return -ENODEV;
	}

	if_num = interface->cur_altsetting->desc.bInterfaceNumber;

	/* Get the interface/port specification from either driver_info or from
	 * the device itself */
	if (id->driver_info) {
		/* if_num is controlled by the device, driver_info is a 0 terminated
		 * array. Make sure, the access is in bounds! */
		for (i = 0; i <= if_num; ++i)
			if (((u32 *)(id->driver_info))[i] == 0)
				goto exit;
		port_spec = ((u32 *)(id->driver_info))[if_num];
	} else {
		port_spec = hso_get_config_data(interface);
		if (port_spec < 0)
			goto exit;
	}
	if (interface->cur_altsetting->desc.bInterfaceClass != 0xFF) {
		dev_err(&interface->dev, "Not our interface\n");
		return -ENODEV;
	}

	if_num = interface->cur_altsetting->desc.bInterfaceNumber;

	/* Get the interface/port specification from either driver_info or from
	 * the device itself */
	if (id->driver_info) {
		/* if_num is controlled by the device, driver_info is a 0 terminated
		 * array. Make sure, the access is in bounds! */
		for (i = 0; i <= if_num; ++i)
			if (((u32 *)(id->driver_info))[i] == 0)
				goto exit;
		port_spec = ((u32 *)(id->driver_info))[if_num];
	} else {
		port_spec = hso_get_config_data(interface);
		if (port_spec < 0)
			goto exit;
	}

	/* Check if we need to switch to alt interfaces prior to port
	 * configuration */
	if (interface->num_altsetting > 1)
		usb_set_interface(interface_to_usbdev(interface), if_num, 1);
	interface->needs_remote_wakeup = 1;

	/* Allocate new hso device(s) */
	switch (port_spec & HSO_INTF_MASK) {
	case HSO_INTF_MUX:
		if ((port_spec & HSO_PORT_MASK) == HSO_PORT_NETWORK) {
			/* Create the network device */
			if (!disable_net) {
				hso_dev = hso_create_net_device(interface,
								port_spec);
				if (!hso_dev)
					goto exit;
				tmp_dev = hso_dev;
			}
		}

		if (hso_get_mux_ports(interface, &port_mask))
			/* TODO: de-allocate everything */
			goto exit;

		shared_int = hso_create_shared_int(interface);
		if (!shared_int)
			goto exit;

		for (i = 1, mux = 0; i < 0x100; i = i << 1, mux++) {
			if (port_mask & i) {
				hso_dev = hso_create_mux_serial_device(
						interface, i, shared_int);
				if (!hso_dev)
					goto exit;
			}
		}

		if (tmp_dev)
			hso_dev = tmp_dev;
		break;

	case HSO_INTF_BULK:
		/* It's a regular bulk interface */
		if ((port_spec & HSO_PORT_MASK) == HSO_PORT_NETWORK) {
			if (!disable_net)
				hso_dev =
				    hso_create_net_device(interface, port_spec);
		} else {
			hso_dev =
			    hso_create_bulk_serial_device(interface, port_spec);
		}
		if (!hso_dev)
			goto exit;
		break;
	default:
		goto exit;
	}

	/* save our data pointer in this device */
	usb_set_intfdata(interface, hso_dev);

	/* done */
	return 0;
exit:
	hso_free_interface(interface);
	return -ENODEV;
}

/* device removed, cleaning up */
static void hso_disconnect(struct usb_interface *interface)
{
	hso_free_interface(interface);

	/* remove reference of our private data */
	usb_set_intfdata(interface, NULL);
}

static void async_get_intf(struct work_struct *data)
{
	struct hso_device *hso_dev =
	    container_of(data, struct hso_device, async_get_intf);
	usb_autopm_get_interface(hso_dev->interface);
}

static void async_put_intf(struct work_struct *data)
{
	struct hso_device *hso_dev =
	    container_of(data, struct hso_device, async_put_intf);
	usb_autopm_put_interface(hso_dev->interface);
}

static int hso_get_activity(struct hso_device *hso_dev)
{
	if (hso_dev->usb->state == USB_STATE_SUSPENDED) {
		if (!hso_dev->is_active) {
			hso_dev->is_active = 1;
			schedule_work(&hso_dev->async_get_intf);
		}
	}

	if (hso_dev->usb->state != USB_STATE_CONFIGURED)
		return -EAGAIN;

	usb_mark_last_busy(hso_dev->usb);

	return 0;
}

static int hso_put_activity(struct hso_device *hso_dev)
{
	if (hso_dev->usb->state != USB_STATE_SUSPENDED) {
		if (hso_dev->is_active) {
			hso_dev->is_active = 0;
			schedule_work(&hso_dev->async_put_intf);
			return -EAGAIN;
		}
	}
	hso_dev->is_active = 0;
	return 0;
}

/* called by kernel when we need to suspend device */
static int hso_suspend(struct usb_interface *iface, pm_message_t message)
{
	int i, result;

	/* Stop all serial ports */
	for (i = 0; i < HSO_SERIAL_TTY_MINORS; i++) {
		if (serial_table[i] && (serial_table[i]->interface == iface)) {
			result = hso_stop_serial_device(serial_table[i]);
			if (result)
				goto out;
		}
	}

	/* Stop all network ports */
	for (i = 0; i < HSO_MAX_NET_DEVICES; i++) {
		if (network_table[i] &&
		    (network_table[i]->interface == iface)) {
			result = hso_stop_net_device(network_table[i]);
			if (result)
				goto out;
		}
	}

out:
	return 0;
}

/* called by kernel when we need to resume device */
static int hso_resume(struct usb_interface *iface)
{
	int i, result = 0;
	struct hso_net *hso_net;

	/* Start all serial ports */
	for (i = 0; i < HSO_SERIAL_TTY_MINORS; i++) {
		if (serial_table[i] && (serial_table[i]->interface == iface)) {
			if (dev2ser(serial_table[i])->port.count) {
				result =
				    hso_start_serial_device(serial_table[i], GFP_NOIO);
				hso_kick_transmit(dev2ser(serial_table[i]));
				if (result)
					goto out;
			}
		}
	}

	/* Start all network ports */
	for (i = 0; i < HSO_MAX_NET_DEVICES; i++) {
		if (network_table[i] &&
		    (network_table[i]->interface == iface)) {
			hso_net = dev2net(network_table[i]);
			if (hso_net->flags & IFF_UP) {
				/* First transmit any lingering data,
				   then restart the device. */
				if (hso_net->skb_tx_buf) {
					dev_dbg(&iface->dev,
						"Transmitting"
						" lingering data\n");
					hso_net_start_xmit(hso_net->skb_tx_buf,
							   hso_net->net);
					hso_net->skb_tx_buf = NULL;
				}
				result = hso_start_net_device(network_table[i]);
				if (result)
					goto out;
			}
		}
	}

out:
	return result;
}

static void hso_serial_ref_free(struct kref *ref)
{
	struct hso_device *hso_dev = container_of(ref, struct hso_device, ref);

	hso_free_serial_device(hso_dev);
}

static void hso_free_interface(struct usb_interface *interface)
{
	struct hso_serial *serial;
	int i;

	for (i = 0; i < HSO_SERIAL_TTY_MINORS; i++) {
		if (serial_table[i] &&
		    (serial_table[i]->interface == interface)) {
			serial = dev2ser(serial_table[i]);
			tty_port_tty_hangup(&serial->port, false);
			mutex_lock(&serial->parent->mutex);
			serial->parent->usb_gone = 1;
			mutex_unlock(&serial->parent->mutex);
			cancel_work_sync(&serial_table[i]->async_put_intf);
			cancel_work_sync(&serial_table[i]->async_get_intf);
			hso_serial_tty_unregister(serial);
			kref_put(&serial_table[i]->ref, hso_serial_ref_free);
			set_serial_by_index(i, NULL);
		}
	}

	for (i = 0; i < HSO_MAX_NET_DEVICES; i++) {
		if (network_table[i] &&
		    (network_table[i]->interface == interface)) {
			struct rfkill *rfk = dev2net(network_table[i])->rfkill;
			/* hso_stop_net_device doesn't stop the net queue since
			 * traffic needs to start it again when suspended */
			netif_stop_queue(dev2net(network_table[i])->net);
			hso_stop_net_device(network_table[i]);
			cancel_work_sync(&network_table[i]->async_put_intf);
			cancel_work_sync(&network_table[i]->async_get_intf);
			if (rfk) {
				rfkill_unregister(rfk);
				rfkill_destroy(rfk);
			}
			hso_free_net_device(network_table[i]);
		}
	}
}

/* Helper functions */

/* Get the endpoint ! */
static struct usb_endpoint_descriptor *hso_get_ep(struct usb_interface *intf,
						  int type, int dir)
{
	int i;
	struct usb_host_interface *iface = intf->cur_altsetting;
	struct usb_endpoint_descriptor *endp;

	for (i = 0; i < iface->desc.bNumEndpoints; i++) {
		endp = &iface->endpoint[i].desc;
		if (((endp->bEndpointAddress & USB_ENDPOINT_DIR_MASK) == dir) &&
		    (usb_endpoint_type(endp) == type))
			return endp;
	}

	return NULL;
}

/* Get the byte that describes which ports are enabled */
static int hso_get_mux_ports(struct usb_interface *intf, unsigned char *ports)
{
	int i;
	struct usb_host_interface *iface = intf->cur_altsetting;

	if (iface->extralen == 3) {
		*ports = iface->extra[2];
		return 0;
	}

	for (i = 0; i < iface->desc.bNumEndpoints; i++) {
		if (iface->endpoint[i].extralen == 3) {
			*ports = iface->endpoint[i].extra[2];
			return 0;
		}
	}

	return -1;
}

/* interrupt urb needs to be submitted, used for serial read of muxed port */
static int hso_mux_submit_intr_urb(struct hso_shared_int *shared_int,
				   struct usb_device *usb, gfp_t gfp)
{
	int result;

	usb_fill_int_urb(shared_int->shared_intr_urb, usb,
			 usb_rcvintpipe(usb,
				shared_int->intr_endp->bEndpointAddress & 0x7F),
			 shared_int->shared_intr_buf,
			 1,
			 intr_callback, shared_int,
			 shared_int->intr_endp->bInterval);

	result = usb_submit_urb(shared_int->shared_intr_urb, gfp);
	if (result)
		dev_warn(&usb->dev, "%s failed mux_intr_urb %d\n", __func__,
			result);

	return result;
}

/* operations setup of the serial interface */
static const struct tty_operations hso_serial_ops = {
	.open = hso_serial_open,
	.close = hso_serial_close,
	.write = hso_serial_write,
	.write_room = hso_serial_write_room,
	.cleanup = hso_serial_cleanup,
	.ioctl = hso_serial_ioctl,
	.set_termios = hso_serial_set_termios,
	.chars_in_buffer = hso_serial_chars_in_buffer,
	.tiocmget = hso_serial_tiocmget,
	.tiocmset = hso_serial_tiocmset,
	.get_icount = hso_get_count,
	.unthrottle = hso_unthrottle
};

static struct usb_driver hso_driver = {
	.name = driver_name,
	.probe = hso_probe,
	.disconnect = hso_disconnect,
	.id_table = hso_ids,
	.suspend = hso_suspend,
	.resume = hso_resume,
	.reset_resume = hso_resume,
	.supports_autosuspend = 1,
	.disable_hub_initiated_lpm = 1,
};

static int __init hso_init(void)
{
	int i;
	int result;

	/* put it in the log */
	pr_info("%s\n", version);

	/* Initialise the serial table semaphore and table */
	spin_lock_init(&serial_table_lock);
	for (i = 0; i < HSO_SERIAL_TTY_MINORS; i++)
		serial_table[i] = NULL;

	/* allocate our driver using the proper amount of supported minors */
	tty_drv = alloc_tty_driver(HSO_SERIAL_TTY_MINORS);
	if (!tty_drv)
		return -ENOMEM;

	/* fill in all needed values */
	tty_drv->driver_name = driver_name;
	tty_drv->name = tty_filename;

	/* if major number is provided as parameter, use that one */
	if (tty_major)
		tty_drv->major = tty_major;

	tty_drv->minor_start = 0;
	tty_drv->type = TTY_DRIVER_TYPE_SERIAL;
	tty_drv->subtype = SERIAL_TYPE_NORMAL;
	tty_drv->flags = TTY_DRIVER_REAL_RAW | TTY_DRIVER_DYNAMIC_DEV;
	tty_drv->init_termios = tty_std_termios;
	hso_init_termios(&tty_drv->init_termios);
	tty_set_operations(tty_drv, &hso_serial_ops);

	/* register the tty driver */
	result = tty_register_driver(tty_drv);
	if (result) {
		pr_err("%s - tty_register_driver failed(%d)\n",
		       __func__, result);
		goto err_free_tty;
	}

	/* register this module as an usb driver */
	result = usb_register(&hso_driver);
	if (result) {
		pr_err("Could not register hso driver - error: %d\n", result);
		goto err_unreg_tty;
	}

	/* done */
	return 0;
err_unreg_tty:
	tty_unregister_driver(tty_drv);
err_free_tty:
	put_tty_driver(tty_drv);
	return result;
}

static void __exit hso_exit(void)
{
	pr_info("unloaded\n");

	tty_unregister_driver(tty_drv);
	/* deregister the usb driver */
	usb_deregister(&hso_driver);
	put_tty_driver(tty_drv);
}

/* Module definitions */
module_init(hso_init);
module_exit(hso_exit);

MODULE_AUTHOR(MOD_AUTHOR);
MODULE_DESCRIPTION(MOD_DESCRIPTION);
MODULE_LICENSE("GPL");

/* change the debug level (eg: insmod hso.ko debug=0x04) */
MODULE_PARM_DESC(debug, "debug level mask [0x01 | 0x02 | 0x04 | 0x08 | 0x10]");
module_param(debug, int, 0644);

/* set the major tty number (eg: insmod hso.ko tty_major=245) */
MODULE_PARM_DESC(tty_major, "Set the major tty number");
module_param(tty_major, int, 0644);

/* disable network interface (eg: insmod hso.ko disable_net=1) */
MODULE_PARM_DESC(disable_net, "Disable the network interface");
module_param(disable_net, int, 0644);
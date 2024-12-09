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
		if (((port_spec & HSO_PORT_MASK) == HSO_PORT_NETWORK) &&
		    !disable_net)
			hso_dev = hso_create_net_device(interface, port_spec);
		else
			hso_dev =
			    hso_create_bulk_serial_device(interface, port_spec);
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
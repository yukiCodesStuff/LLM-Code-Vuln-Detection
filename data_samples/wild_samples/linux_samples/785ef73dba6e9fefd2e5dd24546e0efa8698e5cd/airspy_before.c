	if (ret) {
		dev_err(s->dev, "Failed to register as video device (%d)\n",
				ret);
		goto err_unregister_v4l2_dev;
	}
	dev_info(s->dev, "Registered as %s\n",
			video_device_node_name(&s->vdev));
	dev_notice(s->dev, "SDR API is still slightly experimental and functionality changes may follow\n");
	return 0;

err_free_controls:
	v4l2_ctrl_handler_free(&s->hdl);
err_unregister_v4l2_dev:
	v4l2_device_unregister(&s->v4l2_dev);
err_free_mem:
	kfree(s);
	return ret;
}

/* USB device ID list */
static struct usb_device_id airspy_id_table[] = {
	{ USB_DEVICE(0x1d50, 0x60a1) }, /* AirSpy */
	{ }
};
MODULE_DEVICE_TABLE(usb, airspy_id_table);

/* USB subsystem interface */
static struct usb_driver airspy_driver = {
	.name                     = KBUILD_MODNAME,
	.probe                    = airspy_probe,
	.disconnect               = airspy_disconnect,
	.id_table                 = airspy_id_table,
};

module_usb_driver(airspy_driver);

MODULE_AUTHOR("Antti Palosaari <crope@iki.fi>");
MODULE_DESCRIPTION("AirSpy SDR");
MODULE_LICENSE("GPL");
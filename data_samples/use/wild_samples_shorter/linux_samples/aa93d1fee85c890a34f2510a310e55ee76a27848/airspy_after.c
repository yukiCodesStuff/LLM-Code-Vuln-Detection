	if (ret) {
		dev_err(s->dev, "Failed to register as video device (%d)\n",
				ret);
		goto err_free_controls;
	}
	dev_info(s->dev, "Registered as %s\n",
			video_device_node_name(&s->vdev));
	dev_notice(s->dev, "SDR API is still slightly experimental and functionality changes may follow\n");

err_free_controls:
	v4l2_ctrl_handler_free(&s->hdl);
	v4l2_device_unregister(&s->v4l2_dev);
err_free_mem:
	kfree(s);
	return ret;
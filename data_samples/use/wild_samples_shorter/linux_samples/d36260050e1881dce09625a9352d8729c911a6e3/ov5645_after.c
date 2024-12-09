	}
}

static int ov5645_set_format(struct v4l2_subdev *sd,
			     struct v4l2_subdev_pad_config *cfg,
			     struct v4l2_subdev_format *format)
{
	__crop = __ov5645_get_pad_crop(ov5645, cfg, format->pad,
			format->which);

	new_mode = v4l2_find_nearest_size(ov5645_mode_info_data,
			       ARRAY_SIZE(ov5645_mode_info_data),
			       width, height,
			       format->format.width, format->format.height);

	__crop->width = new_mode->width;
	__crop->height = new_mode->height;

	if (format->which == V4L2_SUBDEV_FORMAT_ACTIVE) {

	ret = v4l2_fwnode_endpoint_parse(of_fwnode_handle(endpoint),
					 &ov5645->ep);

	of_node_put(endpoint);

	if (ret < 0) {
		dev_err(dev, "parsing endpoint node failed\n");
		return ret;
	}

	if (ov5645->ep.bus_type != V4L2_MBUS_CSI2) {
		dev_err(dev, "invalid bus type, must be CSI2\n");
		return -EINVAL;
	}
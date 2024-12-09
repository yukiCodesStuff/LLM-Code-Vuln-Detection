	}
}

static const struct ov5645_mode_info *
ov5645_find_nearest_mode(unsigned int width, unsigned int height)
{
	int i;

	for (i = ARRAY_SIZE(ov5645_mode_info_data) - 1; i >= 0; i--) {
		if (ov5645_mode_info_data[i].width <= width &&
		    ov5645_mode_info_data[i].height <= height)
			break;
	}

	if (i < 0)
		i = 0;

	return &ov5645_mode_info_data[i];
}

static int ov5645_set_format(struct v4l2_subdev *sd,
			     struct v4l2_subdev_pad_config *cfg,
			     struct v4l2_subdev_format *format)
{
	__crop = __ov5645_get_pad_crop(ov5645, cfg, format->pad,
			format->which);

	new_mode = ov5645_find_nearest_mode(format->format.width,
					    format->format.height);
	__crop->width = new_mode->width;
	__crop->height = new_mode->height;

	if (format->which == V4L2_SUBDEV_FORMAT_ACTIVE) {

	ret = v4l2_fwnode_endpoint_parse(of_fwnode_handle(endpoint),
					 &ov5645->ep);
	if (ret < 0) {
		dev_err(dev, "parsing endpoint node failed\n");
		return ret;
	}

	of_node_put(endpoint);

	if (ov5645->ep.bus_type != V4L2_MBUS_CSI2) {
		dev_err(dev, "invalid bus type, must be CSI2\n");
		return -EINVAL;
	}
	return 0;
}

/* restore the last set video mode after chip power-on */
static int ov5640_restore_mode(struct ov5640_dev *sensor)
{
	int ret;
		return ret;

	/* now restore the last capture mode */
	return ov5640_set_mode(sensor, &ov5640_mode_init_data);
}

static void ov5640_power(struct ov5640_dev *sensor, bool enable)
{
		if (ov5640_formats[i].code == fmt->code)
			break;
	if (i >= ARRAY_SIZE(ov5640_formats))
		fmt->code = ov5640_formats[0].code;

	return 0;
}

{
	struct ov5640_dev *sensor = to_ov5640_dev(sd);
	const struct ov5640_mode_info *new_mode;
	int ret;

	if (format->pad != 0)
		return -EINVAL;
		goto out;
	}

	ret = ov5640_try_fmt_internal(sd, &format->format,
				      sensor->current_fr, &new_mode);
	if (ret)
		goto out;

		struct v4l2_mbus_framefmt *fmt =
			v4l2_subdev_get_try_format(sd, cfg, 0);

		*fmt = format->format;
		goto out;
	}

	sensor->current_mode = new_mode;
	sensor->fmt = format->format;
	sensor->pending_mode_change = true;
out:
	mutex_unlock(&sensor->lock);
	return ret;
	struct device *dev = &client->dev;
	struct fwnode_handle *endpoint;
	struct ov5640_dev *sensor;
	int ret;

	sensor = devm_kzalloc(dev, sizeof(*sensor), GFP_KERNEL);
	if (!sensor)
		return -ENOMEM;

	sensor->i2c_client = client;
	sensor->fmt.code = MEDIA_BUS_FMT_UYVY8_2X8;
	sensor->fmt.width = 640;
	sensor->fmt.height = 480;
	sensor->fmt.field = V4L2_FIELD_NONE;
	sensor->frame_interval.numerator = 1;
	sensor->frame_interval.denominator = ov5640_framerates[OV5640_30_FPS];
	sensor->current_fr = OV5640_30_FPS;
	sensor->current_mode =
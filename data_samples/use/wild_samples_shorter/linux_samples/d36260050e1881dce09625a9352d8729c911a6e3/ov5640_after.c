	return 0;
}

static int ov5640_set_framefmt(struct ov5640_dev *sensor,
			       struct v4l2_mbus_framefmt *format);

/* restore the last set video mode after chip power-on */
static int ov5640_restore_mode(struct ov5640_dev *sensor)
{
	int ret;
		return ret;

	/* now restore the last capture mode */
	ret = ov5640_set_mode(sensor, &ov5640_mode_init_data);
	if (ret < 0)
		return ret;

	return ov5640_set_framefmt(sensor, &sensor->fmt);
}

static void ov5640_power(struct ov5640_dev *sensor, bool enable)
{
		if (ov5640_formats[i].code == fmt->code)
			break;
	if (i >= ARRAY_SIZE(ov5640_formats))
		i = 0;

	fmt->code = ov5640_formats[i].code;
	fmt->colorspace = ov5640_formats[i].colorspace;
	fmt->ycbcr_enc = V4L2_MAP_YCBCR_ENC_DEFAULT(fmt->colorspace);
	fmt->quantization = V4L2_QUANTIZATION_FULL_RANGE;
	fmt->xfer_func = V4L2_MAP_XFER_FUNC_DEFAULT(fmt->colorspace);

	return 0;
}

{
	struct ov5640_dev *sensor = to_ov5640_dev(sd);
	const struct ov5640_mode_info *new_mode;
	struct v4l2_mbus_framefmt *mbus_fmt = &format->format;
	int ret;

	if (format->pad != 0)
		return -EINVAL;
		goto out;
	}

	ret = ov5640_try_fmt_internal(sd, mbus_fmt,
				      sensor->current_fr, &new_mode);
	if (ret)
		goto out;

		struct v4l2_mbus_framefmt *fmt =
			v4l2_subdev_get_try_format(sd, cfg, 0);

		*fmt = *mbus_fmt;
		goto out;
	}

	sensor->current_mode = new_mode;
	sensor->fmt = *mbus_fmt;
	sensor->pending_mode_change = true;
out:
	mutex_unlock(&sensor->lock);
	return ret;
	struct device *dev = &client->dev;
	struct fwnode_handle *endpoint;
	struct ov5640_dev *sensor;
	struct v4l2_mbus_framefmt *fmt;
	int ret;

	sensor = devm_kzalloc(dev, sizeof(*sensor), GFP_KERNEL);
	if (!sensor)
		return -ENOMEM;

	sensor->i2c_client = client;
	fmt = &sensor->fmt;
	fmt->code = ov5640_formats[0].code;
	fmt->colorspace = ov5640_formats[0].colorspace;
	fmt->ycbcr_enc = V4L2_MAP_YCBCR_ENC_DEFAULT(fmt->colorspace);
	fmt->quantization = V4L2_QUANTIZATION_FULL_RANGE;
	fmt->xfer_func = V4L2_MAP_XFER_FUNC_DEFAULT(fmt->colorspace);
	fmt->width = 640;
	fmt->height = 480;
	fmt->field = V4L2_FIELD_NONE;
	sensor->frame_interval.numerator = 1;
	sensor->frame_interval.denominator = ov5640_framerates[OV5640_30_FPS];
	sensor->current_fr = OV5640_30_FPS;
	sensor->current_mode =
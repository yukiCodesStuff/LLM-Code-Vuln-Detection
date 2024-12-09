static const struct v4l2_ctrl_ops ctrl_ops = {
	.s_ctrl = gc0310_s_ctrl,
	.g_volatile_ctrl = gc0310_g_volatile_ctrl
};

struct v4l2_ctrl_config gc0310_controls[] = {
	{
	 .ops = &ctrl_ops,
	 .id = V4L2_CID_EXPOSURE_ABSOLUTE,
	 .type = V4L2_CTRL_TYPE_INTEGER,
	 .name = "exposure",
	 .min = 0x0,
	 .max = 0xffff,
	 .step = 0x01,
	 .def = 0x00,
	 .flags = 0,
	 },
	{
	 .ops = &ctrl_ops,
	 .id = V4L2_CID_VFLIP,
	 .type = V4L2_CTRL_TYPE_BOOLEAN,
	 .name = "Flip",
	 .min = 0,
	 .max = 1,
	 .step = 1,
	 .def = 0,
	 },
	{
	 .ops = &ctrl_ops,
	 .id = V4L2_CID_HFLIP,
	 .type = V4L2_CTRL_TYPE_BOOLEAN,
	 .name = "Mirror",
	 .min = 0,
	 .max = 1,
	 .step = 1,
	 .def = 0,
	 },
	{
	 .ops = &ctrl_ops,
	 .id = V4L2_CID_FOCAL_ABSOLUTE,
	 .type = V4L2_CTRL_TYPE_INTEGER,
	 .name = "focal length",
	 .min = GC0310_FOCAL_LENGTH_DEFAULT,
	 .max = GC0310_FOCAL_LENGTH_DEFAULT,
	 .step = 0x01,
	 .def = GC0310_FOCAL_LENGTH_DEFAULT,
	 .flags = 0,
	 },
	{
	 .ops = &ctrl_ops,
	 .id = V4L2_CID_FNUMBER_ABSOLUTE,
	 .type = V4L2_CTRL_TYPE_INTEGER,
	 .name = "f-number",
	 .min = GC0310_F_NUMBER_DEFAULT,
	 .max = GC0310_F_NUMBER_DEFAULT,
	 .step = 0x01,
	 .def = GC0310_F_NUMBER_DEFAULT,
	 .flags = 0,
	 },
	{
	 .ops = &ctrl_ops,
	 .id = V4L2_CID_FNUMBER_RANGE,
	 .type = V4L2_CTRL_TYPE_INTEGER,
	 .name = "f-number range",
	 .min = GC0310_F_NUMBER_RANGE,
	 .max = GC0310_F_NUMBER_RANGE,
	 .step = 0x01,
	 .def = GC0310_F_NUMBER_RANGE,
	 .flags = 0,
	 },
	{
	 .ops = &ctrl_ops,
	 .id = V4L2_CID_BIN_FACTOR_HORZ,
	 .type = V4L2_CTRL_TYPE_INTEGER,
	 .name = "horizontal binning factor",
	 .min = 0,
	 .max = GC0310_BIN_FACTOR_MAX,
	 .step = 1,
	 .def = 0,
	 .flags = 0,
	 },
	{
	 .ops = &ctrl_ops,
	 .id = V4L2_CID_BIN_FACTOR_VERT,
	 .type = V4L2_CTRL_TYPE_INTEGER,
	 .name = "vertical binning factor",
	 .min = 0,
	 .max = GC0310_BIN_FACTOR_MAX,
	 .step = 1,
	 .def = 0,
	 .flags = 0,
	 },
};

static int gc0310_init(struct v4l2_subdev *sd)
{
	int ret;
	struct i2c_client *client = v4l2_get_subdevdata(sd);
	struct gc0310_device *dev = to_gc0310_sensor(sd);

	pr_info("%s S\n", __func__);
	mutex_lock(&dev->input_lock);

	/* set inital registers */
	ret  = gc0310_write_reg_array(client, gc0310_reset_register);

	/* restore settings */
	gc0310_res = gc0310_res_preview;
	N_RES = N_RES_PREVIEW;

	mutex_unlock(&dev->input_lock);

	pr_info("%s E\n", __func__);
	return 0;
}

static int power_ctrl(struct v4l2_subdev *sd, bool flag)
{
	int ret = 0;
	struct gc0310_device *dev = to_gc0310_sensor(sd);
	if (!dev || !dev->platform_data)
		return -ENODEV;

	if (flag) {
		/* The upstream module driver (written to Crystal
		 * Cove) had this logic to pulse the rails low first.
		 * This appears to break things on the MRD7 with the
		 * X-Powers PMIC...
		 *
		 *     ret = dev->platform_data->v1p8_ctrl(sd, 0);
		 *     ret |= dev->platform_data->v2p8_ctrl(sd, 0);
		 *     mdelay(50);
		 */
		ret |= dev->platform_data->v1p8_ctrl(sd, 1);
		ret |= dev->platform_data->v2p8_ctrl(sd, 1);
		usleep_range(10000, 15000);
	}

	if (!flag || ret) {
		ret |= dev->platform_data->v1p8_ctrl(sd, 0);
		ret |= dev->platform_data->v2p8_ctrl(sd, 0);
	}
	return ret;
}

static int gpio_ctrl(struct v4l2_subdev *sd, bool flag)
{
	int ret;
	struct gc0310_device *dev = to_gc0310_sensor(sd);

	if (!dev || !dev->platform_data)
		return -ENODEV;

	/* GPIO0 == "reset" (active low), GPIO1 == "power down" */
	if (flag) {
		/* Pulse reset, then release power down */
		ret = dev->platform_data->gpio0_ctrl(sd, 0);
		usleep_range(5000, 10000);
		ret |= dev->platform_data->gpio0_ctrl(sd, 1);
		usleep_range(10000, 15000);
		ret |= dev->platform_data->gpio1_ctrl(sd, 0);
		usleep_range(10000, 15000);
	} else {
		ret = dev->platform_data->gpio1_ctrl(sd, 1);
		ret |= dev->platform_data->gpio0_ctrl(sd, 0);
	}
	return ret;
}


static int power_down(struct v4l2_subdev *sd);

static int power_up(struct v4l2_subdev *sd)
{
	struct gc0310_device *dev = to_gc0310_sensor(sd);
	struct i2c_client *client = v4l2_get_subdevdata(sd);
	int ret;

	pr_info("%s S\n", __func__);
	if (!dev->platform_data) {
		dev_err(&client->dev,
			"no camera_sensor_platform_data");
		return -ENODEV;
	}

	/* power control */
	ret = power_ctrl(sd, 1);
	if (ret)
		goto fail_power;

	/* flis clock control */
	ret = dev->platform_data->flisclk_ctrl(sd, 1);
	if (ret)
		goto fail_clk;

	/* gpio ctrl */
	ret = gpio_ctrl(sd, 1);
	if (ret) {
		ret = gpio_ctrl(sd, 1);
		if (ret)
			goto fail_gpio;
	}

	msleep(100);

	pr_info("%s E\n", __func__);
	return 0;

fail_gpio:
	dev->platform_data->flisclk_ctrl(sd, 0);
fail_clk:
	power_ctrl(sd, 0);
fail_power:
	dev_err(&client->dev, "sensor power-up failed\n");

	return ret;
}

static int power_down(struct v4l2_subdev *sd)
{
	struct gc0310_device *dev = to_gc0310_sensor(sd);
	struct i2c_client *client = v4l2_get_subdevdata(sd);
	int ret = 0;

	if (!dev->platform_data) {
		dev_err(&client->dev,
			"no camera_sensor_platform_data");
		return -ENODEV;
	}

	/* gpio ctrl */
	ret = gpio_ctrl(sd, 0);
	if (ret) {
		ret = gpio_ctrl(sd, 0);
		if (ret)
			dev_err(&client->dev, "gpio failed 2\n");
	}

	ret = dev->platform_data->flisclk_ctrl(sd, 0);
	if (ret)
		dev_err(&client->dev, "flisclk failed\n");

	/* power control */
	ret = power_ctrl(sd, 0);
	if (ret)
		dev_err(&client->dev, "vprog failed.\n");

	return ret;
}

static int gc0310_s_power(struct v4l2_subdev *sd, int on)
{
	int ret;
	if (on == 0)
		return power_down(sd);
	else {
		ret = power_up(sd);
		if (!ret)
			return gc0310_init(sd);
	}
	return ret;
}

/*
 * distance - calculate the distance
 * @res: resolution
 * @w: width
 * @h: height
 *
 * Get the gap between resolution and w/h.
 * res->width/height smaller than w/h wouldn't be considered.
 * Returns the value of gap or -1 if fail.
 */
#define LARGEST_ALLOWED_RATIO_MISMATCH 800
static int distance(struct gc0310_resolution *res, u32 w, u32 h)
{
	unsigned int w_ratio = (res->width << 13) / w;
	unsigned int h_ratio;
	int match;

	if (h == 0)
		return -1;
	h_ratio = (res->height << 13) / h;
	if (h_ratio == 0)
		return -1;
	match   = abs(((w_ratio << 13) / h_ratio) - ((int)8192));

	if ((w_ratio < (int)8192) || (h_ratio < (int)8192)  ||
		(match > LARGEST_ALLOWED_RATIO_MISMATCH))
		return -1;

	return w_ratio + h_ratio;
}

/* Return the nearest higher resolution index */
static int nearest_resolution_index(int w, int h)
{
	int i;
	int idx = -1;
	int dist;
	int min_dist = INT_MAX;
	struct gc0310_resolution *tmp_res = NULL;

	for (i = 0; i < N_RES; i++) {
		tmp_res = &gc0310_res[i];
		dist = distance(tmp_res, w, h);
		if (dist == -1)
			continue;
		if (dist < min_dist) {
			min_dist = dist;
			idx = i;
		}
	}

	return idx;
}

static int get_resolution_index(int w, int h)
{
	int i;

	for (i = 0; i < N_RES; i++) {
		if (w != gc0310_res[i].width)
			continue;
		if (h != gc0310_res[i].height)
			continue;

		return i;
	}

	return -1;
}


/* TODO: remove it. */
static int startup(struct v4l2_subdev *sd)
{
	struct gc0310_device *dev = to_gc0310_sensor(sd);
	struct i2c_client *client = v4l2_get_subdevdata(sd);
	int ret = 0;

	pr_info("%s S\n", __func__);

	ret = gc0310_write_reg_array(client, gc0310_res[dev->fmt_idx].regs);
	if (ret) {
		dev_err(&client->dev, "gc0310 write register err.\n");
		return ret;
	}

	pr_info("%s E\n", __func__);
	return ret;
}

static int gc0310_set_fmt(struct v4l2_subdev *sd,
			  struct v4l2_subdev_pad_config *cfg,
			  struct v4l2_subdev_format *format)
{
	struct v4l2_mbus_framefmt *fmt = &format->format;
	struct gc0310_device *dev = to_gc0310_sensor(sd);
	struct i2c_client *client = v4l2_get_subdevdata(sd);
	struct camera_mipi_info *gc0310_info = NULL;
	int ret = 0;
	int idx = 0;
	pr_info("%s S\n", __func__);

	if (format->pad)
		return -EINVAL;

	if (!fmt)
		return -EINVAL;

	gc0310_info = v4l2_get_subdev_hostdata(sd);
	if (!gc0310_info)
		return -EINVAL;

	mutex_lock(&dev->input_lock);

	idx = nearest_resolution_index(fmt->width, fmt->height);
	if (idx == -1) {
		/* return the largest resolution */
		fmt->width = gc0310_res[N_RES - 1].width;
		fmt->height = gc0310_res[N_RES - 1].height;
	} else {
		fmt->width = gc0310_res[idx].width;
		fmt->height = gc0310_res[idx].height;
	}
	fmt->code = MEDIA_BUS_FMT_SGRBG8_1X8;

	if (format->which == V4L2_SUBDEV_FORMAT_TRY) {
		cfg->try_fmt = *fmt;
		mutex_unlock(&dev->input_lock);
		return 0;
	}

	dev->fmt_idx = get_resolution_index(fmt->width, fmt->height);
	if (dev->fmt_idx == -1) {
		dev_err(&client->dev, "get resolution fail\n");
		mutex_unlock(&dev->input_lock);
		return -EINVAL;
	}

	printk("%s: before gc0310_write_reg_array %s\n", __FUNCTION__,
	       gc0310_res[dev->fmt_idx].desc);
	ret = startup(sd);
	if (ret) {
		dev_err(&client->dev, "gc0310 startup err\n");
		goto err;
	}

	ret = gc0310_get_intg_factor(client, gc0310_info,
				     &gc0310_res[dev->fmt_idx]);
	if (ret) {
		dev_err(&client->dev, "failed to get integration_factor\n");
		goto err;
	}

	pr_info("%s E\n", __func__);
err:
	mutex_unlock(&dev->input_lock);
	return ret;
}

static int gc0310_get_fmt(struct v4l2_subdev *sd,
			  struct v4l2_subdev_pad_config *cfg,
			  struct v4l2_subdev_format *format)
{
	struct v4l2_mbus_framefmt *fmt = &format->format;
	struct gc0310_device *dev = to_gc0310_sensor(sd);

	if (format->pad)
		return -EINVAL;

	if (!fmt)
		return -EINVAL;

	fmt->width = gc0310_res[dev->fmt_idx].width;
	fmt->height = gc0310_res[dev->fmt_idx].height;
	fmt->code = MEDIA_BUS_FMT_SGRBG8_1X8;

	return 0;
}

static int gc0310_detect(struct i2c_client *client)
{
	struct i2c_adapter *adapter = client->adapter;
	u8 high, low;
	int ret;
	u16 id;

	pr_info("%s S\n", __func__);
	if (!i2c_check_functionality(adapter, I2C_FUNC_I2C))
		return -ENODEV;

	ret = gc0310_read_reg(client, GC0310_8BIT,
					GC0310_SC_CMMN_CHIP_ID_H, &high);
	if (ret) {
		dev_err(&client->dev, "read sensor_id_high failed\n");
		return -ENODEV;
	}
	ret = gc0310_read_reg(client, GC0310_8BIT,
					GC0310_SC_CMMN_CHIP_ID_L, &low);
	if (ret) {
		dev_err(&client->dev, "read sensor_id_low failed\n");
		return -ENODEV;
	}
	id = ((((u16) high) << 8) | (u16) low);
	pr_info("sensor ID = 0x%x\n", id);

	if (id != GC0310_ID) {
		dev_err(&client->dev, "sensor ID error, read id = 0x%x, target id = 0x%x\n", id, GC0310_ID);
		return -ENODEV;
	}

	dev_dbg(&client->dev, "detect gc0310 success\n");

	pr_info("%s E\n", __func__);

	return 0;
}

static int gc0310_s_stream(struct v4l2_subdev *sd, int enable)
{
	struct gc0310_device *dev = to_gc0310_sensor(sd);
	struct i2c_client *client = v4l2_get_subdevdata(sd);
	int ret;

	pr_info("%s S enable=%d\n", __func__, enable);
	mutex_lock(&dev->input_lock);

	if (enable) {
		/* enable per frame MIPI and sensor ctrl reset  */
		ret = gc0310_write_reg(client, GC0310_8BIT,
						0xFE, 0x30);
		if (ret) {
			mutex_unlock(&dev->input_lock);
			return ret;
		}
	}

	ret = gc0310_write_reg(client, GC0310_8BIT,
				GC0310_RESET_RELATED, GC0310_REGISTER_PAGE_3);
	if (ret) {
		mutex_unlock(&dev->input_lock);
		return ret;
	}

	ret = gc0310_write_reg(client, GC0310_8BIT, GC0310_SW_STREAM,
				enable ? GC0310_START_STREAMING :
				GC0310_STOP_STREAMING);
	if (ret) {
		mutex_unlock(&dev->input_lock);
		return ret;
	}

	ret = gc0310_write_reg(client, GC0310_8BIT,
				GC0310_RESET_RELATED, GC0310_REGISTER_PAGE_0);
	if (ret) {
		mutex_unlock(&dev->input_lock);
		return ret;
	}

	mutex_unlock(&dev->input_lock);
	pr_info("%s E\n", __func__);
	return ret;
}


static int gc0310_s_config(struct v4l2_subdev *sd,
			   int irq, void *platform_data)
{
	struct gc0310_device *dev = to_gc0310_sensor(sd);
	struct i2c_client *client = v4l2_get_subdevdata(sd);
	int ret = 0;

	pr_info("%s S\n", __func__);
	if (!platform_data)
		return -ENODEV;

	dev->platform_data =
		(struct camera_sensor_platform_data *)platform_data;

	mutex_lock(&dev->input_lock);
	/* power off the module, then power on it in future
	 * as first power on by board may not fulfill the
	 * power on sequqence needed by the module
	 */
	ret = power_down(sd);
	if (ret) {
		dev_err(&client->dev, "gc0310 power-off err.\n");
		goto fail_power_off;
	}

	ret = power_up(sd);
	if (ret) {
		dev_err(&client->dev, "gc0310 power-up err.\n");
		goto fail_power_on;
	}

	ret = dev->platform_data->csi_cfg(sd, 1);
	if (ret)
		goto fail_csi_cfg;

	/* config & detect sensor */
	ret = gc0310_detect(client);
	if (ret) {
		dev_err(&client->dev, "gc0310_detect err s_config.\n");
		goto fail_csi_cfg;
	}

	/* turn off sensor, after probed */
	ret = power_down(sd);
	if (ret) {
		dev_err(&client->dev, "gc0310 power-off err.\n");
		goto fail_csi_cfg;
	}
	mutex_unlock(&dev->input_lock);

	pr_info("%s E\n", __func__);
	return 0;

fail_csi_cfg:
	dev->platform_data->csi_cfg(sd, 0);
fail_power_on:
	power_down(sd);
	dev_err(&client->dev, "sensor power-gating failed\n");
fail_power_off:
	mutex_unlock(&dev->input_lock);
	return ret;
}

static int gc0310_g_frame_interval(struct v4l2_subdev *sd,
				   struct v4l2_subdev_frame_interval *interval)
{
	struct gc0310_device *dev = to_gc0310_sensor(sd);

	interval->interval.numerator = 1;
	interval->interval.denominator = gc0310_res[dev->fmt_idx].fps;

	return 0;
}

static int gc0310_enum_mbus_code(struct v4l2_subdev *sd,
				 struct v4l2_subdev_pad_config *cfg,
				 struct v4l2_subdev_mbus_code_enum *code)
{
	if (code->index >= MAX_FMTS)
		return -EINVAL;

	code->code = MEDIA_BUS_FMT_SGRBG8_1X8;
	return 0;
}

static int gc0310_enum_frame_size(struct v4l2_subdev *sd,
				  struct v4l2_subdev_pad_config *cfg,
				  struct v4l2_subdev_frame_size_enum *fse)
{
	int index = fse->index;

	if (index >= N_RES)
		return -EINVAL;

	fse->min_width = gc0310_res[index].width;
	fse->min_height = gc0310_res[index].height;
	fse->max_width = gc0310_res[index].width;
	fse->max_height = gc0310_res[index].height;

	return 0;

}


static int gc0310_g_skip_frames(struct v4l2_subdev *sd, u32 *frames)
{
	struct gc0310_device *dev = to_gc0310_sensor(sd);

	mutex_lock(&dev->input_lock);
	*frames = gc0310_res[dev->fmt_idx].skip_frames;
	mutex_unlock(&dev->input_lock);

	return 0;
}

static const struct v4l2_subdev_sensor_ops gc0310_sensor_ops = {
	.g_skip_frames	= gc0310_g_skip_frames,
};

static const struct v4l2_subdev_video_ops gc0310_video_ops = {
	.s_stream = gc0310_s_stream,
	.g_frame_interval = gc0310_g_frame_interval,
};

static const struct v4l2_subdev_core_ops gc0310_core_ops = {
	.s_power = gc0310_s_power,
	.ioctl = gc0310_ioctl,
};

static const struct v4l2_subdev_pad_ops gc0310_pad_ops = {
	.enum_mbus_code = gc0310_enum_mbus_code,
	.enum_frame_size = gc0310_enum_frame_size,
	.get_fmt = gc0310_get_fmt,
	.set_fmt = gc0310_set_fmt,
};

static const struct v4l2_subdev_ops gc0310_ops = {
	.core = &gc0310_core_ops,
	.video = &gc0310_video_ops,
	.pad = &gc0310_pad_ops,
	.sensor = &gc0310_sensor_ops,
};

static int gc0310_remove(struct i2c_client *client)
{
	struct v4l2_subdev *sd = i2c_get_clientdata(client);
	struct gc0310_device *dev = to_gc0310_sensor(sd);
	dev_dbg(&client->dev, "gc0310_remove...\n");

	dev->platform_data->csi_cfg(sd, 0);

	v4l2_device_unregister_subdev(sd);
	media_entity_cleanup(&dev->sd.entity);
	v4l2_ctrl_handler_free(&dev->ctrl_handler);
	kfree(dev);

	return 0;
}

static int gc0310_probe(struct i2c_client *client)
{
	struct gc0310_device *dev;
	int ret;
	void *pdata;
	unsigned int i;

	pr_info("%s S\n", __func__);
	dev = kzalloc(sizeof(*dev), GFP_KERNEL);
	if (!dev)
		return -ENOMEM;

	mutex_init(&dev->input_lock);

	dev->fmt_idx = 0;
	v4l2_i2c_subdev_init(&(dev->sd), client, &gc0310_ops);

	pdata = gmin_camera_platform_data(&dev->sd,
					  ATOMISP_INPUT_FORMAT_RAW_8,
					  atomisp_bayer_order_grbg);
	if (!pdata) {
		ret = -EINVAL;
		goto out_free;
	}

	ret = gc0310_s_config(&dev->sd, client->irq, pdata);
	if (ret)
		goto out_free;

	ret = atomisp_register_i2c_module(&dev->sd, pdata, RAW_CAMERA);
	if (ret)
		goto out_free;

	dev->sd.flags |= V4L2_SUBDEV_FL_HAS_DEVNODE;
	dev->pad.flags = MEDIA_PAD_FL_SOURCE;
	dev->format.code = MEDIA_BUS_FMT_SGRBG8_1X8;
	dev->sd.entity.function = MEDIA_ENT_F_CAM_SENSOR;
	ret =
	    v4l2_ctrl_handler_init(&dev->ctrl_handler,
				   ARRAY_SIZE(gc0310_controls));
	if (ret) {
		gc0310_remove(client);
		return ret;
	}

	for (i = 0; i < ARRAY_SIZE(gc0310_controls); i++)
		v4l2_ctrl_new_custom(&dev->ctrl_handler, &gc0310_controls[i],
				     NULL);

	if (dev->ctrl_handler.error) {
		gc0310_remove(client);
		return dev->ctrl_handler.error;
	}

	/* Use same lock for controls as for everything else. */
	dev->ctrl_handler.lock = &dev->input_lock;
	dev->sd.ctrl_handler = &dev->ctrl_handler;

	ret = media_entity_pads_init(&dev->sd.entity, 1, &dev->pad);
	if (ret)
		gc0310_remove(client);

	pr_info("%s E\n", __func__);
	return ret;
out_free:
	v4l2_device_unregister_subdev(&dev->sd);
	kfree(dev);
	return ret;
}

static const struct acpi_device_id gc0310_acpi_match[] = {
	{"XXGC0310"},
	{"INT0310"},
	{},
};
MODULE_DEVICE_TABLE(acpi, gc0310_acpi_match);

static struct i2c_driver gc0310_driver = {
	.driver = {
		.name = "gc0310",
		.acpi_match_table = gc0310_acpi_match,
	},
	.probe_new = gc0310_probe,
	.remove = gc0310_remove,
};
module_i2c_driver(gc0310_driver);

MODULE_AUTHOR("Lai, Angie <angie.lai@intel.com>");
MODULE_DESCRIPTION("A low-level driver for GalaxyCore GC0310 sensors");
MODULE_LICENSE("GPL");
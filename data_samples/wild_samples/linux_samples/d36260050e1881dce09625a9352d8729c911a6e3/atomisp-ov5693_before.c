	if (err != 2) {
		if (err >= 0)
			err = -EIO;
		dev_err(&client->dev,
			"read from offset 0x%x error %d", reg, err);
		return err;
	}

	*val = 0;
	/* high byte comes first */
	if (data_length == OV5693_8BIT)
		*val = (u8)data[0];
	else if (data_length == OV5693_16BIT)
		*val = be16_to_cpu(*(u16 *)&data[0]);
	else
		*val = be32_to_cpu(*(u32 *)&data[0]);

	return 0;
}

static int ov5693_i2c_write(struct i2c_client *client, u16 len, u8 *data)
{
	struct i2c_msg msg;
	const int num_msg = 1;
	int ret;

	msg.addr = client->addr;
	msg.flags = 0;
	msg.len = len;
	msg.buf = data;
	ret = i2c_transfer(client->adapter, &msg, 1);

	return ret == num_msg ? 0 : -EIO;
}

static int vcm_dw_i2c_write(struct i2c_client *client, u16 data)
{
	struct i2c_msg msg;
	const int num_msg = 1;
	int ret;
	u16 val;

	val = cpu_to_be16(data);
	msg.addr = VCM_ADDR;
	msg.flags = 0;
	msg.len = OV5693_16BIT;
	msg.buf = (u8 *)&val;

	ret = i2c_transfer(client->adapter, &msg, 1);

	return ret == num_msg ? 0 : -EIO;
}

/*
 * Theory: per datasheet, the two VCMs both allow for a 2-byte read.
 * The DW9714 doesn't actually specify what this does (it has a
 * two-byte write-only protocol, but specifies the read sequence as
 * legal), but it returns the same data (zeroes) always, after an
 * undocumented initial NAK.  The AD5823 has a one-byte address
 * register to which all writes go, and subsequent reads will cycle
 * through the 8 bytes of registers.  Notably, the default values (the
 * device is always power-cycled affirmatively, so we can rely on
 * these) in AD5823 are not pairwise repetitions of the same 16 bit
 * word.  So all we have to do is sequentially read two bytes at a
 * time and see if we detect a difference in any of the first four
 * pairs.
 */
static int vcm_detect(struct i2c_client *client)
{
	int i, ret;
	struct i2c_msg msg;
	u16 data0 = 0, data;

	for (i = 0; i < 4; i++) {
		msg.addr = VCM_ADDR;
		msg.flags = I2C_M_RD;
		msg.len = sizeof(data);
		msg.buf = (u8 *)&data;
		ret = i2c_transfer(client->adapter, &msg, 1);

		/*
		 * DW9714 always fails the first read and returns
		 * zeroes for subsequent ones
		 */
		if (i == 0 && ret == -EREMOTEIO) {
			data0 = 0;
			continue;
		}

		if (i == 0)
			data0 = data;

		if (data != data0)
			return VCM_AD5823;
	}
{
	struct i2c_msg msg;
	const int num_msg = 1;
	int ret;
	u16 val;

	val = cpu_to_be16(data);
	msg.addr = VCM_ADDR;
	msg.flags = 0;
	msg.len = OV5693_16BIT;
	msg.buf = (u8 *)&val;

	ret = i2c_transfer(client->adapter, &msg, 1);

	return ret == num_msg ? 0 : -EIO;
}

/*
 * Theory: per datasheet, the two VCMs both allow for a 2-byte read.
 * The DW9714 doesn't actually specify what this does (it has a
 * two-byte write-only protocol, but specifies the read sequence as
 * legal), but it returns the same data (zeroes) always, after an
 * undocumented initial NAK.  The AD5823 has a one-byte address
 * register to which all writes go, and subsequent reads will cycle
 * through the 8 bytes of registers.  Notably, the default values (the
 * device is always power-cycled affirmatively, so we can rely on
 * these) in AD5823 are not pairwise repetitions of the same 16 bit
 * word.  So all we have to do is sequentially read two bytes at a
 * time and see if we detect a difference in any of the first four
 * pairs.
 */
static int vcm_detect(struct i2c_client *client)
{
	int i, ret;
	struct i2c_msg msg;
	u16 data0 = 0, data;

	for (i = 0; i < 4; i++) {
		msg.addr = VCM_ADDR;
		msg.flags = I2C_M_RD;
		msg.len = sizeof(data);
		msg.buf = (u8 *)&data;
		ret = i2c_transfer(client->adapter, &msg, 1);

		/*
		 * DW9714 always fails the first read and returns
		 * zeroes for subsequent ones
		 */
		if (i == 0 && ret == -EREMOTEIO) {
			data0 = 0;
			continue;
		}

		if (i == 0)
			data0 = data;

		if (data != data0)
			return VCM_AD5823;
	}
{
	int ret;
	unsigned char data[4] = {0};
	u16 *wreg = (u16 *)data;
	const u16 len = data_length + sizeof(u16); /* 16-bit address + data */

	if (data_length != OV5693_8BIT && data_length != OV5693_16BIT) {
		dev_err(&client->dev,
			"%s error, invalid data_length\n", __func__);
		return -EINVAL;
	}
	} else {
		/* OV5693_16BIT */
		u16 *wdata = (u16 *)&data[2];
		*wdata = cpu_to_be16(val);
	}
{
	u16 size;

	if (ctrl->index == 0)
		return 0;

	size = sizeof(u16) + ctrl->index; /* 16-bit address + data */
	ctrl->buffer.addr = cpu_to_be16(ctrl->buffer.addr);
	ctrl->index = 0;

	return ov5693_i2c_write(client, size, (u8 *)&ctrl->buffer);
}
	switch (next->type) {
	case OV5693_8BIT:
		size = 1;
		ctrl->buffer.data[ctrl->index] = (u8)next->val;
		break;
	case OV5693_16BIT:
		size = 2;
		data16 = (u16 *)&ctrl->buffer.data[ctrl->index];
		*data16 = cpu_to_be16((u16)next->val);
		break;
	default:
		return -EINVAL;
	}

	/* When first item is added, we need to store its starting address */
	if (ctrl->index == 0)
		ctrl->buffer.addr = next->reg;

	ctrl->index += size;

	/*
	 * Buffer cannot guarantee free space for u32? Better flush it to avoid
	 * possible lack of memory for next item.
	 */
	if (ctrl->index + sizeof(u16) >= OV5693_MAX_WRITE_BUF_SIZE)
		return __ov5693_flush_reg_array(client, ctrl);

	return 0;
}

static int __ov5693_write_reg_is_consecutive(struct i2c_client *client,
					     struct ov5693_write_ctrl *ctrl,
					     const struct ov5693_reg *next)
{
	if (ctrl->index == 0)
		return 1;

	return ctrl->buffer.addr + ctrl->index == next->reg;
}

static int ov5693_write_reg_array(struct i2c_client *client,
				  const struct ov5693_reg *reglist)
{
	const struct ov5693_reg *next = reglist;
	struct ov5693_write_ctrl ctrl;
	int err;

	ctrl.index = 0;
	for (; next->type != OV5693_TOK_TERM; next++) {
		switch (next->type & OV5693_TOK_MASK) {
		case OV5693_TOK_DELAY:
			err = __ov5693_flush_reg_array(client, &ctrl);
			if (err)
				return err;
			msleep(next->val);
			break;
		default:
			/*
			 * If next address is not consecutive, data needs to be
			 * flushed before proceed.
			 */
			if (!__ov5693_write_reg_is_consecutive(client, &ctrl,
								next)) {
				err = __ov5693_flush_reg_array(client, &ctrl);
				if (err)
					return err;
			}
			err = __ov5693_buf_reg_array(client, &ctrl, next);
			if (err) {
				dev_err(&client->dev,
					"%s: write error, aborted\n",
					__func__);
				return err;
			}
			break;
		}
{
	struct i2c_client *client = v4l2_get_subdevdata(sd);
	int ret = -EINVAL;
	u8 vcm_code;

	ret = ad5823_i2c_read(client, AD5823_REG_VCM_CODE_MSB, &vcm_code);
	if (ret)
		return ret;

	/* set reg VCM_CODE_MSB Bit[1:0] */
	vcm_code = (vcm_code & VCM_CODE_MSB_MASK) |
		((val >> 8) & ~VCM_CODE_MSB_MASK);
	ret = ad5823_i2c_write(client, AD5823_REG_VCM_CODE_MSB, vcm_code);
	if (ret)
		return ret;

	/* set reg VCM_CODE_LSB Bit[7:0] */
	ret = ad5823_i2c_write(client, AD5823_REG_VCM_CODE_LSB, (val & 0xff));
	if (ret)
		return ret;

	/* set required vcm move time */
	vcm_code = AD5823_RESONANCE_PERIOD / AD5823_RESONANCE_COEF
		- AD5823_HIGH_FREQ_RANGE;
	ret = ad5823_i2c_write(client, AD5823_REG_VCM_MOVE_TIME, vcm_code);

	return ret;
}

int ad5823_t_focus_abs(struct v4l2_subdev *sd, s32 value)
{
	value = min(value, AD5823_MAX_FOCUS_POS);
	return ad5823_t_focus_vcm(sd, value);
}

static int ov5693_t_focus_abs(struct v4l2_subdev *sd, s32 value)
{
	struct ov5693_device *dev = to_ov5693_sensor(sd);
	struct i2c_client *client = v4l2_get_subdevdata(sd);
	int ret = 0;

	dev_dbg(&client->dev, "%s: FOCUS_POS: 0x%x\n", __func__, value);
	value = clamp(value, 0, OV5693_VCM_MAX_FOCUS_POS);
	if (dev->vcm == VCM_DW9714) {
		if (dev->vcm_update) {
			ret = vcm_dw_i2c_write(client, VCM_PROTECTION_OFF);
			if (ret)
				return ret;
			ret = vcm_dw_i2c_write(client, DIRECT_VCM);
			if (ret)
				return ret;
			ret = vcm_dw_i2c_write(client, VCM_PROTECTION_ON);
			if (ret)
				return ret;
			dev->vcm_update = false;
		}
		ret = vcm_dw_i2c_write(client,
				       vcm_val(value, VCM_DEFAULT_S));
	} else if (dev->vcm == VCM_AD5823) {
		ad5823_t_focus_abs(sd, value);
	}
	if (ret == 0) {
		dev->number_of_steps = value - dev->focus;
		dev->focus = value;
		dev->timestamp_t_focus_abs = ktime_get();
	} else
		dev_err(&client->dev,
			"%s: i2c failed. ret %d\n", __func__, ret);

	return ret;
}

static int ov5693_t_focus_rel(struct v4l2_subdev *sd, s32 value)
{
	struct ov5693_device *dev = to_ov5693_sensor(sd);

	return ov5693_t_focus_abs(sd, dev->focus + value);
}

#define DELAY_PER_STEP_NS	1000000
#define DELAY_MAX_PER_STEP_NS	(1000000 * 1023)
static int ov5693_q_focus_status(struct v4l2_subdev *sd, s32 *value)
{
	u32 status = 0;
	struct ov5693_device *dev = to_ov5693_sensor(sd);
	ktime_t temptime;
	ktime_t timedelay = ns_to_ktime(min_t(u32,
			abs(dev->number_of_steps) * DELAY_PER_STEP_NS,
			DELAY_MAX_PER_STEP_NS));

	temptime = ktime_sub(ktime_get(), (dev->timestamp_t_focus_abs));
	if (ktime_compare(temptime, timedelay) <= 0) {
		status |= ATOMISP_FOCUS_STATUS_MOVING;
		status |= ATOMISP_FOCUS_HP_IN_PROGRESS;
	} else {
		status |= ATOMISP_FOCUS_STATUS_ACCEPTS_NEW_MOVE;
		status |= ATOMISP_FOCUS_HP_COMPLETE;
	}

	*value = status;

	return 0;
}

static int ov5693_q_focus_abs(struct v4l2_subdev *sd, s32 *value)
{
	struct ov5693_device *dev = to_ov5693_sensor(sd);
	s32 val;

	ov5693_q_focus_status(sd, &val);

	if (val & ATOMISP_FOCUS_STATUS_MOVING)
		*value  = dev->focus - dev->number_of_steps;
	else
		*value  = dev->focus;

	return 0;
}

static int ov5693_t_vcm_slew(struct v4l2_subdev *sd, s32 value)
{
	struct ov5693_device *dev = to_ov5693_sensor(sd);

	dev->number_of_steps = value;
	dev->vcm_update = true;
	return 0;
}

static int ov5693_t_vcm_timing(struct v4l2_subdev *sd, s32 value)
{
	struct ov5693_device *dev = to_ov5693_sensor(sd);

	dev->number_of_steps = value;
	dev->vcm_update = true;
	return 0;
}

static int ov5693_s_ctrl(struct v4l2_ctrl *ctrl)
{
	struct ov5693_device *dev =
	    container_of(ctrl->handler, struct ov5693_device, ctrl_handler);
	struct i2c_client *client = v4l2_get_subdevdata(&dev->sd);
	int ret = 0;

	switch (ctrl->id) {
	case V4L2_CID_FOCUS_ABSOLUTE:
		dev_dbg(&client->dev, "%s: CID_FOCUS_ABSOLUTE:%d.\n",
			__func__, ctrl->val);
		ret = ov5693_t_focus_abs(&dev->sd, ctrl->val);
		break;
	case V4L2_CID_FOCUS_RELATIVE:
		dev_dbg(&client->dev, "%s: CID_FOCUS_RELATIVE:%d.\n",
			__func__, ctrl->val);
		ret = ov5693_t_focus_rel(&dev->sd, ctrl->val);
		break;
	case V4L2_CID_VCM_SLEW:
		ret = ov5693_t_vcm_slew(&dev->sd, ctrl->val);
		break;
	case V4L2_CID_VCM_TIMEING:
		ret = ov5693_t_vcm_timing(&dev->sd, ctrl->val);
		break;
	default:
		ret = -EINVAL;
	}
	return ret;
}

static int ov5693_g_volatile_ctrl(struct v4l2_ctrl *ctrl)
{
	struct ov5693_device *dev =
	    container_of(ctrl->handler, struct ov5693_device, ctrl_handler);
	int ret = 0;

	switch (ctrl->id) {
	case V4L2_CID_EXPOSURE_ABSOLUTE:
		ret = ov5693_q_exposure(&dev->sd, &ctrl->val);
		break;
	case V4L2_CID_FOCAL_ABSOLUTE:
		ret = ov5693_g_focal(&dev->sd, &ctrl->val);
		break;
	case V4L2_CID_FNUMBER_ABSOLUTE:
		ret = ov5693_g_fnumber(&dev->sd, &ctrl->val);
		break;
	case V4L2_CID_FNUMBER_RANGE:
		ret = ov5693_g_fnumber_range(&dev->sd, &ctrl->val);
		break;
	case V4L2_CID_FOCUS_ABSOLUTE:
		ret = ov5693_q_focus_abs(&dev->sd, &ctrl->val);
		break;
	case V4L2_CID_FOCUS_STATUS:
		ret = ov5693_q_focus_status(&dev->sd, &ctrl->val);
		break;
	case V4L2_CID_BIN_FACTOR_HORZ:
		ret = ov5693_g_bin_factor_x(&dev->sd, &ctrl->val);
		break;
	case V4L2_CID_BIN_FACTOR_VERT:
		ret = ov5693_g_bin_factor_y(&dev->sd, &ctrl->val);
		break;
	default:
		ret = -EINVAL;
	}

	return ret;
}

static const struct v4l2_ctrl_ops ctrl_ops = {
	.s_ctrl = ov5693_s_ctrl,
	.g_volatile_ctrl = ov5693_g_volatile_ctrl
};

struct v4l2_ctrl_config ov5693_controls[] = {
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
	 .id = V4L2_CID_FOCAL_ABSOLUTE,
	 .type = V4L2_CTRL_TYPE_INTEGER,
	 .name = "focal length",
	 .min = OV5693_FOCAL_LENGTH_DEFAULT,
	 .max = OV5693_FOCAL_LENGTH_DEFAULT,
	 .step = 0x01,
	 .def = OV5693_FOCAL_LENGTH_DEFAULT,
	 .flags = 0,
	 },
	{
	 .ops = &ctrl_ops,
	 .id = V4L2_CID_FNUMBER_ABSOLUTE,
	 .type = V4L2_CTRL_TYPE_INTEGER,
	 .name = "f-number",
	 .min = OV5693_F_NUMBER_DEFAULT,
	 .max = OV5693_F_NUMBER_DEFAULT,
	 .step = 0x01,
	 .def = OV5693_F_NUMBER_DEFAULT,
	 .flags = 0,
	 },
	{
	 .ops = &ctrl_ops,
	 .id = V4L2_CID_FNUMBER_RANGE,
	 .type = V4L2_CTRL_TYPE_INTEGER,
	 .name = "f-number range",
	 .min = OV5693_F_NUMBER_RANGE,
	 .max = OV5693_F_NUMBER_RANGE,
	 .step = 0x01,
	 .def = OV5693_F_NUMBER_RANGE,
	 .flags = 0,
	 },
	{
	 .ops = &ctrl_ops,
	 .id = V4L2_CID_FOCUS_ABSOLUTE,
	 .type = V4L2_CTRL_TYPE_INTEGER,
	 .name = "focus move absolute",
	 .min = 0,
	 .max = OV5693_VCM_MAX_FOCUS_POS,
	 .step = 1,
	 .def = 0,
	 .flags = 0,
	 },
	{
	 .ops = &ctrl_ops,
	 .id = V4L2_CID_FOCUS_RELATIVE,
	 .type = V4L2_CTRL_TYPE_INTEGER,
	 .name = "focus move relative",
	 .min = OV5693_VCM_MAX_FOCUS_NEG,
	 .max = OV5693_VCM_MAX_FOCUS_POS,
	 .step = 1,
	 .def = 0,
	 .flags = 0,
	 },
	{
	 .ops = &ctrl_ops,
	 .id = V4L2_CID_FOCUS_STATUS,
	 .type = V4L2_CTRL_TYPE_INTEGER,
	 .name = "focus status",
	 .min = 0,
	 .max = 100,		/* allow enum to grow in the future */
	 .step = 1,
	 .def = 0,
	 .flags = 0,
	 },
	{
	 .ops = &ctrl_ops,
	 .id = V4L2_CID_VCM_SLEW,
	 .type = V4L2_CTRL_TYPE_INTEGER,
	 .name = "vcm slew",
	 .min = 0,
	 .max = OV5693_VCM_SLEW_STEP_MAX,
	 .step = 1,
	 .def = 0,
	 .flags = 0,
	 },
	{
	 .ops = &ctrl_ops,
	 .id = V4L2_CID_VCM_TIMEING,
	 .type = V4L2_CTRL_TYPE_INTEGER,
	 .name = "vcm step time",
	 .min = 0,
	 .max = OV5693_VCM_SLEW_TIME_MAX,
	 .step = 1,
	 .def = 0,
	 .flags = 0,
	 },
	{
	 .ops = &ctrl_ops,
	 .id = V4L2_CID_BIN_FACTOR_HORZ,
	 .type = V4L2_CTRL_TYPE_INTEGER,
	 .name = "horizontal binning factor",
	 .min = 0,
	 .max = OV5693_BIN_FACTOR_MAX,
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
	 .max = OV5693_BIN_FACTOR_MAX,
	 .step = 1,
	 .def = 0,
	 .flags = 0,
	 },
};

static int ov5693_init(struct v4l2_subdev *sd)
{
	struct ov5693_device *dev = to_ov5693_sensor(sd);
	struct i2c_client *client = v4l2_get_subdevdata(sd);
	int ret;

	pr_info("%s\n", __func__);
	mutex_lock(&dev->input_lock);
	dev->vcm_update = false;

	if (dev->vcm == VCM_AD5823) {
		ret = vcm_ad_i2c_wr8(client, 0x01, 0x01); /* vcm init test */
		if (ret)
			dev_err(&client->dev,
				"vcm reset failed\n");
		/*change the mode*/
		ret = ad5823_i2c_write(client, AD5823_REG_VCM_CODE_MSB,
				       AD5823_RING_CTRL_ENABLE);
		if (ret)
			dev_err(&client->dev,
				"vcm enable ringing failed\n");
		ret = ad5823_i2c_write(client, AD5823_REG_MODE,
					AD5823_ARC_RES1);
		if (ret)
			dev_err(&client->dev,
				"vcm change mode failed\n");
	}

	/*change initial focus value for ad5823*/
	if (dev->vcm == VCM_AD5823) {
		dev->focus = AD5823_INIT_FOCUS_POS;
		ov5693_t_focus_abs(sd, AD5823_INIT_FOCUS_POS);
	} else {
		dev->focus = 0;
		ov5693_t_focus_abs(sd, 0);
	}

	mutex_unlock(&dev->input_lock);

	return 0;
}

static int power_ctrl(struct v4l2_subdev *sd, bool flag)
{
	int ret;
	struct ov5693_device *dev = to_ov5693_sensor(sd);

	if (!dev || !dev->platform_data)
		return -ENODEV;

	/*
	 * This driver assumes "internal DVDD, PWDNB tied to DOVDD".
	 * In this set up only gpio0 (XSHUTDN) should be available
	 * but in some products (for example ECS) gpio1 (PWDNB) is
	 * also available. If gpio1 is available we emulate it being
	 * tied to DOVDD here.
	 */
	if (flag) {
		ret = dev->platform_data->v2p8_ctrl(sd, 1);
		dev->platform_data->gpio1_ctrl(sd, 1);
		if (ret == 0) {
			ret = dev->platform_data->v1p8_ctrl(sd, 1);
			if (ret) {
				dev->platform_data->gpio1_ctrl(sd, 0);
				ret = dev->platform_data->v2p8_ctrl(sd, 0);
			}
		}
	} else {
		dev->platform_data->gpio1_ctrl(sd, 0);
		ret = dev->platform_data->v1p8_ctrl(sd, 0);
		ret |= dev->platform_data->v2p8_ctrl(sd, 0);
	}

	return ret;
}

static int gpio_ctrl(struct v4l2_subdev *sd, bool flag)
{
	struct ov5693_device *dev = to_ov5693_sensor(sd);

	if (!dev || !dev->platform_data)
		return -ENODEV;

	return dev->platform_data->gpio0_ctrl(sd, flag);
}

static int __power_up(struct v4l2_subdev *sd)
{
	struct ov5693_device *dev = to_ov5693_sensor(sd);
	struct i2c_client *client = v4l2_get_subdevdata(sd);
	int ret;

	if (!dev->platform_data) {
		dev_err(&client->dev,
			"no camera_sensor_platform_data");
		return -ENODEV;
	}

	/* power control */
	ret = power_ctrl(sd, 1);
	if (ret)
		goto fail_power;

	/* according to DS, at least 5ms is needed between DOVDD and PWDN */
	/* add this delay time to 10~11ms*/
	usleep_range(10000, 11000);

	/* gpio ctrl */
	ret = gpio_ctrl(sd, 1);
	if (ret) {
		ret = gpio_ctrl(sd, 1);
		if (ret)
			goto fail_power;
	}

	/* flis clock control */
	ret = dev->platform_data->flisclk_ctrl(sd, 1);
	if (ret)
		goto fail_clk;

	__cci_delay(up_delay);

	return 0;

fail_clk:
	gpio_ctrl(sd, 0);
fail_power:
	power_ctrl(sd, 0);
	dev_err(&client->dev, "sensor power-up failed\n");

	return ret;
}

static int power_down(struct v4l2_subdev *sd)
{
	struct ov5693_device *dev = to_ov5693_sensor(sd);
	struct i2c_client *client = v4l2_get_subdevdata(sd);
	int ret = 0;

	dev->focus = OV5693_INVALID_CONFIG;
	if (!dev->platform_data) {
		dev_err(&client->dev,
			"no camera_sensor_platform_data");
		return -ENODEV;
	}

	ret = dev->platform_data->flisclk_ctrl(sd, 0);
	if (ret)
		dev_err(&client->dev, "flisclk failed\n");

	/* gpio ctrl */
	ret = gpio_ctrl(sd, 0);
	if (ret) {
		ret = gpio_ctrl(sd, 0);
		if (ret)
			dev_err(&client->dev, "gpio failed 2\n");
	}

	/* power control */
	ret = power_ctrl(sd, 0);
	if (ret)
		dev_err(&client->dev, "vprog failed.\n");

	return ret;
}

static int power_up(struct v4l2_subdev *sd)
{
	static const int retry_count = 4;
	int i, ret;

	for (i = 0; i < retry_count; i++) {
		ret = __power_up(sd);
		if (!ret)
			return 0;

		power_down(sd);
	}
	return ret;
}

static int ov5693_s_power(struct v4l2_subdev *sd, int on)
{
	int ret;

	pr_info("%s: on %d\n", __func__, on);
	if (on == 0)
		return power_down(sd);
	else {
		ret = power_up(sd);
		if (!ret) {
			ret = ov5693_init(sd);
			/* restore settings */
			ov5693_res = ov5693_res_preview;
			N_RES = N_RES_PREVIEW;
		}
	}
	return ret;
}

/*
 * distance - calculate the distance
 * @res: resolution
 * @w: width
 * @h: height
 *
 * Get the gap between res_w/res_h and w/h.
 * distance = (res_w/res_h - w/h) / (w/h) * 8192
 * res->width/height smaller than w/h wouldn't be considered.
 * The gap of ratio larger than 1/8 wouldn't be considered.
 * Returns the value of gap or -1 if fail.
 */
#define LARGEST_ALLOWED_RATIO_MISMATCH 1024
static int distance(struct ov5693_resolution *res, u32 w, u32 h)
{
	int ratio;
	int distance;

	if (w == 0 || h == 0 ||
	    res->width < w || res->height < h)
		return -1;

	ratio = res->width << 13;
	ratio /= w;
	ratio *= h;
	ratio /= res->height;

	distance = abs(ratio - 8192);

	if (distance > LARGEST_ALLOWED_RATIO_MISMATCH)
		return -1;

	return distance;
}

/* Return the nearest higher resolution index
 * Firstly try to find the approximate aspect ratio resolution
 * If we find multiple same AR resolutions, choose the
 * minimal size.
 */
static int nearest_resolution_index(int w, int h)
{
	int i;
	int idx = -1;
	int dist;
	int min_dist = INT_MAX;
	int min_res_w = INT_MAX;
	struct ov5693_resolution *tmp_res = NULL;

	for (i = 0; i < N_RES; i++) {
		tmp_res = &ov5693_res[i];
		dist = distance(tmp_res, w, h);
		if (dist == -1)
			continue;
		if (dist < min_dist) {
			min_dist = dist;
			idx = i;
			min_res_w = ov5693_res[i].width;
			continue;
		}
		if (dist == min_dist && ov5693_res[i].width < min_res_w)
			idx = i;
	}

	return idx;
}

static int get_resolution_index(int w, int h)
{
	int i;

	for (i = 0; i < N_RES; i++) {
		if (w != ov5693_res[i].width)
			continue;
		if (h != ov5693_res[i].height)
			continue;

		return i;
	}

	return -1;
}

/* TODO: remove it. */
static int startup(struct v4l2_subdev *sd)
{
	struct ov5693_device *dev = to_ov5693_sensor(sd);
	struct i2c_client *client = v4l2_get_subdevdata(sd);
	int ret = 0;

	ret = ov5693_write_reg(client, OV5693_8BIT,
					OV5693_SW_RESET, 0x01);
	if (ret) {
		dev_err(&client->dev, "ov5693 reset err.\n");
		return ret;
	}

	ret = ov5693_write_reg_array(client, ov5693_global_setting);
	if (ret) {
		dev_err(&client->dev, "ov5693 write register err.\n");
		return ret;
	}

	ret = ov5693_write_reg_array(client, ov5693_res[dev->fmt_idx].regs);
	if (ret) {
		dev_err(&client->dev, "ov5693 write register err.\n");
		return ret;
	}

	return ret;
}

static int ov5693_set_fmt(struct v4l2_subdev *sd,
			  struct v4l2_subdev_pad_config *cfg,
			  struct v4l2_subdev_format *format)
{
	struct v4l2_mbus_framefmt *fmt = &format->format;
	struct ov5693_device *dev = to_ov5693_sensor(sd);
	struct i2c_client *client = v4l2_get_subdevdata(sd);
	struct camera_mipi_info *ov5693_info = NULL;
	int ret = 0;
	int idx;

	if (format->pad)
		return -EINVAL;
	if (!fmt)
		return -EINVAL;
	ov5693_info = v4l2_get_subdev_hostdata(sd);
	if (ov5693_info == NULL)
		return -EINVAL;

	mutex_lock(&dev->input_lock);
	idx = nearest_resolution_index(fmt->width, fmt->height);
	if (idx == -1) {
		/* return the largest resolution */
		fmt->width = ov5693_res[N_RES - 1].width;
		fmt->height = ov5693_res[N_RES - 1].height;
	} else {
		fmt->width = ov5693_res[idx].width;
		fmt->height = ov5693_res[idx].height;
	}

	fmt->code = MEDIA_BUS_FMT_SBGGR10_1X10;
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

	ret = startup(sd);
	if (ret) {
		int i = 0;

		dev_err(&client->dev, "ov5693 startup err, retry to power up\n");
		for (i = 0; i < OV5693_POWER_UP_RETRY_NUM; i++) {
			dev_err(&client->dev,
				"ov5693 retry to power up %d/%d times, result: ",
				i+1, OV5693_POWER_UP_RETRY_NUM);
			power_down(sd);
			ret = power_up(sd);
			if (!ret) {
				mutex_unlock(&dev->input_lock);
				ov5693_init(sd);
				mutex_lock(&dev->input_lock);
			} else {
				dev_err(&client->dev, "power up failed, continue\n");
				continue;
			}
			ret = startup(sd);
			if (ret) {
				dev_err(&client->dev, " startup FAILED!\n");
			} else {
				dev_err(&client->dev, " startup SUCCESS!\n");
				break;
			}
		}
	}

	/*
	 * After sensor settings are set to HW, sometimes stream is started.
	 * This would cause ISP timeout because ISP is not ready to receive
	 * data yet. So add stop streaming here.
	 */
	ret = ov5693_write_reg(client, OV5693_8BIT, OV5693_SW_STREAM,
				OV5693_STOP_STREAMING);
	if (ret)
		dev_warn(&client->dev, "ov5693 stream off err\n");

	ret = ov5693_get_intg_factor(client, ov5693_info,
					&ov5693_res[dev->fmt_idx]);
	if (ret) {
		dev_err(&client->dev, "failed to get integration_factor\n");
		goto err;
	}

	ov5693_info->metadata_width = fmt->width * 10 / 8;
	ov5693_info->metadata_height = 1;
	ov5693_info->metadata_effective_width = &ov5693_embedded_effective_size;

err:
	mutex_unlock(&dev->input_lock);
	return ret;
}
static int ov5693_get_fmt(struct v4l2_subdev *sd,
			  struct v4l2_subdev_pad_config *cfg,
			  struct v4l2_subdev_format *format)
{
	struct v4l2_mbus_framefmt *fmt = &format->format;
	struct ov5693_device *dev = to_ov5693_sensor(sd);

	if (format->pad)
		return -EINVAL;

	if (!fmt)
		return -EINVAL;

	fmt->width = ov5693_res[dev->fmt_idx].width;
	fmt->height = ov5693_res[dev->fmt_idx].height;
	fmt->code = MEDIA_BUS_FMT_SBGGR10_1X10;

	return 0;
}

static int ov5693_detect(struct i2c_client *client)
{
	struct i2c_adapter *adapter = client->adapter;
	u16 high, low;
	int ret;
	u16 id;
	u8 revision;

	if (!i2c_check_functionality(adapter, I2C_FUNC_I2C))
		return -ENODEV;

	ret = ov5693_read_reg(client, OV5693_8BIT,
					OV5693_SC_CMMN_CHIP_ID_H, &high);
	if (ret) {
		dev_err(&client->dev, "sensor_id_high = 0x%x\n", high);
		return -ENODEV;
	}
	ret = ov5693_read_reg(client, OV5693_8BIT,
					OV5693_SC_CMMN_CHIP_ID_L, &low);
	id = ((((u16) high) << 8) | (u16) low);

	if (id != OV5693_ID) {
		dev_err(&client->dev, "sensor ID error 0x%x\n", id);
		return -ENODEV;
	}

	ret = ov5693_read_reg(client, OV5693_8BIT,
					OV5693_SC_CMMN_SUB_ID, &high);
	revision = (u8) high & 0x0f;

	dev_dbg(&client->dev, "sensor_revision = 0x%x\n", revision);
	dev_dbg(&client->dev, "detect ov5693 success\n");
	return 0;
}

static int ov5693_s_stream(struct v4l2_subdev *sd, int enable)
{
	struct ov5693_device *dev = to_ov5693_sensor(sd);
	struct i2c_client *client = v4l2_get_subdevdata(sd);
	int ret;

	mutex_lock(&dev->input_lock);

	ret = ov5693_write_reg(client, OV5693_8BIT, OV5693_SW_STREAM,
				enable ? OV5693_START_STREAMING :
				OV5693_STOP_STREAMING);

	mutex_unlock(&dev->input_lock);

	return ret;
}


static int ov5693_s_config(struct v4l2_subdev *sd,
			   int irq, void *platform_data)
{
	struct ov5693_device *dev = to_ov5693_sensor(sd);
	struct i2c_client *client = v4l2_get_subdevdata(sd);
	int ret = 0;

	if (platform_data == NULL)
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
		dev_err(&client->dev, "ov5693 power-off err.\n");
		goto fail_power_off;
	}

	ret = power_up(sd);
	if (ret) {
		dev_err(&client->dev, "ov5693 power-up err.\n");
		goto fail_power_on;
	}

	if (!dev->vcm)
		dev->vcm = vcm_detect(client);

	ret = dev->platform_data->csi_cfg(sd, 1);
	if (ret)
		goto fail_csi_cfg;

	/* config & detect sensor */
	ret = ov5693_detect(client);
	if (ret) {
		dev_err(&client->dev, "ov5693_detect err s_config.\n");
		goto fail_csi_cfg;
	}

	dev->otp_data = ov5693_otp_read(sd);

	/* turn off sensor, after probed */
	ret = power_down(sd);
	if (ret) {
		dev_err(&client->dev, "ov5693 power-off err.\n");
		goto fail_csi_cfg;
	}
	mutex_unlock(&dev->input_lock);

	return ret;

fail_csi_cfg:
	dev->platform_data->csi_cfg(sd, 0);
fail_power_on:
	power_down(sd);
	dev_err(&client->dev, "sensor power-gating failed\n");
fail_power_off:
	mutex_unlock(&dev->input_lock);
	return ret;
}

static int ov5693_g_frame_interval(struct v4l2_subdev *sd,
				   struct v4l2_subdev_frame_interval *interval)
{
	struct ov5693_device *dev = to_ov5693_sensor(sd);

	interval->interval.numerator = 1;
	interval->interval.denominator = ov5693_res[dev->fmt_idx].fps;

	return 0;
}

static int ov5693_enum_mbus_code(struct v4l2_subdev *sd,
				 struct v4l2_subdev_pad_config *cfg,
				 struct v4l2_subdev_mbus_code_enum *code)
{
	if (code->index >= MAX_FMTS)
		return -EINVAL;

	code->code = MEDIA_BUS_FMT_SBGGR10_1X10;
	return 0;
}

static int ov5693_enum_frame_size(struct v4l2_subdev *sd,
				  struct v4l2_subdev_pad_config *cfg,
				  struct v4l2_subdev_frame_size_enum *fse)
{
	int index = fse->index;

	if (index >= N_RES)
		return -EINVAL;

	fse->min_width = ov5693_res[index].width;
	fse->min_height = ov5693_res[index].height;
	fse->max_width = ov5693_res[index].width;
	fse->max_height = ov5693_res[index].height;

	return 0;

}

static const struct v4l2_subdev_video_ops ov5693_video_ops = {
	.s_stream = ov5693_s_stream,
	.g_frame_interval = ov5693_g_frame_interval,
};

static const struct v4l2_subdev_core_ops ov5693_core_ops = {
	.s_power = ov5693_s_power,
	.ioctl = ov5693_ioctl,
};

static const struct v4l2_subdev_pad_ops ov5693_pad_ops = {
	.enum_mbus_code = ov5693_enum_mbus_code,
	.enum_frame_size = ov5693_enum_frame_size,
	.get_fmt = ov5693_get_fmt,
	.set_fmt = ov5693_set_fmt,
};

static const struct v4l2_subdev_ops ov5693_ops = {
	.core = &ov5693_core_ops,
	.video = &ov5693_video_ops,
	.pad = &ov5693_pad_ops,
};

static int ov5693_remove(struct i2c_client *client)
{
	struct v4l2_subdev *sd = i2c_get_clientdata(client);
	struct ov5693_device *dev = to_ov5693_sensor(sd);

	dev_dbg(&client->dev, "ov5693_remove...\n");

	dev->platform_data->csi_cfg(sd, 0);

	v4l2_device_unregister_subdev(sd);

	atomisp_gmin_remove_subdev(sd);

	media_entity_cleanup(&dev->sd.entity);
	v4l2_ctrl_handler_free(&dev->ctrl_handler);
	kfree(dev);

	return 0;
}

static int ov5693_probe(struct i2c_client *client)
{
	struct ov5693_device *dev;
	int i2c;
	int ret = 0;
	void *pdata;
	unsigned int i;

	/*
	 * Firmware workaround: Some modules use a "secondary default"
	 * address of 0x10 which doesn't appear on schematics, and
	 * some BIOS versions haven't gotten the memo.  Work around
	 * via config.
	 */
	i2c = gmin_get_var_int(&client->dev, "I2CAddr", -1);
	if (i2c != -1) {
		dev_info(&client->dev,
		"Overriding firmware-provided I2C address (0x%x) with 0x%x\n",
			 client->addr, i2c);
		client->addr = i2c;
	}

	dev = kzalloc(sizeof(*dev), GFP_KERNEL);
	if (!dev)
		return -ENOMEM;

	mutex_init(&dev->input_lock);

	dev->fmt_idx = 0;
	v4l2_i2c_subdev_init(&(dev->sd), client, &ov5693_ops);

	pdata = gmin_camera_platform_data(&dev->sd,
					  ATOMISP_INPUT_FORMAT_RAW_10,
					  atomisp_bayer_order_bggr);
	if (!pdata)
		goto out_free;

	ret = ov5693_s_config(&dev->sd, client->irq, pdata);
	if (ret)
		goto out_free;

	ret = atomisp_register_i2c_module(&dev->sd, pdata, RAW_CAMERA);
	if (ret)
		goto out_free;

	dev->sd.flags |= V4L2_SUBDEV_FL_HAS_DEVNODE;
	dev->pad.flags = MEDIA_PAD_FL_SOURCE;
	dev->format.code = MEDIA_BUS_FMT_SBGGR10_1X10;
	dev->sd.entity.function = MEDIA_ENT_F_CAM_SENSOR;
	ret =
	    v4l2_ctrl_handler_init(&dev->ctrl_handler,
				   ARRAY_SIZE(ov5693_controls));
	if (ret) {
		ov5693_remove(client);
		return ret;
	}

	for (i = 0; i < ARRAY_SIZE(ov5693_controls); i++)
		v4l2_ctrl_new_custom(&dev->ctrl_handler, &ov5693_controls[i],
				     NULL);

	if (dev->ctrl_handler.error) {
		ov5693_remove(client);
		return dev->ctrl_handler.error;
	}

	/* Use same lock for controls as for everything else. */
	dev->ctrl_handler.lock = &dev->input_lock;
	dev->sd.ctrl_handler = &dev->ctrl_handler;

	ret = media_entity_pads_init(&dev->sd.entity, 1, &dev->pad);
	if (ret)
		ov5693_remove(client);

	return ret;
out_free:
	v4l2_device_unregister_subdev(&dev->sd);
	kfree(dev);
	return ret;
}

static const struct acpi_device_id ov5693_acpi_match[] = {
	{"INT33BE"},
	{},
};
MODULE_DEVICE_TABLE(acpi, ov5693_acpi_match);

static struct i2c_driver ov5693_driver = {
	.driver = {
		.name = "ov5693",
		.acpi_match_table = ov5693_acpi_match,
	},
	.probe_new = ov5693_probe,
	.remove = ov5693_remove,
};
module_i2c_driver(ov5693_driver);

MODULE_DESCRIPTION("A low-level driver for OmniVision 5693 sensors");
MODULE_LICENSE("GPL");
static const struct v4l2_ctrl_ops ctrl_ops = {
	.s_ctrl = ov5693_s_ctrl,
	.g_volatile_ctrl = ov5693_g_volatile_ctrl
};

struct v4l2_ctrl_config ov5693_controls[] = {
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
	 .id = V4L2_CID_FOCAL_ABSOLUTE,
	 .type = V4L2_CTRL_TYPE_INTEGER,
	 .name = "focal length",
	 .min = OV5693_FOCAL_LENGTH_DEFAULT,
	 .max = OV5693_FOCAL_LENGTH_DEFAULT,
	 .step = 0x01,
	 .def = OV5693_FOCAL_LENGTH_DEFAULT,
	 .flags = 0,
	 },
	{
	 .ops = &ctrl_ops,
	 .id = V4L2_CID_FNUMBER_ABSOLUTE,
	 .type = V4L2_CTRL_TYPE_INTEGER,
	 .name = "f-number",
	 .min = OV5693_F_NUMBER_DEFAULT,
	 .max = OV5693_F_NUMBER_DEFAULT,
	 .step = 0x01,
	 .def = OV5693_F_NUMBER_DEFAULT,
	 .flags = 0,
	 },
	{
	 .ops = &ctrl_ops,
	 .id = V4L2_CID_FNUMBER_RANGE,
	 .type = V4L2_CTRL_TYPE_INTEGER,
	 .name = "f-number range",
	 .min = OV5693_F_NUMBER_RANGE,
	 .max = OV5693_F_NUMBER_RANGE,
	 .step = 0x01,
	 .def = OV5693_F_NUMBER_RANGE,
	 .flags = 0,
	 },
	{
	 .ops = &ctrl_ops,
	 .id = V4L2_CID_FOCUS_ABSOLUTE,
	 .type = V4L2_CTRL_TYPE_INTEGER,
	 .name = "focus move absolute",
	 .min = 0,
	 .max = OV5693_VCM_MAX_FOCUS_POS,
	 .step = 1,
	 .def = 0,
	 .flags = 0,
	 },
	{
	 .ops = &ctrl_ops,
	 .id = V4L2_CID_FOCUS_RELATIVE,
	 .type = V4L2_CTRL_TYPE_INTEGER,
	 .name = "focus move relative",
	 .min = OV5693_VCM_MAX_FOCUS_NEG,
	 .max = OV5693_VCM_MAX_FOCUS_POS,
	 .step = 1,
	 .def = 0,
	 .flags = 0,
	 },
	{
	 .ops = &ctrl_ops,
	 .id = V4L2_CID_FOCUS_STATUS,
	 .type = V4L2_CTRL_TYPE_INTEGER,
	 .name = "focus status",
	 .min = 0,
	 .max = 100,		/* allow enum to grow in the future */
	 .step = 1,
	 .def = 0,
	 .flags = 0,
	 },
	{
	 .ops = &ctrl_ops,
	 .id = V4L2_CID_VCM_SLEW,
	 .type = V4L2_CTRL_TYPE_INTEGER,
	 .name = "vcm slew",
	 .min = 0,
	 .max = OV5693_VCM_SLEW_STEP_MAX,
	 .step = 1,
	 .def = 0,
	 .flags = 0,
	 },
	{
	 .ops = &ctrl_ops,
	 .id = V4L2_CID_VCM_TIMEING,
	 .type = V4L2_CTRL_TYPE_INTEGER,
	 .name = "vcm step time",
	 .min = 0,
	 .max = OV5693_VCM_SLEW_TIME_MAX,
	 .step = 1,
	 .def = 0,
	 .flags = 0,
	 },
	{
	 .ops = &ctrl_ops,
	 .id = V4L2_CID_BIN_FACTOR_HORZ,
	 .type = V4L2_CTRL_TYPE_INTEGER,
	 .name = "horizontal binning factor",
	 .min = 0,
	 .max = OV5693_BIN_FACTOR_MAX,
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
	 .max = OV5693_BIN_FACTOR_MAX,
	 .step = 1,
	 .def = 0,
	 .flags = 0,
	 },
};

static int ov5693_init(struct v4l2_subdev *sd)
{
	struct ov5693_device *dev = to_ov5693_sensor(sd);
	struct i2c_client *client = v4l2_get_subdevdata(sd);
	int ret;

	pr_info("%s\n", __func__);
	mutex_lock(&dev->input_lock);
	dev->vcm_update = false;

	if (dev->vcm == VCM_AD5823) {
		ret = vcm_ad_i2c_wr8(client, 0x01, 0x01); /* vcm init test */
		if (ret)
			dev_err(&client->dev,
				"vcm reset failed\n");
		/*change the mode*/
		ret = ad5823_i2c_write(client, AD5823_REG_VCM_CODE_MSB,
				       AD5823_RING_CTRL_ENABLE);
		if (ret)
			dev_err(&client->dev,
				"vcm enable ringing failed\n");
		ret = ad5823_i2c_write(client, AD5823_REG_MODE,
					AD5823_ARC_RES1);
		if (ret)
			dev_err(&client->dev,
				"vcm change mode failed\n");
	}

	/*change initial focus value for ad5823*/
	if (dev->vcm == VCM_AD5823) {
		dev->focus = AD5823_INIT_FOCUS_POS;
		ov5693_t_focus_abs(sd, AD5823_INIT_FOCUS_POS);
	} else {
		dev->focus = 0;
		ov5693_t_focus_abs(sd, 0);
	}

	mutex_unlock(&dev->input_lock);

	return 0;
}

static int power_ctrl(struct v4l2_subdev *sd, bool flag)
{
	int ret;
	struct ov5693_device *dev = to_ov5693_sensor(sd);

	if (!dev || !dev->platform_data)
		return -ENODEV;

	/*
	 * This driver assumes "internal DVDD, PWDNB tied to DOVDD".
	 * In this set up only gpio0 (XSHUTDN) should be available
	 * but in some products (for example ECS) gpio1 (PWDNB) is
	 * also available. If gpio1 is available we emulate it being
	 * tied to DOVDD here.
	 */
	if (flag) {
		ret = dev->platform_data->v2p8_ctrl(sd, 1);
		dev->platform_data->gpio1_ctrl(sd, 1);
		if (ret == 0) {
			ret = dev->platform_data->v1p8_ctrl(sd, 1);
			if (ret) {
				dev->platform_data->gpio1_ctrl(sd, 0);
				ret = dev->platform_data->v2p8_ctrl(sd, 0);
			}
		}
	} else {
		dev->platform_data->gpio1_ctrl(sd, 0);
		ret = dev->platform_data->v1p8_ctrl(sd, 0);
		ret |= dev->platform_data->v2p8_ctrl(sd, 0);
	}

	return ret;
}

static int gpio_ctrl(struct v4l2_subdev *sd, bool flag)
{
	struct ov5693_device *dev = to_ov5693_sensor(sd);

	if (!dev || !dev->platform_data)
		return -ENODEV;

	return dev->platform_data->gpio0_ctrl(sd, flag);
}

static int __power_up(struct v4l2_subdev *sd)
{
	struct ov5693_device *dev = to_ov5693_sensor(sd);
	struct i2c_client *client = v4l2_get_subdevdata(sd);
	int ret;

	if (!dev->platform_data) {
		dev_err(&client->dev,
			"no camera_sensor_platform_data");
		return -ENODEV;
	}

	/* power control */
	ret = power_ctrl(sd, 1);
	if (ret)
		goto fail_power;

	/* according to DS, at least 5ms is needed between DOVDD and PWDN */
	/* add this delay time to 10~11ms*/
	usleep_range(10000, 11000);

	/* gpio ctrl */
	ret = gpio_ctrl(sd, 1);
	if (ret) {
		ret = gpio_ctrl(sd, 1);
		if (ret)
			goto fail_power;
	}

	/* flis clock control */
	ret = dev->platform_data->flisclk_ctrl(sd, 1);
	if (ret)
		goto fail_clk;

	__cci_delay(up_delay);

	return 0;

fail_clk:
	gpio_ctrl(sd, 0);
fail_power:
	power_ctrl(sd, 0);
	dev_err(&client->dev, "sensor power-up failed\n");

	return ret;
}

static int power_down(struct v4l2_subdev *sd)
{
	struct ov5693_device *dev = to_ov5693_sensor(sd);
	struct i2c_client *client = v4l2_get_subdevdata(sd);
	int ret = 0;

	dev->focus = OV5693_INVALID_CONFIG;
	if (!dev->platform_data) {
		dev_err(&client->dev,
			"no camera_sensor_platform_data");
		return -ENODEV;
	}

	ret = dev->platform_data->flisclk_ctrl(sd, 0);
	if (ret)
		dev_err(&client->dev, "flisclk failed\n");

	/* gpio ctrl */
	ret = gpio_ctrl(sd, 0);
	if (ret) {
		ret = gpio_ctrl(sd, 0);
		if (ret)
			dev_err(&client->dev, "gpio failed 2\n");
	}

	/* power control */
	ret = power_ctrl(sd, 0);
	if (ret)
		dev_err(&client->dev, "vprog failed.\n");

	return ret;
}

static int power_up(struct v4l2_subdev *sd)
{
	static const int retry_count = 4;
	int i, ret;

	for (i = 0; i < retry_count; i++) {
		ret = __power_up(sd);
		if (!ret)
			return 0;

		power_down(sd);
	}
	return ret;
}

static int ov5693_s_power(struct v4l2_subdev *sd, int on)
{
	int ret;

	pr_info("%s: on %d\n", __func__, on);
	if (on == 0)
		return power_down(sd);
	else {
		ret = power_up(sd);
		if (!ret) {
			ret = ov5693_init(sd);
			/* restore settings */
			ov5693_res = ov5693_res_preview;
			N_RES = N_RES_PREVIEW;
		}
	}
	return ret;
}

/*
 * distance - calculate the distance
 * @res: resolution
 * @w: width
 * @h: height
 *
 * Get the gap between res_w/res_h and w/h.
 * distance = (res_w/res_h - w/h) / (w/h) * 8192
 * res->width/height smaller than w/h wouldn't be considered.
 * The gap of ratio larger than 1/8 wouldn't be considered.
 * Returns the value of gap or -1 if fail.
 */
#define LARGEST_ALLOWED_RATIO_MISMATCH 1024
static int distance(struct ov5693_resolution *res, u32 w, u32 h)
{
	int ratio;
	int distance;

	if (w == 0 || h == 0 ||
	    res->width < w || res->height < h)
		return -1;

	ratio = res->width << 13;
	ratio /= w;
	ratio *= h;
	ratio /= res->height;

	distance = abs(ratio - 8192);

	if (distance > LARGEST_ALLOWED_RATIO_MISMATCH)
		return -1;

	return distance;
}

/* Return the nearest higher resolution index
 * Firstly try to find the approximate aspect ratio resolution
 * If we find multiple same AR resolutions, choose the
 * minimal size.
 */
static int nearest_resolution_index(int w, int h)
{
	int i;
	int idx = -1;
	int dist;
	int min_dist = INT_MAX;
	int min_res_w = INT_MAX;
	struct ov5693_resolution *tmp_res = NULL;

	for (i = 0; i < N_RES; i++) {
		tmp_res = &ov5693_res[i];
		dist = distance(tmp_res, w, h);
		if (dist == -1)
			continue;
		if (dist < min_dist) {
			min_dist = dist;
			idx = i;
			min_res_w = ov5693_res[i].width;
			continue;
		}
		if (dist == min_dist && ov5693_res[i].width < min_res_w)
			idx = i;
	}

	return idx;
}

static int get_resolution_index(int w, int h)
{
	int i;

	for (i = 0; i < N_RES; i++) {
		if (w != ov5693_res[i].width)
			continue;
		if (h != ov5693_res[i].height)
			continue;

		return i;
	}

	return -1;
}

/* TODO: remove it. */
static int startup(struct v4l2_subdev *sd)
{
	struct ov5693_device *dev = to_ov5693_sensor(sd);
	struct i2c_client *client = v4l2_get_subdevdata(sd);
	int ret = 0;

	ret = ov5693_write_reg(client, OV5693_8BIT,
					OV5693_SW_RESET, 0x01);
	if (ret) {
		dev_err(&client->dev, "ov5693 reset err.\n");
		return ret;
	}

	ret = ov5693_write_reg_array(client, ov5693_global_setting);
	if (ret) {
		dev_err(&client->dev, "ov5693 write register err.\n");
		return ret;
	}

	ret = ov5693_write_reg_array(client, ov5693_res[dev->fmt_idx].regs);
	if (ret) {
		dev_err(&client->dev, "ov5693 write register err.\n");
		return ret;
	}

	return ret;
}

static int ov5693_set_fmt(struct v4l2_subdev *sd,
			  struct v4l2_subdev_pad_config *cfg,
			  struct v4l2_subdev_format *format)
{
	struct v4l2_mbus_framefmt *fmt = &format->format;
	struct ov5693_device *dev = to_ov5693_sensor(sd);
	struct i2c_client *client = v4l2_get_subdevdata(sd);
	struct camera_mipi_info *ov5693_info = NULL;
	int ret = 0;
	int idx;

	if (format->pad)
		return -EINVAL;
	if (!fmt)
		return -EINVAL;
	ov5693_info = v4l2_get_subdev_hostdata(sd);
	if (ov5693_info == NULL)
		return -EINVAL;

	mutex_lock(&dev->input_lock);
	idx = nearest_resolution_index(fmt->width, fmt->height);
	if (idx == -1) {
		/* return the largest resolution */
		fmt->width = ov5693_res[N_RES - 1].width;
		fmt->height = ov5693_res[N_RES - 1].height;
	} else {
		fmt->width = ov5693_res[idx].width;
		fmt->height = ov5693_res[idx].height;
	}

	fmt->code = MEDIA_BUS_FMT_SBGGR10_1X10;
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

	ret = startup(sd);
	if (ret) {
		int i = 0;

		dev_err(&client->dev, "ov5693 startup err, retry to power up\n");
		for (i = 0; i < OV5693_POWER_UP_RETRY_NUM; i++) {
			dev_err(&client->dev,
				"ov5693 retry to power up %d/%d times, result: ",
				i+1, OV5693_POWER_UP_RETRY_NUM);
			power_down(sd);
			ret = power_up(sd);
			if (!ret) {
				mutex_unlock(&dev->input_lock);
				ov5693_init(sd);
				mutex_lock(&dev->input_lock);
			} else {
				dev_err(&client->dev, "power up failed, continue\n");
				continue;
			}
			ret = startup(sd);
			if (ret) {
				dev_err(&client->dev, " startup FAILED!\n");
			} else {
				dev_err(&client->dev, " startup SUCCESS!\n");
				break;
			}
		}
	}

	/*
	 * After sensor settings are set to HW, sometimes stream is started.
	 * This would cause ISP timeout because ISP is not ready to receive
	 * data yet. So add stop streaming here.
	 */
	ret = ov5693_write_reg(client, OV5693_8BIT, OV5693_SW_STREAM,
				OV5693_STOP_STREAMING);
	if (ret)
		dev_warn(&client->dev, "ov5693 stream off err\n");

	ret = ov5693_get_intg_factor(client, ov5693_info,
					&ov5693_res[dev->fmt_idx]);
	if (ret) {
		dev_err(&client->dev, "failed to get integration_factor\n");
		goto err;
	}

	ov5693_info->metadata_width = fmt->width * 10 / 8;
	ov5693_info->metadata_height = 1;
	ov5693_info->metadata_effective_width = &ov5693_embedded_effective_size;

err:
	mutex_unlock(&dev->input_lock);
	return ret;
}
static int ov5693_get_fmt(struct v4l2_subdev *sd,
			  struct v4l2_subdev_pad_config *cfg,
			  struct v4l2_subdev_format *format)
{
	struct v4l2_mbus_framefmt *fmt = &format->format;
	struct ov5693_device *dev = to_ov5693_sensor(sd);

	if (format->pad)
		return -EINVAL;

	if (!fmt)
		return -EINVAL;

	fmt->width = ov5693_res[dev->fmt_idx].width;
	fmt->height = ov5693_res[dev->fmt_idx].height;
	fmt->code = MEDIA_BUS_FMT_SBGGR10_1X10;

	return 0;
}

static int ov5693_detect(struct i2c_client *client)
{
	struct i2c_adapter *adapter = client->adapter;
	u16 high, low;
	int ret;
	u16 id;
	u8 revision;

	if (!i2c_check_functionality(adapter, I2C_FUNC_I2C))
		return -ENODEV;

	ret = ov5693_read_reg(client, OV5693_8BIT,
					OV5693_SC_CMMN_CHIP_ID_H, &high);
	if (ret) {
		dev_err(&client->dev, "sensor_id_high = 0x%x\n", high);
		return -ENODEV;
	}
	ret = ov5693_read_reg(client, OV5693_8BIT,
					OV5693_SC_CMMN_CHIP_ID_L, &low);
	id = ((((u16) high) << 8) | (u16) low);

	if (id != OV5693_ID) {
		dev_err(&client->dev, "sensor ID error 0x%x\n", id);
		return -ENODEV;
	}

	ret = ov5693_read_reg(client, OV5693_8BIT,
					OV5693_SC_CMMN_SUB_ID, &high);
	revision = (u8) high & 0x0f;

	dev_dbg(&client->dev, "sensor_revision = 0x%x\n", revision);
	dev_dbg(&client->dev, "detect ov5693 success\n");
	return 0;
}

static int ov5693_s_stream(struct v4l2_subdev *sd, int enable)
{
	struct ov5693_device *dev = to_ov5693_sensor(sd);
	struct i2c_client *client = v4l2_get_subdevdata(sd);
	int ret;

	mutex_lock(&dev->input_lock);

	ret = ov5693_write_reg(client, OV5693_8BIT, OV5693_SW_STREAM,
				enable ? OV5693_START_STREAMING :
				OV5693_STOP_STREAMING);

	mutex_unlock(&dev->input_lock);

	return ret;
}


static int ov5693_s_config(struct v4l2_subdev *sd,
			   int irq, void *platform_data)
{
	struct ov5693_device *dev = to_ov5693_sensor(sd);
	struct i2c_client *client = v4l2_get_subdevdata(sd);
	int ret = 0;

	if (platform_data == NULL)
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
		dev_err(&client->dev, "ov5693 power-off err.\n");
		goto fail_power_off;
	}

	ret = power_up(sd);
	if (ret) {
		dev_err(&client->dev, "ov5693 power-up err.\n");
		goto fail_power_on;
	}

	if (!dev->vcm)
		dev->vcm = vcm_detect(client);

	ret = dev->platform_data->csi_cfg(sd, 1);
	if (ret)
		goto fail_csi_cfg;

	/* config & detect sensor */
	ret = ov5693_detect(client);
	if (ret) {
		dev_err(&client->dev, "ov5693_detect err s_config.\n");
		goto fail_csi_cfg;
	}

	dev->otp_data = ov5693_otp_read(sd);

	/* turn off sensor, after probed */
	ret = power_down(sd);
	if (ret) {
		dev_err(&client->dev, "ov5693 power-off err.\n");
		goto fail_csi_cfg;
	}
	mutex_unlock(&dev->input_lock);

	return ret;

fail_csi_cfg:
	dev->platform_data->csi_cfg(sd, 0);
fail_power_on:
	power_down(sd);
	dev_err(&client->dev, "sensor power-gating failed\n");
fail_power_off:
	mutex_unlock(&dev->input_lock);
	return ret;
}

static int ov5693_g_frame_interval(struct v4l2_subdev *sd,
				   struct v4l2_subdev_frame_interval *interval)
{
	struct ov5693_device *dev = to_ov5693_sensor(sd);

	interval->interval.numerator = 1;
	interval->interval.denominator = ov5693_res[dev->fmt_idx].fps;

	return 0;
}

static int ov5693_enum_mbus_code(struct v4l2_subdev *sd,
				 struct v4l2_subdev_pad_config *cfg,
				 struct v4l2_subdev_mbus_code_enum *code)
{
	if (code->index >= MAX_FMTS)
		return -EINVAL;

	code->code = MEDIA_BUS_FMT_SBGGR10_1X10;
	return 0;
}

static int ov5693_enum_frame_size(struct v4l2_subdev *sd,
				  struct v4l2_subdev_pad_config *cfg,
				  struct v4l2_subdev_frame_size_enum *fse)
{
	int index = fse->index;

	if (index >= N_RES)
		return -EINVAL;

	fse->min_width = ov5693_res[index].width;
	fse->min_height = ov5693_res[index].height;
	fse->max_width = ov5693_res[index].width;
	fse->max_height = ov5693_res[index].height;

	return 0;

}

static const struct v4l2_subdev_video_ops ov5693_video_ops = {
	.s_stream = ov5693_s_stream,
	.g_frame_interval = ov5693_g_frame_interval,
};

static const struct v4l2_subdev_core_ops ov5693_core_ops = {
	.s_power = ov5693_s_power,
	.ioctl = ov5693_ioctl,
};

static const struct v4l2_subdev_pad_ops ov5693_pad_ops = {
	.enum_mbus_code = ov5693_enum_mbus_code,
	.enum_frame_size = ov5693_enum_frame_size,
	.get_fmt = ov5693_get_fmt,
	.set_fmt = ov5693_set_fmt,
};

static const struct v4l2_subdev_ops ov5693_ops = {
	.core = &ov5693_core_ops,
	.video = &ov5693_video_ops,
	.pad = &ov5693_pad_ops,
};

static int ov5693_remove(struct i2c_client *client)
{
	struct v4l2_subdev *sd = i2c_get_clientdata(client);
	struct ov5693_device *dev = to_ov5693_sensor(sd);

	dev_dbg(&client->dev, "ov5693_remove...\n");

	dev->platform_data->csi_cfg(sd, 0);

	v4l2_device_unregister_subdev(sd);

	atomisp_gmin_remove_subdev(sd);

	media_entity_cleanup(&dev->sd.entity);
	v4l2_ctrl_handler_free(&dev->ctrl_handler);
	kfree(dev);

	return 0;
}

static int ov5693_probe(struct i2c_client *client)
{
	struct ov5693_device *dev;
	int i2c;
	int ret = 0;
	void *pdata;
	unsigned int i;

	/*
	 * Firmware workaround: Some modules use a "secondary default"
	 * address of 0x10 which doesn't appear on schematics, and
	 * some BIOS versions haven't gotten the memo.  Work around
	 * via config.
	 */
	i2c = gmin_get_var_int(&client->dev, "I2CAddr", -1);
	if (i2c != -1) {
		dev_info(&client->dev,
		"Overriding firmware-provided I2C address (0x%x) with 0x%x\n",
			 client->addr, i2c);
		client->addr = i2c;
	}

	dev = kzalloc(sizeof(*dev), GFP_KERNEL);
	if (!dev)
		return -ENOMEM;

	mutex_init(&dev->input_lock);

	dev->fmt_idx = 0;
	v4l2_i2c_subdev_init(&(dev->sd), client, &ov5693_ops);

	pdata = gmin_camera_platform_data(&dev->sd,
					  ATOMISP_INPUT_FORMAT_RAW_10,
					  atomisp_bayer_order_bggr);
	if (!pdata)
		goto out_free;

	ret = ov5693_s_config(&dev->sd, client->irq, pdata);
	if (ret)
		goto out_free;

	ret = atomisp_register_i2c_module(&dev->sd, pdata, RAW_CAMERA);
	if (ret)
		goto out_free;

	dev->sd.flags |= V4L2_SUBDEV_FL_HAS_DEVNODE;
	dev->pad.flags = MEDIA_PAD_FL_SOURCE;
	dev->format.code = MEDIA_BUS_FMT_SBGGR10_1X10;
	dev->sd.entity.function = MEDIA_ENT_F_CAM_SENSOR;
	ret =
	    v4l2_ctrl_handler_init(&dev->ctrl_handler,
				   ARRAY_SIZE(ov5693_controls));
	if (ret) {
		ov5693_remove(client);
		return ret;
	}

	for (i = 0; i < ARRAY_SIZE(ov5693_controls); i++)
		v4l2_ctrl_new_custom(&dev->ctrl_handler, &ov5693_controls[i],
				     NULL);

	if (dev->ctrl_handler.error) {
		ov5693_remove(client);
		return dev->ctrl_handler.error;
	}

	/* Use same lock for controls as for everything else. */
	dev->ctrl_handler.lock = &dev->input_lock;
	dev->sd.ctrl_handler = &dev->ctrl_handler;

	ret = media_entity_pads_init(&dev->sd.entity, 1, &dev->pad);
	if (ret)
		ov5693_remove(client);

	return ret;
out_free:
	v4l2_device_unregister_subdev(&dev->sd);
	kfree(dev);
	return ret;
}

static const struct acpi_device_id ov5693_acpi_match[] = {
	{"INT33BE"},
	{},
};
MODULE_DEVICE_TABLE(acpi, ov5693_acpi_match);

static struct i2c_driver ov5693_driver = {
	.driver = {
		.name = "ov5693",
		.acpi_match_table = ov5693_acpi_match,
	},
	.probe_new = ov5693_probe,
	.remove = ov5693_remove,
};
module_i2c_driver(ov5693_driver);

MODULE_DESCRIPTION("A low-level driver for OmniVision 5693 sensors");
MODULE_LICENSE("GPL");
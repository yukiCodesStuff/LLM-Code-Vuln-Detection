	if (data_length == OV2722_8BIT)
		*val = (u8)data[0];
	else if (data_length == OV2722_16BIT)
		*val = be16_to_cpu(*(u16 *)&data[0]);
	else
		*val = be32_to_cpu(*(u32 *)&data[0]);

	return 0;
}

{
	int ret;
	unsigned char data[4] = {0};
	u16 *wreg = (u16 *)data;
	const u16 len = data_length + sizeof(u16); /* 16-bit address + data */

	if (data_length != OV2722_8BIT && data_length != OV2722_16BIT) {
		dev_err(&client->dev,
		data[2] = (u8)(val);
	} else {
		/* OV2722_16BIT */
		u16 *wdata = (u16 *)&data[2];
		*wdata = cpu_to_be16(val);
	}

	ret = ov2722_i2c_write(client, len, data);
				    struct ov2722_write_ctrl *ctrl)
{
	u16 size;

	if (ctrl->index == 0)
		return 0;

	size = sizeof(u16) + ctrl->index; /* 16-bit address + data */
	ctrl->buffer.addr = cpu_to_be16(ctrl->buffer.addr);
	ctrl->index = 0;

	return ov2722_i2c_write(client, size, (u8 *)&ctrl->buffer);
}
				  const struct ov2722_reg *next)
{
	int size;
	u16 *data16;

	switch (next->type) {
	case OV2722_8BIT:
		size = 1;
		break;
	case OV2722_16BIT:
		size = 2;
		data16 = (u16 *)&ctrl->buffer.data[ctrl->index];
		*data16 = cpu_to_be16((u16)next->val);
		break;
	default:
		return -EINVAL;
	.g_volatile_ctrl = ov2722_g_volatile_ctrl
};

struct v4l2_ctrl_config ov2722_controls[] = {
	{
	 .ops = &ctrl_ops,
	 .id = V4L2_CID_EXPOSURE_ABSOLUTE,
	 .type = V4L2_CTRL_TYPE_INTEGER,
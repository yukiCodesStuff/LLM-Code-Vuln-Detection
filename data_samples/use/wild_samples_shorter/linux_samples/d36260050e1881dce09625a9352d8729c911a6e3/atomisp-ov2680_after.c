	if (data_length == OV2680_8BIT)
		*val = (u8)data[0];
	else if (data_length == OV2680_16BIT)
		*val = be16_to_cpu(*(__be16 *)&data[0]);
	else
		*val = be32_to_cpu(*(__be32 *)&data[0]);
	//dev_dbg(&client->dev,  "++++i2c read adr%x = %x\n", reg,*val);
	return 0;
}

{
	int ret;
	unsigned char data[4] = {0};
	__be16 *wreg = (void *)data;
	const u16 len = data_length + sizeof(u16); /* 16-bit address + data */

	if (data_length != OV2680_8BIT && data_length != OV2680_16BIT) {
		dev_err(&client->dev,
		data[2] = (u8)(val);
	} else {
		/* OV2680_16BIT */
		__be16 *wdata = (void *)&data[2];

		*wdata = cpu_to_be16(val);
	}

	ret = ov2680_i2c_write(client, len, data);
				    struct ov2680_write_ctrl *ctrl)
{
	u16 size;
	__be16 *data16 = (void *)&ctrl->buffer.addr;

	if (ctrl->index == 0)
		return 0;

	size = sizeof(u16) + ctrl->index; /* 16-bit address + data */
	*data16 = cpu_to_be16(ctrl->buffer.addr);
	ctrl->index = 0;

	return ov2680_i2c_write(client, size, (u8 *)&ctrl->buffer);
}
				  const struct ov2680_reg *next)
{
	int size;
	__be16 *data16;

	switch (next->type) {
	case OV2680_8BIT:
		size = 1;
		break;
	case OV2680_16BIT:
		size = 2;
		data16 = (void *)&ctrl->buffer.data[ctrl->index];
		*data16 = cpu_to_be16((u16)next->val);
		break;
	default:
		return -EINVAL;
	.g_volatile_ctrl = ov2680_g_volatile_ctrl
};

static const struct v4l2_ctrl_config ov2680_controls[] = {
	{
	 .ops = &ctrl_ops,
	 .id = V4L2_CID_EXPOSURE_ABSOLUTE,
	 .type = V4L2_CTRL_TYPE_INTEGER,
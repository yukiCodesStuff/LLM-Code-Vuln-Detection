	int num_msg;
	struct i2c_msg msg;
	unsigned char data[6] = {0};
	__be16 *wreg;
	int retry = 0;

	if (!client->adapter) {
		v4l2_err(client, "%s error, no client->adapter\n", __func__);
	msg.buf = data;

	/* high byte goes out first */
	wreg = (void *)data;
	*wreg = cpu_to_be16(reg);

	if (data_length == MISENSOR_8BIT) {
		data[2] = (u8)(val);
	} else if (data_length == MISENSOR_16BIT) {
		u16 *wdata = (void *)&data[2];

		*wdata = be16_to_cpu(*(__be16 *)&data[2]);
	} else {
		/* MISENSOR_32BIT */
		u32 *wdata = (void *)&data[2];

		*wdata = be32_to_cpu(*(__be32 *)&data[2]);
	}

	num_msg = i2c_transfer(client->adapter, &msg, 1);

	const int num_msg = 1;
	int ret;
	int retry = 0;
	__be16 *data16 = (void *)&ctrl->buffer.addr;

	if (ctrl->index == 0)
		return 0;

	msg.addr = client->addr;
	msg.flags = 0;
	msg.len = 2 + ctrl->index;
	*data16 = cpu_to_be16(ctrl->buffer.addr);
	msg.buf = (u8 *)&ctrl->buffer;

	ret = i2c_transfer(client->adapter, &msg, num_msg);
	if (ret != num_msg) {
				   struct mt9m114_write_ctrl *ctrl,
				   const struct misensor_reg *next)
{
	__be16 *data16;
	__be32 *data32;
	int err;

	/* Insufficient buffer? Let's flush and get more free space. */
	if (ctrl->index + next->length >= MT9M114_MAX_WRITE_BUF_SIZE) {
		ctrl->buffer.data[ctrl->index] = (u8)next->val;
		break;
	case MISENSOR_16BIT:
		data16 = (__be16 *)&ctrl->buffer.data[ctrl->index];
		*data16 = cpu_to_be16((u16)next->val);
		break;
	case MISENSOR_32BIT:
		data32 = (__be32 *)&ctrl->buffer.data[ctrl->index];
		*data32 = cpu_to_be32(next->val);
		break;
	default:
		return -EINVAL;
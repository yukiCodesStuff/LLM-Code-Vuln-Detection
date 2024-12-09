	.g_volatile_ctrl = gc0310_g_volatile_ctrl
};

struct v4l2_ctrl_config gc0310_controls[] = {
	{
	 .ops = &ctrl_ops,
	 .id = V4L2_CID_EXPOSURE_ABSOLUTE,
	 .type = V4L2_CTRL_TYPE_INTEGER,
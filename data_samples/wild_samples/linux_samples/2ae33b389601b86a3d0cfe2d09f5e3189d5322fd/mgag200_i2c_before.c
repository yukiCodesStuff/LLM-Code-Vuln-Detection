{
	struct mga_device *mdev = dev->dev_private;
	struct mga_i2c_chan *i2c;
	int ret;
	int data, clock;

	WREG_DAC(MGA1064_GEN_IO_DATA, 0xff);
	WREG_DAC(MGA1064_GEN_IO_CTL, 0);

	switch (mdev->type) {
	case G200_SE_A:
	case G200_SE_B:
	case G200_EV:
	case G200_WB:
		data = 1;
		clock = 2;
		break;
	case G200_EH:
	case G200_ER:
		data = 2;
		clock = 1;
		break;
	default:
		data = 2;
		clock = 8;
		break;
	}
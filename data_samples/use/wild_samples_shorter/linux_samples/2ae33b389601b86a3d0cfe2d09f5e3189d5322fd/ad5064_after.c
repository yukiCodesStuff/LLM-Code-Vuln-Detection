#define AD5064_ADDR(x)				((x) << 20)
#define AD5064_CMD(x)				((x) << 24)

#define AD5064_ADDR_ALL_DAC			0xF

#define AD5064_CMD_WRITE_INPUT_N		0x0
#define AD5064_CMD_UPDATE_DAC_N			0x1
}

static int ad5064_sync_powerdown_mode(struct ad5064_state *st,
	const struct iio_chan_spec *chan)
{
	unsigned int val;
	int ret;

	val = (0x1 << chan->address);

	if (st->pwr_down[chan->channel])
		val |= st->pwr_down_mode[chan->channel] << 8;

	ret = ad5064_write(st, AD5064_CMD_POWERDOWN_DAC, 0, val, 0);

	return ret;
	mutex_lock(&indio_dev->mlock);
	st->pwr_down_mode[chan->channel] = mode + 1;

	ret = ad5064_sync_powerdown_mode(st, chan);
	mutex_unlock(&indio_dev->mlock);

	return ret;
}
	mutex_lock(&indio_dev->mlock);
	st->pwr_down[chan->channel] = pwr_down;

	ret = ad5064_sync_powerdown_mode(st, chan);
	mutex_unlock(&indio_dev->mlock);
	return ret ? ret : len;
}


	switch (mask) {
	case IIO_CHAN_INFO_RAW:
		if (val >= (1 << chan->scan_type.realbits) || val < 0)
			return -EINVAL;

		mutex_lock(&indio_dev->mlock);
		ret = ad5064_write(st, AD5064_CMD_WRITE_INPUT_N_UPDATE_N,
	{ },
};

#define AD5064_CHANNEL(chan, addr, bits) {			\
	.type = IIO_VOLTAGE,					\
	.indexed = 1,						\
	.output = 1,						\
	.channel = (chan),					\
	.info_mask = IIO_CHAN_INFO_RAW_SEPARATE_BIT |		\
	IIO_CHAN_INFO_SCALE_SEPARATE_BIT,			\
	.address = addr,					\
	.scan_type = IIO_ST('u', (bits), 16, 20 - (bits)),	\
	.ext_info = ad5064_ext_info,				\
}

#define DECLARE_AD5064_CHANNELS(name, bits) \
const struct iio_chan_spec name[] = { \
	AD5064_CHANNEL(0, 0, bits), \
	AD5064_CHANNEL(1, 1, bits), \
	AD5064_CHANNEL(2, 2, bits), \
	AD5064_CHANNEL(3, 3, bits), \
	AD5064_CHANNEL(4, 4, bits), \
	AD5064_CHANNEL(5, 5, bits), \
	AD5064_CHANNEL(6, 6, bits), \
	AD5064_CHANNEL(7, 7, bits), \
}

#define DECLARE_AD5065_CHANNELS(name, bits) \
const struct iio_chan_spec name[] = { \
	AD5064_CHANNEL(0, 0, bits), \
	AD5064_CHANNEL(1, 3, bits), \
}

static DECLARE_AD5064_CHANNELS(ad5024_channels, 12);
static DECLARE_AD5064_CHANNELS(ad5044_channels, 14);
static DECLARE_AD5064_CHANNELS(ad5064_channels, 16);

static DECLARE_AD5065_CHANNELS(ad5025_channels, 12);
static DECLARE_AD5065_CHANNELS(ad5045_channels, 14);
static DECLARE_AD5065_CHANNELS(ad5065_channels, 16);

static const struct ad5064_chip_info ad5064_chip_info_tbl[] = {
	[ID_AD5024] = {
		.shared_vref = false,
		.channels = ad5024_channels,
	},
	[ID_AD5025] = {
		.shared_vref = false,
		.channels = ad5025_channels,
		.num_channels = 2,
	},
	[ID_AD5044] = {
		.shared_vref = false,
	},
	[ID_AD5045] = {
		.shared_vref = false,
		.channels = ad5045_channels,
		.num_channels = 2,
	},
	[ID_AD5064] = {
		.shared_vref = false,
	},
	[ID_AD5065] = {
		.shared_vref = false,
		.channels = ad5065_channels,
		.num_channels = 2,
	},
	[ID_AD5628_1] = {
		.shared_vref = true,
{
	struct iio_dev *indio_dev;
	struct ad5064_state *st;
	unsigned int midscale;
	unsigned int i;
	int ret;

	indio_dev = iio_device_alloc(sizeof(*st));
			goto error_free_reg;
	}

	indio_dev->dev.parent = dev;
	indio_dev->name = name;
	indio_dev->info = &ad5064_info;
	indio_dev->modes = INDIO_DIRECT_MODE;
	indio_dev->channels = st->chip_info->channels;
	indio_dev->num_channels = st->chip_info->num_channels;

	midscale = (1 << indio_dev->channels[0].scan_type.realbits) /  2;

	for (i = 0; i < st->chip_info->num_channels; ++i) {
		st->pwr_down_mode[i] = AD5064_LDAC_PWRDN_1K;
		st->dac_cache[i] = midscale;
	}

	ret = iio_device_register(indio_dev);
	if (ret)
		goto error_disable_reg;

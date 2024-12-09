	case IIO_CHAN_INFO_SCALE:
		switch (chan->type) {
		case IIO_TEMP:
			*val = 10;
			return IIO_VAL_INT;
		case IIO_PH:
			*val = 1; /* 0.001 */
			*val2 = 1000;
			break;
			   int val, int val2, long mask)
{
	struct atlas_data *data = iio_priv(indio_dev);
	__be32 reg = cpu_to_be32(val / 10);

	if (val2 != 0 || val < 0 || val > 20000)
		return -EINVAL;

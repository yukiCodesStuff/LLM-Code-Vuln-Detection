		switch (chan->type) {
		case IIO_TEMP:
			*val = 10;
			return IIO_VAL_INT;
		case IIO_PH:
			*val = 1; /* 0.001 */
			*val2 = 1000;
			break;
		case IIO_ELECTRICALCONDUCTIVITY:
			*val = 1; /* 0.00001 */
			*val2 = 100000;
			break;
		case IIO_CONCENTRATION:
			*val = 0; /* 0.000000001 */
			*val2 = 1000;
			return IIO_VAL_INT_PLUS_NANO;
		case IIO_VOLTAGE:
			*val = 1; /* 0.1 */
			*val2 = 10;
			break;
		default:
			return -EINVAL;
		}
{
	struct atlas_data *data = iio_priv(indio_dev);
	__be32 reg = cpu_to_be32(val / 10);

	if (val2 != 0 || val < 0 || val > 20000)
		return -EINVAL;

	if (mask != IIO_CHAN_INFO_RAW || chan->type != IIO_TEMP)
		return -EINVAL;

	return regmap_bulk_write(data->regmap, chan->address,
				 &reg, sizeof(reg));
}
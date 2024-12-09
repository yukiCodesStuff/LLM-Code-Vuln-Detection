int st_sensors_set_odr(struct iio_dev *indio_dev, unsigned int odr)
{
	int err;
	struct st_sensor_odr_avl odr_out;
	struct st_sensor_data *sdata = iio_priv(indio_dev);

	err = st_sensors_match_odr(sdata->sensor, odr, &odr_out);
	if (err < 0)

static int st_sensors_set_fullscale(struct iio_dev *indio_dev, unsigned int fs)
{
	int err, i;
	struct st_sensor_data *sdata = iio_priv(indio_dev);

	err = st_sensors_match_fs(sdata->sensor, fs, &i);
	if (err < 0)

int st_sensors_set_enable(struct iio_dev *indio_dev, bool enable)
{
	bool found;
	u8 tmp_value;
	int err = -EINVAL;
	struct st_sensor_odr_avl odr_out;
	struct st_sensor_data *sdata = iio_priv(indio_dev);

	if (enable) {
		found = false;
		tmp_value = sdata->sensor->pw.value_on;
		if ((sdata->sensor->odr.addr == sdata->sensor->pw.addr) &&
			(sdata->sensor->odr.mask == sdata->sensor->pw.mask)) {
			err = st_sensors_match_odr(sdata->sensor,
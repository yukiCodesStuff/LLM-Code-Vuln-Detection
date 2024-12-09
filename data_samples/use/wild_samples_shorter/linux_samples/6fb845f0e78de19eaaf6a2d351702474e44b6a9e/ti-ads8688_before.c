
#define ADS8688_VREF_MV			4096
#define ADS8688_REALBITS		16

/*
 * enum ads8688_range - ADS8688 reference voltage range
 * @ADS8688_PLUSMINUS25VREF: Device is configured for input range ±2.5 * VREF
{
	struct iio_poll_func *pf = p;
	struct iio_dev *indio_dev = pf->indio_dev;
	u16 buffer[8];
	int i, j = 0;

	for (i = 0; i < indio_dev->masklength; i++) {
		if (!test_bit(i, indio_dev->active_scan_mask))
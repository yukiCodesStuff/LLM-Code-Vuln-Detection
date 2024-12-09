#include <linux/iio/machine.h>
#include <linux/iio/driver.h>

#define AXP288_ADC_EN_MASK		0xF1
#define AXP288_ADC_TS_PIN_GPADC		0xF2
#define AXP288_ADC_TS_PIN_ON		0xF3

enum axp288_adc_id {
	AXP288_ADC_TS,
	AXP288_ADC_PMIC,
struct axp288_adc_info {
	int irq;
	struct regmap *regmap;
};

static const struct iio_chan_spec axp288_adc_channels[] = {
	{
	return IIO_VAL_INT;
}

static int axp288_adc_set_ts(struct regmap *regmap, unsigned int mode,
				unsigned long address)
{
	int ret;

	/* channels other than GPADC do not need to switch TS pin */
	if (address != AXP288_GP_ADC_H)
		return 0;

	ret = regmap_write(regmap, AXP288_ADC_TS_PIN_CTRL, mode);
	if (ret)
		return ret;

	/* When switching to the GPADC pin give things some time to settle */
	if (mode == AXP288_ADC_TS_PIN_GPADC)
		usleep_range(6000, 10000);

	return 0;
}
	mutex_lock(&indio_dev->mlock);
	switch (mask) {
	case IIO_CHAN_INFO_RAW:
		if (axp288_adc_set_ts(info->regmap, AXP288_ADC_TS_PIN_GPADC,
					chan->address)) {
			dev_err(&indio_dev->dev, "GPADC mode\n");
			ret = -EINVAL;
			break;
		}
		ret = axp288_adc_read_channel(val, chan->address, info->regmap);
		if (axp288_adc_set_ts(info->regmap, AXP288_ADC_TS_PIN_ON,
						chan->address))
			dev_err(&indio_dev->dev, "TS pin restore\n");
		break;
	default:
	return ret;
}

static int axp288_adc_set_state(struct regmap *regmap)
{
	/* ADC should be always enabled for internal FG to function */
	if (regmap_write(regmap, AXP288_ADC_TS_PIN_CTRL, AXP288_ADC_TS_PIN_ON))
		return -EIO;

	return regmap_write(regmap, AXP20X_ADC_EN1, AXP288_ADC_EN_MASK);
}

static const struct iio_info axp288_adc_iio_info = {
	.read_raw = &axp288_adc_read_raw,
	 * Set ADC to enabled state at all time, including system suspend.
	 * otherwise internal fuel gauge functionality may be affected.
	 */
	ret = axp288_adc_set_state(axp20x->regmap);
	if (ret) {
		dev_err(&pdev->dev, "unable to enable ADC device\n");
		return ret;
	}
#include <linux/iio/machine.h>
#include <linux/iio/driver.h>

/*
 * This mask enables all ADCs except for the battery temp-sensor (TS), that is
 * left as-is to avoid breaking charging on devices without a temp-sensor.
 */
#define AXP288_ADC_EN_MASK				0xF0
#define AXP288_ADC_TS_ENABLE				0x01

#define AXP288_ADC_TS_CURRENT_ON_OFF_MASK		GENMASK(1, 0)
#define AXP288_ADC_TS_CURRENT_OFF			(0 << 0)
#define AXP288_ADC_TS_CURRENT_ON_WHEN_CHARGING		(1 << 0)
#define AXP288_ADC_TS_CURRENT_ON_ONDEMAND		(2 << 0)
#define AXP288_ADC_TS_CURRENT_ON			(3 << 0)

enum axp288_adc_id {
	AXP288_ADC_TS,
	AXP288_ADC_PMIC,
struct axp288_adc_info {
	int irq;
	struct regmap *regmap;
	bool ts_enabled;
};

static const struct iio_chan_spec axp288_adc_channels[] = {
	{
	return IIO_VAL_INT;
}

/*
 * The current-source used for the battery temp-sensor (TS) is shared
 * with the GPADC. For proper fuel-gauge and charger operation the TS
 * current-source needs to be permanently on. But to read the GPADC we
 * need to temporary switch the TS current-source to ondemand, so that
 * the GPADC can use it, otherwise we will always read an all 0 value.
 */
static int axp288_adc_set_ts(struct axp288_adc_info *info,
			     unsigned int mode, unsigned long address)
{
	int ret;

	/* No need to switch the current-source if the TS pin is disabled */
	if (!info->ts_enabled)
		return 0;

	/* Channels other than GPADC do not need the current source */
	if (address != AXP288_GP_ADC_H)
		return 0;

	ret = regmap_update_bits(info->regmap, AXP288_ADC_TS_PIN_CTRL,
				 AXP288_ADC_TS_CURRENT_ON_OFF_MASK, mode);
	if (ret)
		return ret;

	/* When switching to the GPADC pin give things some time to settle */
	if (mode == AXP288_ADC_TS_CURRENT_ON_ONDEMAND)
		usleep_range(6000, 10000);

	return 0;
}
	mutex_lock(&indio_dev->mlock);
	switch (mask) {
	case IIO_CHAN_INFO_RAW:
		if (axp288_adc_set_ts(info, AXP288_ADC_TS_CURRENT_ON_ONDEMAND,
					chan->address)) {
			dev_err(&indio_dev->dev, "GPADC mode\n");
			ret = -EINVAL;
			break;
		}
		ret = axp288_adc_read_channel(val, chan->address, info->regmap);
		if (axp288_adc_set_ts(info, AXP288_ADC_TS_CURRENT_ON,
						chan->address))
			dev_err(&indio_dev->dev, "TS pin restore\n");
		break;
	default:
	return ret;
}

static int axp288_adc_initialize(struct axp288_adc_info *info)
{
	int ret, adc_enable_val;

	/*
	 * Determine if the TS pin is enabled and set the TS current-source
	 * accordingly.
	 */
	ret = regmap_read(info->regmap, AXP20X_ADC_EN1, &adc_enable_val);
	if (ret)
		return ret;

	if (adc_enable_val & AXP288_ADC_TS_ENABLE) {
		info->ts_enabled = true;
		ret = regmap_update_bits(info->regmap, AXP288_ADC_TS_PIN_CTRL,
					 AXP288_ADC_TS_CURRENT_ON_OFF_MASK,
					 AXP288_ADC_TS_CURRENT_ON);
	} else {
		info->ts_enabled = false;
		ret = regmap_update_bits(info->regmap, AXP288_ADC_TS_PIN_CTRL,
					 AXP288_ADC_TS_CURRENT_ON_OFF_MASK,
					 AXP288_ADC_TS_CURRENT_OFF);
	}
	if (ret)
		return ret;

	/* Turn on the ADC for all channels except TS, leave TS as is */
	return regmap_update_bits(info->regmap, AXP20X_ADC_EN1,
				  AXP288_ADC_EN_MASK, AXP288_ADC_EN_MASK);
}

static const struct iio_info axp288_adc_iio_info = {
	.read_raw = &axp288_adc_read_raw,
	 * Set ADC to enabled state at all time, including system suspend.
	 * otherwise internal fuel gauge functionality may be affected.
	 */
	ret = axp288_adc_initialize(info);
	if (ret) {
		dev_err(&pdev->dev, "unable to enable ADC device\n");
		return ret;
	}
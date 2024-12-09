{
	sun6i_rtc_clk_init(node, &sun50i_h6_rtc_data);
}
CLK_OF_DECLARE_DRIVER(sun50i_h6_rtc_clk, "allwinner,sun50i-h6-rtc",
		      sun50i_h6_rtc_clk_init);

/*
 * The R40 user manual is self-conflicting on whether the prescaler is
 * fixed or configurable. The clock diagram shows it as fixed, but there
 * is also a configurable divider in the RTC block.
 */
static const struct sun6i_rtc_clk_data sun8i_r40_rtc_data = {
	.rc_osc_rate = 16000000,
	.fixed_prescaler = 512,
};
static void __init sun8i_r40_rtc_clk_init(struct device_node *node)
{
	sun6i_rtc_clk_init(node, &sun8i_r40_rtc_data);
}
CLK_OF_DECLARE_DRIVER(sun8i_r40_rtc_clk, "allwinner,sun8i-r40-rtc",
		      sun8i_r40_rtc_clk_init);

static const struct sun6i_rtc_clk_data sun8i_v3_rtc_data = {
	.rc_osc_rate = 32000,
	.has_out_clk = 1,
};

static void __init sun8i_v3_rtc_clk_init(struct device_node *node)
{
	sun6i_rtc_clk_init(node, &sun8i_v3_rtc_data);
}
CLK_OF_DECLARE_DRIVER(sun8i_v3_rtc_clk, "allwinner,sun8i-v3-rtc",
		      sun8i_v3_rtc_clk_init);

static irqreturn_t sun6i_rtc_alarmirq(int irq, void *id)
{
	struct sun6i_rtc_dev *chip = (struct sun6i_rtc_dev *) id;
	irqreturn_t ret = IRQ_NONE;
	u32 val;

	spin_lock(&chip->lock);
	val = readl(chip->base + SUN6I_ALRM_IRQ_STA);

	if (val & SUN6I_ALRM_IRQ_STA_CNT_IRQ_PEND) {
		val |= SUN6I_ALRM_IRQ_STA_CNT_IRQ_PEND;
		writel(val, chip->base + SUN6I_ALRM_IRQ_STA);

		rtc_update_irq(chip->rtc, 1, RTC_AF | RTC_IRQF);

		ret = IRQ_HANDLED;
	}
	spin_unlock(&chip->lock);

	return ret;
}

static void sun6i_rtc_setaie(int to, struct sun6i_rtc_dev *chip)
{
	u32 alrm_val = 0;
	u32 alrm_irq_val = 0;
	u32 alrm_wake_val = 0;
	unsigned long flags;

	if (to) {
		alrm_val = SUN6I_ALRM_EN_CNT_EN;
		alrm_irq_val = SUN6I_ALRM_IRQ_EN_CNT_IRQ_EN;
		alrm_wake_val = SUN6I_ALARM_CONFIG_WAKEUP;
	} else {
		writel(SUN6I_ALRM_IRQ_STA_CNT_IRQ_PEND,
		       chip->base + SUN6I_ALRM_IRQ_STA);
	}

	spin_lock_irqsave(&chip->lock, flags);
	writel(alrm_val, chip->base + SUN6I_ALRM_EN);
	writel(alrm_irq_val, chip->base + SUN6I_ALRM_IRQ_EN);
	writel(alrm_wake_val, chip->base + SUN6I_ALARM_CONFIG);
	spin_unlock_irqrestore(&chip->lock, flags);
}

static int sun6i_rtc_gettime(struct device *dev, struct rtc_time *rtc_tm)
{
	struct sun6i_rtc_dev *chip = dev_get_drvdata(dev);
	u32 date, time;

	/*
	 * read again in case it changes
	 */
	do {
		date = readl(chip->base + SUN6I_RTC_YMD);
		time = readl(chip->base + SUN6I_RTC_HMS);
	} while ((date != readl(chip->base + SUN6I_RTC_YMD)) ||
		 (time != readl(chip->base + SUN6I_RTC_HMS)));

	rtc_tm->tm_sec  = SUN6I_TIME_GET_SEC_VALUE(time);
	rtc_tm->tm_min  = SUN6I_TIME_GET_MIN_VALUE(time);
	rtc_tm->tm_hour = SUN6I_TIME_GET_HOUR_VALUE(time);

	rtc_tm->tm_mday = SUN6I_DATE_GET_DAY_VALUE(date);
	rtc_tm->tm_mon  = SUN6I_DATE_GET_MON_VALUE(date);
	rtc_tm->tm_year = SUN6I_DATE_GET_YEAR_VALUE(date);

	rtc_tm->tm_mon  -= 1;

	/*
	 * switch from (data_year->min)-relative offset to
	 * a (1900)-relative one
	 */
	rtc_tm->tm_year += SUN6I_YEAR_OFF;

	return 0;
}

static int sun6i_rtc_getalarm(struct device *dev, struct rtc_wkalrm *wkalrm)
{
	struct sun6i_rtc_dev *chip = dev_get_drvdata(dev);
	unsigned long flags;
	u32 alrm_st;
	u32 alrm_en;

	spin_lock_irqsave(&chip->lock, flags);
	alrm_en = readl(chip->base + SUN6I_ALRM_IRQ_EN);
	alrm_st = readl(chip->base + SUN6I_ALRM_IRQ_STA);
	spin_unlock_irqrestore(&chip->lock, flags);

	wkalrm->enabled = !!(alrm_en & SUN6I_ALRM_EN_CNT_EN);
	wkalrm->pending = !!(alrm_st & SUN6I_ALRM_EN_CNT_EN);
	rtc_time_to_tm(chip->alarm, &wkalrm->time);

	return 0;
}

static int sun6i_rtc_setalarm(struct device *dev, struct rtc_wkalrm *wkalrm)
{
	struct sun6i_rtc_dev *chip = dev_get_drvdata(dev);
	struct rtc_time *alrm_tm = &wkalrm->time;
	struct rtc_time tm_now;
	unsigned long time_now = 0;
	unsigned long time_set = 0;
	unsigned long time_gap = 0;
	int ret = 0;

	ret = sun6i_rtc_gettime(dev, &tm_now);
	if (ret < 0) {
		dev_err(dev, "Error in getting time\n");
		return -EINVAL;
	}

	rtc_tm_to_time(alrm_tm, &time_set);
	rtc_tm_to_time(&tm_now, &time_now);
	if (time_set <= time_now) {
		dev_err(dev, "Date to set in the past\n");
		return -EINVAL;
	}

	time_gap = time_set - time_now;

	if (time_gap > U32_MAX) {
		dev_err(dev, "Date too far in the future\n");
		return -EINVAL;
	}

	sun6i_rtc_setaie(0, chip);
	writel(0, chip->base + SUN6I_ALRM_COUNTER);
	usleep_range(100, 300);

	writel(time_gap, chip->base + SUN6I_ALRM_COUNTER);
	chip->alarm = time_set;

	sun6i_rtc_setaie(wkalrm->enabled, chip);

	return 0;
}

static int sun6i_rtc_wait(struct sun6i_rtc_dev *chip, int offset,
			  unsigned int mask, unsigned int ms_timeout)
{
	const unsigned long timeout = jiffies + msecs_to_jiffies(ms_timeout);
	u32 reg;

	do {
		reg = readl(chip->base + offset);
		reg &= mask;

		if (!reg)
			return 0;

	} while (time_before(jiffies, timeout));

	return -ETIMEDOUT;
}

static int sun6i_rtc_settime(struct device *dev, struct rtc_time *rtc_tm)
{
	struct sun6i_rtc_dev *chip = dev_get_drvdata(dev);
	u32 date = 0;
	u32 time = 0;
	int year;

	year = rtc_tm->tm_year + 1900;
	if (year < SUN6I_YEAR_MIN || year > SUN6I_YEAR_MAX) {
		dev_err(dev, "rtc only supports year in range %d - %d\n",
			SUN6I_YEAR_MIN, SUN6I_YEAR_MAX);
		return -EINVAL;
	}

	rtc_tm->tm_year -= SUN6I_YEAR_OFF;
	rtc_tm->tm_mon += 1;

	date = SUN6I_DATE_SET_DAY_VALUE(rtc_tm->tm_mday) |
		SUN6I_DATE_SET_MON_VALUE(rtc_tm->tm_mon)  |
		SUN6I_DATE_SET_YEAR_VALUE(rtc_tm->tm_year);

	if (is_leap_year(year))
		date |= SUN6I_LEAP_SET_VALUE(1);

	time = SUN6I_TIME_SET_SEC_VALUE(rtc_tm->tm_sec)  |
		SUN6I_TIME_SET_MIN_VALUE(rtc_tm->tm_min)  |
		SUN6I_TIME_SET_HOUR_VALUE(rtc_tm->tm_hour);

	/* Check whether registers are writable */
	if (sun6i_rtc_wait(chip, SUN6I_LOSC_CTRL,
			   SUN6I_LOSC_CTRL_ACC_MASK, 50)) {
		dev_err(dev, "rtc is still busy.\n");
		return -EBUSY;
	}

	writel(time, chip->base + SUN6I_RTC_HMS);

	/*
	 * After writing the RTC HH-MM-SS register, the
	 * SUN6I_LOSC_CTRL_RTC_HMS_ACC bit is set and it will not
	 * be cleared until the real writing operation is finished
	 */

	if (sun6i_rtc_wait(chip, SUN6I_LOSC_CTRL,
			   SUN6I_LOSC_CTRL_RTC_HMS_ACC, 50)) {
		dev_err(dev, "Failed to set rtc time.\n");
		return -ETIMEDOUT;
	}

	writel(date, chip->base + SUN6I_RTC_YMD);

	/*
	 * After writing the RTC YY-MM-DD register, the
	 * SUN6I_LOSC_CTRL_RTC_YMD_ACC bit is set and it will not
	 * be cleared until the real writing operation is finished
	 */

	if (sun6i_rtc_wait(chip, SUN6I_LOSC_CTRL,
			   SUN6I_LOSC_CTRL_RTC_YMD_ACC, 50)) {
		dev_err(dev, "Failed to set rtc time.\n");
		return -ETIMEDOUT;
	}

	return 0;
}

static int sun6i_rtc_alarm_irq_enable(struct device *dev, unsigned int enabled)
{
	struct sun6i_rtc_dev *chip = dev_get_drvdata(dev);

	if (!enabled)
		sun6i_rtc_setaie(enabled, chip);

	return 0;
}

static const struct rtc_class_ops sun6i_rtc_ops = {
	.read_time		= sun6i_rtc_gettime,
	.set_time		= sun6i_rtc_settime,
	.read_alarm		= sun6i_rtc_getalarm,
	.set_alarm		= sun6i_rtc_setalarm,
	.alarm_irq_enable	= sun6i_rtc_alarm_irq_enable
};

#ifdef CONFIG_PM_SLEEP
/* Enable IRQ wake on suspend, to wake up from RTC. */
static int sun6i_rtc_suspend(struct device *dev)
{
	struct sun6i_rtc_dev *chip = dev_get_drvdata(dev);

	if (device_may_wakeup(dev))
		enable_irq_wake(chip->irq);

	return 0;
}

/* Disable IRQ wake on resume. */
static int sun6i_rtc_resume(struct device *dev)
{
	struct sun6i_rtc_dev *chip = dev_get_drvdata(dev);

	if (device_may_wakeup(dev))
		disable_irq_wake(chip->irq);

	return 0;
}
#endif

static SIMPLE_DEV_PM_OPS(sun6i_rtc_pm_ops,
	sun6i_rtc_suspend, sun6i_rtc_resume);

static int sun6i_rtc_probe(struct platform_device *pdev)
{
	struct sun6i_rtc_dev *chip = sun6i_rtc;
	int ret;

	if (!chip)
		return -ENODEV;

	platform_set_drvdata(pdev, chip);

	chip->irq = platform_get_irq(pdev, 0);
	if (chip->irq < 0)
		return chip->irq;

	ret = devm_request_irq(&pdev->dev, chip->irq, sun6i_rtc_alarmirq,
			       0, dev_name(&pdev->dev), chip);
	if (ret) {
		dev_err(&pdev->dev, "Could not request IRQ\n");
		return ret;
	}

	/* clear the alarm counter value */
	writel(0, chip->base + SUN6I_ALRM_COUNTER);

	/* disable counter alarm */
	writel(0, chip->base + SUN6I_ALRM_EN);

	/* disable counter alarm interrupt */
	writel(0, chip->base + SUN6I_ALRM_IRQ_EN);

	/* disable week alarm */
	writel(0, chip->base + SUN6I_ALRM1_EN);

	/* disable week alarm interrupt */
	writel(0, chip->base + SUN6I_ALRM1_IRQ_EN);

	/* clear counter alarm pending interrupts */
	writel(SUN6I_ALRM_IRQ_STA_CNT_IRQ_PEND,
	       chip->base + SUN6I_ALRM_IRQ_STA);

	/* clear week alarm pending interrupts */
	writel(SUN6I_ALRM1_IRQ_STA_WEEK_IRQ_PEND,
	       chip->base + SUN6I_ALRM1_IRQ_STA);

	/* disable alarm wakeup */
	writel(0, chip->base + SUN6I_ALARM_CONFIG);

	clk_prepare_enable(chip->losc);

	device_init_wakeup(&pdev->dev, 1);

	chip->rtc = devm_rtc_device_register(&pdev->dev, "rtc-sun6i",
					     &sun6i_rtc_ops, THIS_MODULE);
	if (IS_ERR(chip->rtc)) {
		dev_err(&pdev->dev, "unable to register device\n");
		return PTR_ERR(chip->rtc);
	}

	dev_info(&pdev->dev, "RTC enabled\n");

	return 0;
}

/*
 * As far as RTC functionality goes, all models are the same. The
 * datasheets claim that different models have different number of
 * registers available for non-volatile storage, but experiments show
 * that all SoCs have 16 registers available for this purpose.
 */
static const struct of_device_id sun6i_rtc_dt_ids[] = {
	{ .compatible = "allwinner,sun6i-a31-rtc" },
	{ .compatible = "allwinner,sun8i-a23-rtc" },
	{ .compatible = "allwinner,sun8i-h3-rtc" },
	{ .compatible = "allwinner,sun8i-r40-rtc" },
	{ .compatible = "allwinner,sun8i-v3-rtc" },
	{ .compatible = "allwinner,sun50i-h5-rtc" },
	{ .compatible = "allwinner,sun50i-h6-rtc" },
	{ /* sentinel */ },
};
MODULE_DEVICE_TABLE(of, sun6i_rtc_dt_ids);

static struct platform_driver sun6i_rtc_driver = {
	.probe		= sun6i_rtc_probe,
	.driver		= {
		.name		= "sun6i-rtc",
		.of_match_table = sun6i_rtc_dt_ids,
		.pm = &sun6i_rtc_pm_ops,
	},
};
builtin_platform_driver(sun6i_rtc_driver);
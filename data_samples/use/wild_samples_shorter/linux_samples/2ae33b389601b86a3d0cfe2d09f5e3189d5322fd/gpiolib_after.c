static void gpiod_free(struct gpio_desc *desc);
static int gpiod_direction_input(struct gpio_desc *desc);
static int gpiod_direction_output(struct gpio_desc *desc, int value);
static int gpiod_get_direction(const struct gpio_desc *desc);
static int gpiod_set_debounce(struct gpio_desc *desc, unsigned debounce);
static int gpiod_get_value_cansleep(const struct gpio_desc *desc);
static void gpiod_set_value_cansleep(struct gpio_desc *desc, int value);
static int gpiod_get_value(const struct gpio_desc *desc);
static void gpiod_set_value(struct gpio_desc *desc, int value);
static int gpiod_cansleep(const struct gpio_desc *desc);
static int gpiod_to_irq(const struct gpio_desc *desc);
static int gpiod_export(struct gpio_desc *desc, bool direction_may_change);
static int gpiod_export_link(struct device *dev, const char *name,
			     struct gpio_desc *desc);
static int gpiod_sysfs_set_active_low(struct gpio_desc *desc, int value);
	return 0;
}

static struct gpio_chip *gpiod_to_chip(const struct gpio_desc *desc)
{
	return desc ? desc->chip : NULL;
}

/* caller holds gpio_lock *OR* gpio is marked as requested */
struct gpio_chip *gpio_to_chip(unsigned gpio)
{
	return gpiod_to_chip(gpio_to_desc(gpio));
}
}

/* caller ensures gpio is valid and requested, chip->get_direction may sleep  */
static int gpiod_get_direction(const struct gpio_desc *desc)
{
	struct gpio_chip	*chip;
	unsigned		offset;
	int			status = -EINVAL;
	if (status > 0) {
		/* GPIOF_DIR_IN, or other positive */
		status = 1;
		/* FLAG_IS_OUT is just a cache of the result of get_direction(),
		 * so it does not affect constness per se */
		clear_bit(FLAG_IS_OUT, &((struct gpio_desc *)desc)->flags);
	}
	if (status == 0) {
		/* GPIOF_DIR_OUT */
		set_bit(FLAG_IS_OUT, &((struct gpio_desc *)desc)->flags);
	}
	return status;
}

static ssize_t gpio_direction_show(struct device *dev,
		struct device_attribute *attr, char *buf)
{
	const struct gpio_desc	*desc = dev_get_drvdata(dev);
	ssize_t			status;

	mutex_lock(&sysfs_lock);

		goto done;

	desc = gpio_to_desc(gpio);
	/* reject invalid GPIOs */
	if (!desc) {
		pr_warn("%s: invalid GPIO %ld\n", __func__, gpio);
		return -EINVAL;
	}

	/* No extra locking here; FLAG_SYSFS just signifies that the
	 * request and export were done by on behalf of userspace, so
	 * they may be undone on its behalf too.
	if (status < 0)
		goto done;

	desc = gpio_to_desc(gpio);
	/* reject bogus commands (gpio_unexport ignores them) */
	if (!desc) {
		pr_warn("%s: invalid GPIO %ld\n", __func__, gpio);
		return -EINVAL;
	}

	status = -EINVAL;

	/* No extra locking here; FLAG_SYSFS just signifies that the
	 * request and export were done by on behalf of userspace, so
	 * they may be undone on its behalf too.
{
	int			status = -EINVAL;

	if (!desc) {
		pr_warn("%s: invalid GPIO\n", __func__);
		return -EINVAL;
	}

	mutex_lock(&sysfs_lock);

	if (test_bit(FLAG_EXPORT, &desc->flags)) {

	mutex_unlock(&sysfs_lock);

	if (status)
		pr_debug("%s: gpio%d status %d\n", __func__, desc_to_gpio(desc),
			 status);

	struct device		*dev = NULL;
	int			status = -EINVAL;

	if (!desc) {
		pr_warn("%s: invalid GPIO\n", __func__);
		return -EINVAL;
	}

	mutex_lock(&sysfs_lock);

	if (test_bit(FLAG_EXPORT, &desc->flags)) {
unlock:
	mutex_unlock(&sysfs_lock);

	if (status)
		pr_debug("%s: gpio%d status %d\n", __func__, desc_to_gpio(desc),
			 status);

	struct device		*dev = NULL;

	if (!desc) {
		pr_warn("%s: invalid GPIO\n", __func__);
		return;
	}

	mutex_lock(&sysfs_lock);

		device_unregister(dev);
		put_device(dev);
	}

	if (status)
		pr_debug("%s: gpio%d status %d\n", __func__, desc_to_gpio(desc),
			 status);
}
	int			status = -EPROBE_DEFER;
	unsigned long		flags;

	if (!desc) {
		pr_warn("%s: invalid GPIO\n", __func__);
		return -EINVAL;
	}

	spin_lock_irqsave(&gpio_lock, flags);

	chip = desc->chip;
	if (chip == NULL)
		goto done;

done:
	if (status)
		pr_debug("_gpio_request: gpio-%d (%s) status %d\n",
			 desc_to_gpio(desc), label ? : "?", status);
	spin_unlock_irqrestore(&gpio_lock, flags);
	return status;
}

	int			status = -EINVAL;
	int			offset;

	if (!desc) {
		pr_warn("%s: invalid GPIO\n", __func__);
		return -EINVAL;
	}

	spin_lock_irqsave(&gpio_lock, flags);

	chip = desc->chip;
	if (!chip || !chip->get || !chip->direction_input)
		goto fail;
	status = gpio_ensure_requested(desc);
	return status;
fail:
	spin_unlock_irqrestore(&gpio_lock, flags);
	if (status)
		pr_debug("%s: gpio-%d status %d\n", __func__,
			 desc_to_gpio(desc), status);
	return status;
}

int gpio_direction_input(unsigned gpio)
	int			status = -EINVAL;
	int offset;

	if (!desc) {
		pr_warn("%s: invalid GPIO\n", __func__);
		return -EINVAL;
	}

	/* Open drain pin should not be driven to 1 */
	if (value && test_bit(FLAG_OPEN_DRAIN,  &desc->flags))
		return gpiod_direction_input(desc);


	spin_lock_irqsave(&gpio_lock, flags);

	chip = desc->chip;
	if (!chip || !chip->set || !chip->direction_output)
		goto fail;
	status = gpio_ensure_requested(desc);
	return status;
fail:
	spin_unlock_irqrestore(&gpio_lock, flags);
	if (status)
		pr_debug("%s: gpio-%d status %d\n", __func__,
			 desc_to_gpio(desc), status);
	return status;
}

int gpio_direction_output(unsigned gpio, int value)
	int			status = -EINVAL;
	int			offset;

	if (!desc) {
		pr_warn("%s: invalid GPIO\n", __func__);
		return -EINVAL;
	}

	spin_lock_irqsave(&gpio_lock, flags);

	chip = desc->chip;
	if (!chip || !chip->set || !chip->set_debounce)
		goto fail;


fail:
	spin_unlock_irqrestore(&gpio_lock, flags);
	if (status)
		pr_debug("%s: gpio-%d status %d\n", __func__,
			 desc_to_gpio(desc), status);

	return status;
}

 * It returns the zero or nonzero value provided by the associated
 * gpio_chip.get() method; or zero if no such method is provided.
 */
static int gpiod_get_value(const struct gpio_desc *desc)
{
	struct gpio_chip	*chip;
	int value;
	int offset;

	if (!desc)
		return 0;
	chip = desc->chip;
	offset = gpio_chip_hwgpio(desc);
	/* Should be using gpio_get_value_cansleep() */
	WARN_ON(chip->can_sleep);
{
	struct gpio_chip	*chip;

	if (!desc)
		return;
	chip = desc->chip;
	/* Should be using gpio_set_value_cansleep() */
	WARN_ON(chip->can_sleep);
	trace_gpio_value(desc_to_gpio(desc), 0, value);
 * This is used directly or indirectly to implement gpio_cansleep().  It
 * returns nonzero if access reading or writing the GPIO value can sleep.
 */
static int gpiod_cansleep(const struct gpio_desc *desc)
{
	if (!desc)
		return 0;
	/* only call this on GPIOs that are valid! */
	return desc->chip->can_sleep;
}

 * It returns the number of the IRQ signaled by this (input) GPIO,
 * or a negative errno.
 */
static int gpiod_to_irq(const struct gpio_desc *desc)
{
	struct gpio_chip	*chip;
	int			offset;

	if (!desc)
		return -EINVAL;
	chip = desc->chip;
	offset = gpio_chip_hwgpio(desc);
	return chip->to_irq ? chip->to_irq(chip, offset) : -ENXIO;
}
 * Common examples include ones connected to I2C or SPI chips.
 */

static int gpiod_get_value_cansleep(const struct gpio_desc *desc)
{
	struct gpio_chip	*chip;
	int value;
	int offset;

	might_sleep_if(extra_checks);
	if (!desc)
		return 0;
	chip = desc->chip;
	offset = gpio_chip_hwgpio(desc);
	value = chip->get ? chip->get(chip, offset) : 0;
	trace_gpio_value(desc_to_gpio(desc), 1, value);
	struct gpio_chip	*chip;

	might_sleep_if(extra_checks);
	if (!desc)
		return;
	chip = desc->chip;
	trace_gpio_value(desc_to_gpio(desc), 0, value);
	if (test_bit(FLAG_OPEN_DRAIN,  &desc->flags))
		_gpio_set_open_drain_value(desc, value);
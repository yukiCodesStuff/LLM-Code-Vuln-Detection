		if (status == 0) {
			gpiod_set_value_cansleep(desc, value);
			status = size;
		}
	}

	mutex_unlock(&sysfs_lock);
	return status;
}

static const DEVICE_ATTR(value, 0644,
		gpio_value_show, gpio_value_store);

static irqreturn_t gpio_sysfs_irq(int irq, void *priv)
{
	struct kernfs_node	*value_sd = priv;

	sysfs_notify_dirent(value_sd);
	return IRQ_HANDLED;
}

static int gpio_setup_irq(struct gpio_desc *desc, struct device *dev,
		unsigned long gpio_flags)
{
	struct kernfs_node	*value_sd;
	unsigned long		irq_flags;
	int			ret, irq, id;

	if ((desc->flags & GPIO_TRIGGER_MASK) == gpio_flags)
		return 0;

	irq = gpiod_to_irq(desc);
	if (irq < 0)
		return -EIO;

	id = desc->flags >> ID_SHIFT;
	value_sd = idr_find(&dirent_idr, id);
	if (value_sd)
		free_irq(irq, value_sd);

	desc->flags &= ~GPIO_TRIGGER_MASK;

	if (!gpio_flags) {
		gpiochip_unlock_as_irq(desc->chip, gpio_chip_hwgpio(desc));
		ret = 0;
		goto free_id;
	}
	} else {
		long		value;

		status = kstrtol(buf, 0, &value);
		if (status == 0)
			status = sysfs_set_active_low(desc, dev, value != 0);
	}

	mutex_unlock(&sysfs_lock);

	return status ? : size;
}

static const DEVICE_ATTR(active_low, 0644,
		gpio_active_low_show, gpio_active_low_store);

static const struct attribute *gpio_attrs[] = {
	&dev_attr_value.attr,
	&dev_attr_active_low.attr,
	NULL,
};

static const struct attribute_group gpio_attr_group = {
	.attrs = (struct attribute **) gpio_attrs,
};

/*
 * /sys/class/gpio/gpiochipN/
 *   /base ... matching gpio_chip.base (N)
 *   /label ... matching gpio_chip.label
 *   /ngpio ... matching gpio_chip.ngpio
 */

static ssize_t chip_base_show(struct device *dev,
			       struct device_attribute *attr, char *buf)
{
	const struct gpio_chip	*chip = dev_get_drvdata(dev);

	return sprintf(buf, "%d\n", chip->base);
}
static DEVICE_ATTR(base, 0444, chip_base_show, NULL);

static ssize_t chip_label_show(struct device *dev,
			       struct device_attribute *attr, char *buf)
{
	const struct gpio_chip	*chip = dev_get_drvdata(dev);

	return sprintf(buf, "%s\n", chip->label ? : "");
}
static DEVICE_ATTR(label, 0444, chip_label_show, NULL);

static ssize_t chip_ngpio_show(struct device *dev,
			       struct device_attribute *attr, char *buf)
{
	const struct gpio_chip	*chip = dev_get_drvdata(dev);

	return sprintf(buf, "%u\n", chip->ngpio);
}
static DEVICE_ATTR(ngpio, 0444, chip_ngpio_show, NULL);

static const struct attribute *gpiochip_attrs[] = {
	&dev_attr_base.attr,
	&dev_attr_label.attr,
	&dev_attr_ngpio.attr,
	NULL,
};

static const struct attribute_group gpiochip_attr_group = {
	.attrs = (struct attribute **) gpiochip_attrs,
};

/*
 * /sys/class/gpio/export ... write-only
 *	integer N ... number of GPIO to export (full access)
 * /sys/class/gpio/unexport ... write-only
 *	integer N ... number of GPIO to unexport
 */
static ssize_t export_store(struct class *class,
				struct class_attribute *attr,
				const char *buf, size_t len)
{
	long			gpio;
	struct gpio_desc	*desc;
	int			status;

	status = kstrtol(buf, 0, &gpio);
	if (status < 0)
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
	 */

	status = gpiod_request(desc, "sysfs");
	if (status < 0) {
		if (status == -EPROBE_DEFER)
			status = -ENODEV;
		goto done;
	}
	status = gpiod_export(desc, true);
	if (status < 0)
		gpiod_free(desc);
	else
		set_bit(FLAG_SYSFS, &desc->flags);

done:
	if (status)
		pr_debug("%s: status %d\n", __func__, status);
	return status ? : len;
}

static ssize_t unexport_store(struct class *class,
				struct class_attribute *attr,
				const char *buf, size_t len)
{
	long			gpio;
	struct gpio_desc	*desc;
	int			status;

	status = kstrtol(buf, 0, &gpio);
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
	 */
	if (test_and_clear_bit(FLAG_SYSFS, &desc->flags)) {
		status = 0;
		gpiod_free(desc);
	}
done:
	if (status)
		pr_debug("%s: status %d\n", __func__, status);
	return status ? : len;
}

static struct class_attribute gpio_class_attrs[] = {
	__ATTR(export, 0200, NULL, export_store),
	__ATTR(unexport, 0200, NULL, unexport_store),
	__ATTR_NULL,
};

static struct class gpio_class = {
	.name =		"gpio",
	.owner =	THIS_MODULE,

	.class_attrs =	gpio_class_attrs,
};


/**
 * gpiod_export - export a GPIO through sysfs
 * @gpio: gpio to make available, already requested
 * @direction_may_change: true if userspace may change gpio direction
 * Context: arch_initcall or later
 *
 * When drivers want to make a GPIO accessible to userspace after they
 * have requested it -- perhaps while debugging, or as part of their
 * public interface -- they may use this routine.  If the GPIO can
 * change direction (some can't) and the caller allows it, userspace
 * will see "direction" sysfs attribute which may be used to change
 * the gpio's direction.  A "value" attribute will always be provided.
 *
 * Returns zero on success, else an error.
 */
int gpiod_export(struct gpio_desc *desc, bool direction_may_change)
{
	unsigned long		flags;
	int			status;
	const char		*ioname = NULL;
	struct device		*dev;
	int			offset;

	/* can't export until sysfs is available ... */
	if (!gpio_class.p) {
		pr_debug("%s: called too early!\n", __func__);
		return -ENOENT;
	}

	if (!desc) {
		pr_debug("%s: invalid gpio descriptor\n", __func__);
		return -EINVAL;
	}

	mutex_lock(&sysfs_lock);

	spin_lock_irqsave(&gpio_lock, flags);
	if (!test_bit(FLAG_REQUESTED, &desc->flags) ||
	     test_bit(FLAG_EXPORT, &desc->flags)) {
		spin_unlock_irqrestore(&gpio_lock, flags);
		gpiod_dbg(desc, "%s: unavailable (requested=%d, exported=%d)\n",
				__func__,
				test_bit(FLAG_REQUESTED, &desc->flags),
				test_bit(FLAG_EXPORT, &desc->flags));
		status = -EPERM;
		goto fail_unlock;
	}

	if (!desc->chip->direction_input || !desc->chip->direction_output)
		direction_may_change = false;
	spin_unlock_irqrestore(&gpio_lock, flags);

	offset = gpio_chip_hwgpio(desc);
	if (desc->chip->names && desc->chip->names[offset])
		ioname = desc->chip->names[offset];

	dev = device_create(&gpio_class, desc->chip->dev, MKDEV(0, 0),
			    desc, ioname ? ioname : "gpio%u",
			    desc_to_gpio(desc));
	if (IS_ERR(dev)) {
		status = PTR_ERR(dev);
		goto fail_unlock;
	}

	status = sysfs_create_group(&dev->kobj, &gpio_attr_group);
	if (status)
		goto fail_unregister_device;

	if (direction_may_change) {
		status = device_create_file(dev, &dev_attr_direction);
		if (status)
			goto fail_unregister_device;
	}

	if (gpiod_to_irq(desc) >= 0 && (direction_may_change ||
				       !test_bit(FLAG_IS_OUT, &desc->flags))) {
		status = device_create_file(dev, &dev_attr_edge);
		if (status)
			goto fail_unregister_device;
	}

	set_bit(FLAG_EXPORT, &desc->flags);
	mutex_unlock(&sysfs_lock);
	return 0;

fail_unregister_device:
	device_unregister(dev);
fail_unlock:
	mutex_unlock(&sysfs_lock);
	gpiod_dbg(desc, "%s: status %d\n", __func__, status);
	return status;
}
EXPORT_SYMBOL_GPL(gpiod_export);

static int match_export(struct device *dev, const void *data)
{
	return dev_get_drvdata(dev) == data;
}

/**
 * gpiod_export_link - create a sysfs link to an exported GPIO node
 * @dev: device under which to create symlink
 * @name: name of the symlink
 * @gpio: gpio to create symlink to, already exported
 *
 * Set up a symlink from /sys/.../dev/name to /sys/class/gpio/gpioN
 * node. Caller is responsible for unlinking.
 *
 * Returns zero on success, else an error.
 */
int gpiod_export_link(struct device *dev, const char *name,
		      struct gpio_desc *desc)
{
	int			status = -EINVAL;

	if (!desc) {
		pr_warn("%s: invalid GPIO\n", __func__);
		return -EINVAL;
	}

	mutex_lock(&sysfs_lock);

	if (test_bit(FLAG_EXPORT, &desc->flags)) {
		struct device *tdev;

		tdev = class_find_device(&gpio_class, NULL, desc, match_export);
		if (tdev != NULL) {
			status = sysfs_create_link(&dev->kobj, &tdev->kobj,
						name);
		} else {
			status = -ENODEV;
		}
	}

	mutex_unlock(&sysfs_lock);

	if (status)
		gpiod_dbg(desc, "%s: status %d\n", __func__, status);

	return status;
}
EXPORT_SYMBOL_GPL(gpiod_export_link);

/**
 * gpiod_sysfs_set_active_low - set the polarity of gpio sysfs value
 * @gpio: gpio to change
 * @value: non-zero to use active low, i.e. inverted values
 *
 * Set the polarity of /sys/class/gpio/gpioN/value sysfs attribute.
 * The GPIO does not have to be exported yet.  If poll(2) support has
 * been enabled for either rising or falling edge, it will be
 * reconfigured to follow the new polarity.
 *
 * Returns zero on success, else an error.
 */
int gpiod_sysfs_set_active_low(struct gpio_desc *desc, int value)
{
	struct device		*dev = NULL;
	int			status = -EINVAL;

	if (!desc) {
		pr_warn("%s: invalid GPIO\n", __func__);
		return -EINVAL;
	}

	mutex_lock(&sysfs_lock);

	if (test_bit(FLAG_EXPORT, &desc->flags)) {
		dev = class_find_device(&gpio_class, NULL, desc, match_export);
		if (dev == NULL) {
			status = -ENODEV;
			goto unlock;
		}
	}

	status = sysfs_set_active_low(desc, dev, value);

unlock:
	mutex_unlock(&sysfs_lock);

	if (status)
		gpiod_dbg(desc, "%s: status %d\n", __func__, status);

	return status;
}
EXPORT_SYMBOL_GPL(gpiod_sysfs_set_active_low);

/**
 * gpiod_unexport - reverse effect of gpio_export()
 * @gpio: gpio to make unavailable
 *
 * This is implicit on gpio_free().
 */
void gpiod_unexport(struct gpio_desc *desc)
{
	int			status = 0;
	struct device		*dev = NULL;

	if (!desc) {
		pr_warn("%s: invalid GPIO\n", __func__);
		return;
	}

	mutex_lock(&sysfs_lock);

	if (test_bit(FLAG_EXPORT, &desc->flags)) {

		dev = class_find_device(&gpio_class, NULL, desc, match_export);
		if (dev) {
			gpio_setup_irq(desc, dev, 0);
			clear_bit(FLAG_EXPORT, &desc->flags);
		} else
			status = -ENODEV;
	}

	mutex_unlock(&sysfs_lock);

	if (dev) {
		device_unregister(dev);
		put_device(dev);
	}

	if (status)
		gpiod_dbg(desc, "%s: status %d\n", __func__, status);
}
EXPORT_SYMBOL_GPL(gpiod_unexport);

int gpiochip_export(struct gpio_chip *chip)
{
	int		status;
	struct device	*dev;

	/* Many systems register gpio chips for SOC support very early,
	 * before driver model support is available.  In those cases we
	 * export this later, in gpiolib_sysfs_init() ... here we just
	 * verify that _some_ field of gpio_class got initialized.
	 */
	if (!gpio_class.p)
		return 0;

	/* use chip->base for the ID; it's already known to be unique */
	mutex_lock(&sysfs_lock);
	dev = device_create(&gpio_class, chip->dev, MKDEV(0, 0), chip,
				"gpiochip%d", chip->base);
	if (!IS_ERR(dev)) {
		status = sysfs_create_group(&dev->kobj,
				&gpiochip_attr_group);
	} else
		status = PTR_ERR(dev);
	chip->exported = (status == 0);
	mutex_unlock(&sysfs_lock);

	if (status)
		chip_dbg(chip, "%s: status %d\n", __func__, status);

	return status;
}

void gpiochip_unexport(struct gpio_chip *chip)
{
	int			status;
	struct device		*dev;

	mutex_lock(&sysfs_lock);
	dev = class_find_device(&gpio_class, NULL, chip, match_export);
	if (dev) {
		put_device(dev);
		device_unregister(dev);
		chip->exported = false;
		status = 0;
	} else
		status = -ENODEV;
	mutex_unlock(&sysfs_lock);

	if (status)
		chip_dbg(chip, "%s: status %d\n", __func__, status);
}

static int __init gpiolib_sysfs_init(void)
{
	int		status;
	unsigned long	flags;
	struct gpio_chip *chip;

	status = class_register(&gpio_class);
	if (status < 0)
		return status;

	/* Scan and register the gpio_chips which registered very
	 * early (e.g. before the class_register above was called).
	 *
	 * We run before arch_initcall() so chip->dev nodes can have
	 * registered, and so arch_initcall() can always gpio_export().
	 */
	spin_lock_irqsave(&gpio_lock, flags);
	list_for_each_entry(chip, &gpio_chips, list) {
		if (chip->exported)
			continue;

		/*
		 * TODO we yield gpio_lock here because gpiochip_export()
		 * acquires a mutex. This is unsafe and needs to be fixed.
		 *
		 * Also it would be nice to use gpiochip_find() here so we
		 * can keep gpio_chips local to gpiolib.c, but the yield of
		 * gpio_lock prevents us from doing this.
		 */
		spin_unlock_irqrestore(&gpio_lock, flags);
		status = gpiochip_export(chip);
		spin_lock_irqsave(&gpio_lock, flags);
	}
	spin_unlock_irqrestore(&gpio_lock, flags);


	return status;
}
postcore_initcall(gpiolib_sysfs_init);
{
	const struct gpio_chip	*chip = dev_get_drvdata(dev);

	return sprintf(buf, "%u\n", chip->ngpio);
}
static DEVICE_ATTR(ngpio, 0444, chip_ngpio_show, NULL);

static const struct attribute *gpiochip_attrs[] = {
	&dev_attr_base.attr,
	&dev_attr_label.attr,
	&dev_attr_ngpio.attr,
	NULL,
};

static const struct attribute_group gpiochip_attr_group = {
	.attrs = (struct attribute **) gpiochip_attrs,
};

/*
 * /sys/class/gpio/export ... write-only
 *	integer N ... number of GPIO to export (full access)
 * /sys/class/gpio/unexport ... write-only
 *	integer N ... number of GPIO to unexport
 */
static ssize_t export_store(struct class *class,
				struct class_attribute *attr,
				const char *buf, size_t len)
{
	long			gpio;
	struct gpio_desc	*desc;
	int			status;

	status = kstrtol(buf, 0, &gpio);
	if (status < 0)
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
	 */

	status = gpiod_request(desc, "sysfs");
	if (status < 0) {
		if (status == -EPROBE_DEFER)
			status = -ENODEV;
		goto done;
	}
	status = gpiod_export(desc, true);
	if (status < 0)
		gpiod_free(desc);
	else
		set_bit(FLAG_SYSFS, &desc->flags);

done:
	if (status)
		pr_debug("%s: status %d\n", __func__, status);
	return status ? : len;
}

static ssize_t unexport_store(struct class *class,
				struct class_attribute *attr,
				const char *buf, size_t len)
{
	long			gpio;
	struct gpio_desc	*desc;
	int			status;

	status = kstrtol(buf, 0, &gpio);
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
	 */
	if (test_and_clear_bit(FLAG_SYSFS, &desc->flags)) {
		status = 0;
		gpiod_free(desc);
	}
done:
	if (status)
		pr_debug("%s: status %d\n", __func__, status);
	return status ? : len;
}

static struct class_attribute gpio_class_attrs[] = {
	__ATTR(export, 0200, NULL, export_store),
	__ATTR(unexport, 0200, NULL, unexport_store),
	__ATTR_NULL,
};

static struct class gpio_class = {
	.name =		"gpio",
	.owner =	THIS_MODULE,

	.class_attrs =	gpio_class_attrs,
};


/**
 * gpiod_export - export a GPIO through sysfs
 * @gpio: gpio to make available, already requested
 * @direction_may_change: true if userspace may change gpio direction
 * Context: arch_initcall or later
 *
 * When drivers want to make a GPIO accessible to userspace after they
 * have requested it -- perhaps while debugging, or as part of their
 * public interface -- they may use this routine.  If the GPIO can
 * change direction (some can't) and the caller allows it, userspace
 * will see "direction" sysfs attribute which may be used to change
 * the gpio's direction.  A "value" attribute will always be provided.
 *
 * Returns zero on success, else an error.
 */
int gpiod_export(struct gpio_desc *desc, bool direction_may_change)
{
	unsigned long		flags;
	int			status;
	const char		*ioname = NULL;
	struct device		*dev;
	int			offset;

	/* can't export until sysfs is available ... */
	if (!gpio_class.p) {
		pr_debug("%s: called too early!\n", __func__);
		return -ENOENT;
	}

	if (!desc) {
		pr_debug("%s: invalid gpio descriptor\n", __func__);
		return -EINVAL;
	}

	mutex_lock(&sysfs_lock);

	spin_lock_irqsave(&gpio_lock, flags);
	if (!test_bit(FLAG_REQUESTED, &desc->flags) ||
	     test_bit(FLAG_EXPORT, &desc->flags)) {
		spin_unlock_irqrestore(&gpio_lock, flags);
		gpiod_dbg(desc, "%s: unavailable (requested=%d, exported=%d)\n",
				__func__,
				test_bit(FLAG_REQUESTED, &desc->flags),
				test_bit(FLAG_EXPORT, &desc->flags));
		status = -EPERM;
		goto fail_unlock;
	}

	if (!desc->chip->direction_input || !desc->chip->direction_output)
		direction_may_change = false;
	spin_unlock_irqrestore(&gpio_lock, flags);

	offset = gpio_chip_hwgpio(desc);
	if (desc->chip->names && desc->chip->names[offset])
		ioname = desc->chip->names[offset];

	dev = device_create(&gpio_class, desc->chip->dev, MKDEV(0, 0),
			    desc, ioname ? ioname : "gpio%u",
			    desc_to_gpio(desc));
	if (IS_ERR(dev)) {
		status = PTR_ERR(dev);
		goto fail_unlock;
	}

	status = sysfs_create_group(&dev->kobj, &gpio_attr_group);
	if (status)
		goto fail_unregister_device;

	if (direction_may_change) {
		status = device_create_file(dev, &dev_attr_direction);
		if (status)
			goto fail_unregister_device;
	}

	if (gpiod_to_irq(desc) >= 0 && (direction_may_change ||
				       !test_bit(FLAG_IS_OUT, &desc->flags))) {
		status = device_create_file(dev, &dev_attr_edge);
		if (status)
			goto fail_unregister_device;
	}

	set_bit(FLAG_EXPORT, &desc->flags);
	mutex_unlock(&sysfs_lock);
	return 0;

fail_unregister_device:
	device_unregister(dev);
fail_unlock:
	mutex_unlock(&sysfs_lock);
	gpiod_dbg(desc, "%s: status %d\n", __func__, status);
	return status;
}
EXPORT_SYMBOL_GPL(gpiod_export);

static int match_export(struct device *dev, const void *data)
{
	return dev_get_drvdata(dev) == data;
}

/**
 * gpiod_export_link - create a sysfs link to an exported GPIO node
 * @dev: device under which to create symlink
 * @name: name of the symlink
 * @gpio: gpio to create symlink to, already exported
 *
 * Set up a symlink from /sys/.../dev/name to /sys/class/gpio/gpioN
 * node. Caller is responsible for unlinking.
 *
 * Returns zero on success, else an error.
 */
int gpiod_export_link(struct device *dev, const char *name,
		      struct gpio_desc *desc)
{
	int			status = -EINVAL;

	if (!desc) {
		pr_warn("%s: invalid GPIO\n", __func__);
		return -EINVAL;
	}

	mutex_lock(&sysfs_lock);

	if (test_bit(FLAG_EXPORT, &desc->flags)) {
		struct device *tdev;

		tdev = class_find_device(&gpio_class, NULL, desc, match_export);
		if (tdev != NULL) {
			status = sysfs_create_link(&dev->kobj, &tdev->kobj,
						name);
		} else {
			status = -ENODEV;
		}
	}

	mutex_unlock(&sysfs_lock);

	if (status)
		gpiod_dbg(desc, "%s: status %d\n", __func__, status);

	return status;
}
EXPORT_SYMBOL_GPL(gpiod_export_link);

/**
 * gpiod_sysfs_set_active_low - set the polarity of gpio sysfs value
 * @gpio: gpio to change
 * @value: non-zero to use active low, i.e. inverted values
 *
 * Set the polarity of /sys/class/gpio/gpioN/value sysfs attribute.
 * The GPIO does not have to be exported yet.  If poll(2) support has
 * been enabled for either rising or falling edge, it will be
 * reconfigured to follow the new polarity.
 *
 * Returns zero on success, else an error.
 */
int gpiod_sysfs_set_active_low(struct gpio_desc *desc, int value)
{
	struct device		*dev = NULL;
	int			status = -EINVAL;

	if (!desc) {
		pr_warn("%s: invalid GPIO\n", __func__);
		return -EINVAL;
	}

	mutex_lock(&sysfs_lock);

	if (test_bit(FLAG_EXPORT, &desc->flags)) {
		dev = class_find_device(&gpio_class, NULL, desc, match_export);
		if (dev == NULL) {
			status = -ENODEV;
			goto unlock;
		}
	}

	status = sysfs_set_active_low(desc, dev, value);

unlock:
	mutex_unlock(&sysfs_lock);

	if (status)
		gpiod_dbg(desc, "%s: status %d\n", __func__, status);

	return status;
}
EXPORT_SYMBOL_GPL(gpiod_sysfs_set_active_low);

/**
 * gpiod_unexport - reverse effect of gpio_export()
 * @gpio: gpio to make unavailable
 *
 * This is implicit on gpio_free().
 */
void gpiod_unexport(struct gpio_desc *desc)
{
	int			status = 0;
	struct device		*dev = NULL;

	if (!desc) {
		pr_warn("%s: invalid GPIO\n", __func__);
		return;
	}

	mutex_lock(&sysfs_lock);

	if (test_bit(FLAG_EXPORT, &desc->flags)) {

		dev = class_find_device(&gpio_class, NULL, desc, match_export);
		if (dev) {
			gpio_setup_irq(desc, dev, 0);
			clear_bit(FLAG_EXPORT, &desc->flags);
		} else
			status = -ENODEV;
	}

	mutex_unlock(&sysfs_lock);

	if (dev) {
		device_unregister(dev);
		put_device(dev);
	}

	if (status)
		gpiod_dbg(desc, "%s: status %d\n", __func__, status);
}
EXPORT_SYMBOL_GPL(gpiod_unexport);

int gpiochip_export(struct gpio_chip *chip)
{
	int		status;
	struct device	*dev;

	/* Many systems register gpio chips for SOC support very early,
	 * before driver model support is available.  In those cases we
	 * export this later, in gpiolib_sysfs_init() ... here we just
	 * verify that _some_ field of gpio_class got initialized.
	 */
	if (!gpio_class.p)
		return 0;

	/* use chip->base for the ID; it's already known to be unique */
	mutex_lock(&sysfs_lock);
	dev = device_create(&gpio_class, chip->dev, MKDEV(0, 0), chip,
				"gpiochip%d", chip->base);
	if (!IS_ERR(dev)) {
		status = sysfs_create_group(&dev->kobj,
				&gpiochip_attr_group);
	} else
		status = PTR_ERR(dev);
	chip->exported = (status == 0);
	mutex_unlock(&sysfs_lock);

	if (status)
		chip_dbg(chip, "%s: status %d\n", __func__, status);

	return status;
}

void gpiochip_unexport(struct gpio_chip *chip)
{
	int			status;
	struct device		*dev;

	mutex_lock(&sysfs_lock);
	dev = class_find_device(&gpio_class, NULL, chip, match_export);
	if (dev) {
		put_device(dev);
		device_unregister(dev);
		chip->exported = false;
		status = 0;
	} else
		status = -ENODEV;
	mutex_unlock(&sysfs_lock);

	if (status)
		chip_dbg(chip, "%s: status %d\n", __func__, status);
}

static int __init gpiolib_sysfs_init(void)
{
	int		status;
	unsigned long	flags;
	struct gpio_chip *chip;

	status = class_register(&gpio_class);
	if (status < 0)
		return status;

	/* Scan and register the gpio_chips which registered very
	 * early (e.g. before the class_register above was called).
	 *
	 * We run before arch_initcall() so chip->dev nodes can have
	 * registered, and so arch_initcall() can always gpio_export().
	 */
	spin_lock_irqsave(&gpio_lock, flags);
	list_for_each_entry(chip, &gpio_chips, list) {
		if (chip->exported)
			continue;

		/*
		 * TODO we yield gpio_lock here because gpiochip_export()
		 * acquires a mutex. This is unsafe and needs to be fixed.
		 *
		 * Also it would be nice to use gpiochip_find() here so we
		 * can keep gpio_chips local to gpiolib.c, but the yield of
		 * gpio_lock prevents us from doing this.
		 */
		spin_unlock_irqrestore(&gpio_lock, flags);
		status = gpiochip_export(chip);
		spin_lock_irqsave(&gpio_lock, flags);
	}
	spin_unlock_irqrestore(&gpio_lock, flags);


	return status;
}
postcore_initcall(gpiolib_sysfs_init);
	     test_bit(FLAG_EXPORT, &desc->flags)) {
		spin_unlock_irqrestore(&gpio_lock, flags);
		gpiod_dbg(desc, "%s: unavailable (requested=%d, exported=%d)\n",
				__func__,
				test_bit(FLAG_REQUESTED, &desc->flags),
				test_bit(FLAG_EXPORT, &desc->flags));
		status = -EPERM;
		goto fail_unlock;
	}

	if (!desc->chip->direction_input || !desc->chip->direction_output)
		direction_may_change = false;
	spin_unlock_irqrestore(&gpio_lock, flags);

	offset = gpio_chip_hwgpio(desc);
	if (desc->chip->names && desc->chip->names[offset])
		ioname = desc->chip->names[offset];

	dev = device_create(&gpio_class, desc->chip->dev, MKDEV(0, 0),
			    desc, ioname ? ioname : "gpio%u",
			    desc_to_gpio(desc));
	if (IS_ERR(dev)) {
		status = PTR_ERR(dev);
		goto fail_unlock;
	}

	status = sysfs_create_group(&dev->kobj, &gpio_attr_group);
	if (status)
		goto fail_unregister_device;

	if (direction_may_change) {
		status = device_create_file(dev, &dev_attr_direction);
		if (status)
			goto fail_unregister_device;
	}

	if (gpiod_to_irq(desc) >= 0 && (direction_may_change ||
				       !test_bit(FLAG_IS_OUT, &desc->flags))) {
		status = device_create_file(dev, &dev_attr_edge);
		if (status)
			goto fail_unregister_device;
	}

	set_bit(FLAG_EXPORT, &desc->flags);
	mutex_unlock(&sysfs_lock);
	return 0;

fail_unregister_device:
	device_unregister(dev);
fail_unlock:
	mutex_unlock(&sysfs_lock);
	gpiod_dbg(desc, "%s: status %d\n", __func__, status);
	return status;
}
EXPORT_SYMBOL_GPL(gpiod_export);

static int match_export(struct device *dev, const void *data)
{
	return dev_get_drvdata(dev) == data;
}

/**
 * gpiod_export_link - create a sysfs link to an exported GPIO node
 * @dev: device under which to create symlink
 * @name: name of the symlink
 * @gpio: gpio to create symlink to, already exported
 *
 * Set up a symlink from /sys/.../dev/name to /sys/class/gpio/gpioN
 * node. Caller is responsible for unlinking.
 *
 * Returns zero on success, else an error.
 */
int gpiod_export_link(struct device *dev, const char *name,
		      struct gpio_desc *desc)
{
	int			status = -EINVAL;

	if (!desc) {
		pr_warn("%s: invalid GPIO\n", __func__);
		return -EINVAL;
	}

	mutex_lock(&sysfs_lock);

	if (test_bit(FLAG_EXPORT, &desc->flags)) {
		struct device *tdev;

		tdev = class_find_device(&gpio_class, NULL, desc, match_export);
		if (tdev != NULL) {
			status = sysfs_create_link(&dev->kobj, &tdev->kobj,
						name);
		} else {
			status = -ENODEV;
		}
	}
		if (dev) {
			gpio_setup_irq(desc, dev, 0);
			clear_bit(FLAG_EXPORT, &desc->flags);
		} else
			status = -ENODEV;
	}
{
	int		status;
	struct device	*dev;

	/* Many systems register gpio chips for SOC support very early,
	 * before driver model support is available.  In those cases we
	 * export this later, in gpiolib_sysfs_init() ... here we just
	 * verify that _some_ field of gpio_class got initialized.
	 */
	if (!gpio_class.p)
		return 0;

	/* use chip->base for the ID; it's already known to be unique */
	mutex_lock(&sysfs_lock);
	dev = device_create(&gpio_class, chip->dev, MKDEV(0, 0), chip,
				"gpiochip%d", chip->base);
	if (!IS_ERR(dev)) {
		status = sysfs_create_group(&dev->kobj,
				&gpiochip_attr_group);
	} else
		status = PTR_ERR(dev);
	chip->exported = (status == 0);
	mutex_unlock(&sysfs_lock);

	if (status)
		chip_dbg(chip, "%s: status %d\n", __func__, status);

	return status;
}
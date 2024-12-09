	return status;
}

static DEVICE_ATTR(value, 0644,
		gpio_value_show, gpio_value_store);

static irqreturn_t gpio_sysfs_irq(int irq, void *priv)
{
	return status ? : size;
}

static DEVICE_ATTR(active_low, 0644,
		gpio_active_low_show, gpio_active_low_store);

static umode_t gpio_is_visible(struct kobject *kobj, struct attribute *attr,
			       int n)
{
	struct device *dev = container_of(kobj, struct device, kobj);
	struct gpio_desc *desc = dev_get_drvdata(dev);
	umode_t mode = attr->mode;
	bool show_direction = test_bit(FLAG_SYSFS_DIR, &desc->flags);

	if (attr == &dev_attr_direction.attr) {
		if (!show_direction)
			mode = 0;
	} else if (attr == &dev_attr_edge.attr) {
		if (gpiod_to_irq(desc) < 0)
			mode = 0;
		if (!show_direction && test_bit(FLAG_IS_OUT, &desc->flags))
			mode = 0;
	}

	return mode;
}

static struct attribute *gpio_attrs[] = {
	&dev_attr_direction.attr,
	&dev_attr_edge.attr,
	&dev_attr_value.attr,
	&dev_attr_active_low.attr,
	NULL,
};

static const struct attribute_group gpio_group = {
	.attrs = gpio_attrs,
	.is_visible = gpio_is_visible,
};

static const struct attribute_group *gpio_groups[] = {
	&gpio_group,
	NULL
};

/*
 * /sys/class/gpio/gpiochipN/
}
static DEVICE_ATTR(ngpio, 0444, chip_ngpio_show, NULL);

static struct attribute *gpiochip_attrs[] = {
	&dev_attr_base.attr,
	&dev_attr_label.attr,
	&dev_attr_ngpio.attr,
	NULL,
};
ATTRIBUTE_GROUPS(gpiochip);

/*
 * /sys/class/gpio/export ... write-only
 *	integer N ... number of GPIO to export (full access)
		goto fail_unlock;
	}

	if (desc->chip->direction_input && desc->chip->direction_output &&
			direction_may_change) {
		set_bit(FLAG_SYSFS_DIR, &desc->flags);
	}

	spin_unlock_irqrestore(&gpio_lock, flags);

	offset = gpio_chip_hwgpio(desc);
	if (desc->chip->names && desc->chip->names[offset])
		ioname = desc->chip->names[offset];

	dev = device_create_with_groups(&gpio_class, desc->chip->dev,
					MKDEV(0, 0), desc, gpio_groups,
					ioname ? ioname : "gpio%u",
					desc_to_gpio(desc));
	if (IS_ERR(dev)) {
		status = PTR_ERR(dev);
		goto fail_unlock;
	}

	set_bit(FLAG_EXPORT, &desc->flags);
	mutex_unlock(&sysfs_lock);
	return 0;

fail_unlock:
	mutex_unlock(&sysfs_lock);
	gpiod_dbg(desc, "%s: status %d\n", __func__, status);
	return status;
		dev = class_find_device(&gpio_class, NULL, desc, match_export);
		if (dev) {
			gpio_setup_irq(desc, dev, 0);
			clear_bit(FLAG_SYSFS_DIR, &desc->flags);
			clear_bit(FLAG_EXPORT, &desc->flags);
		} else
			status = -ENODEV;
	}

	/* use chip->base for the ID; it's already known to be unique */
	mutex_lock(&sysfs_lock);
	dev = device_create_with_groups(&gpio_class, chip->dev, MKDEV(0, 0),
					chip, gpiochip_groups,
					"gpiochip%d", chip->base);
	if (IS_ERR(dev))
		status = PTR_ERR(dev);
	else
		status = 0;
	chip->exported = (status == 0);
	mutex_unlock(&sysfs_lock);

	if (status)
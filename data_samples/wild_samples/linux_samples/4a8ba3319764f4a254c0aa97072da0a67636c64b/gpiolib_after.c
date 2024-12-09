		if (base < 0) {
			status = base;
			spin_unlock_irqrestore(&gpio_lock, flags);
			goto err_free_descs;
		}
		desc->flags = !chip->direction_input ? (1 << FLAG_IS_OUT) : 0;
	}

	chip->desc = descs;

	spin_unlock_irqrestore(&gpio_lock, flags);

#ifdef CONFIG_PINCTRL
	INIT_LIST_HEAD(&chip->pin_ranges);
#endif

	of_gpiochip_add(chip);
	acpi_gpiochip_add(chip);

	status = gpiochip_export(chip);
	if (status)
		goto err_remove_chip;

	pr_debug("%s: registered GPIOs %d to %d on device: %s\n", __func__,
		chip->base, chip->base + chip->ngpio - 1,
		chip->label ? : "generic");

	return 0;

err_remove_chip:
	acpi_gpiochip_remove(chip);
	of_gpiochip_remove(chip);
	spin_lock_irqsave(&gpio_lock, flags);
	list_del(&chip->list);
	spin_unlock_irqrestore(&gpio_lock, flags);
	chip->desc = NULL;
err_free_descs:
	kfree(descs);

	/* failures here can mean systems won't boot... */
	pr_err("%s: GPIOs %d..%d (%s) failed to register\n", __func__,
		chip->base, chip->base + chip->ngpio - 1,
		chip->label ? : "generic");
	return status;
}
EXPORT_SYMBOL_GPL(gpiochip_add);

/* Forward-declaration */
static void gpiochip_irqchip_remove(struct gpio_chip *gpiochip);

/**
 * gpiochip_remove() - unregister a gpio_chip
 * @chip: the chip to unregister
 *
 * A gpio_chip with any GPIOs still requested may not be removed.
 */
void gpiochip_remove(struct gpio_chip *chip)
{
	unsigned long	flags;
	unsigned	id;

	gpiochip_unexport(chip);

	gpiochip_irqchip_remove(chip);

	acpi_gpiochip_remove(chip);
	gpiochip_remove_pin_ranges(chip);
	of_gpiochip_remove(chip);

	spin_lock_irqsave(&gpio_lock, flags);
	for (id = 0; id < chip->ngpio; id++) {
		if (test_bit(FLAG_REQUESTED, &chip->desc[id].flags))
			dev_crit(chip->dev, "REMOVING GPIOCHIP WITH GPIOS STILL REQUESTED\n");
	}
{
	unsigned long	flags;
	unsigned	id;

	gpiochip_unexport(chip);

	gpiochip_irqchip_remove(chip);

	acpi_gpiochip_remove(chip);
	gpiochip_remove_pin_ranges(chip);
	of_gpiochip_remove(chip);

	spin_lock_irqsave(&gpio_lock, flags);
	for (id = 0; id < chip->ngpio; id++) {
		if (test_bit(FLAG_REQUESTED, &chip->desc[id].flags))
			dev_crit(chip->dev, "REMOVING GPIOCHIP WITH GPIOS STILL REQUESTED\n");
	}
	for (id = 0; id < chip->ngpio; id++) {
		if (test_bit(FLAG_REQUESTED, &chip->desc[id].flags))
			dev_crit(chip->dev, "REMOVING GPIOCHIP WITH GPIOS STILL REQUESTED\n");
	}
	for (id = 0; id < chip->ngpio; id++)
		chip->desc[id].chip = NULL;

	list_del(&chip->list);
	spin_unlock_irqrestore(&gpio_lock, flags);

	kfree(chip->desc);
	chip->desc = NULL;
}
EXPORT_SYMBOL_GPL(gpiochip_remove);

/**
 * gpiochip_find() - iterator for locating a specific gpio_chip
 * @data: data to pass to match function
 * @callback: Callback function to check gpio_chip
 *
 * Similar to bus_find_device.  It returns a reference to a gpio_chip as
 * determined by a user supplied @match callback.  The callback should return
 * 0 if the device doesn't match and non-zero if it does.  If the callback is
 * non-zero, this function will return to the caller and not iterate over any
 * more gpio_chips.
 */
struct gpio_chip *gpiochip_find(void *data,
				int (*match)(struct gpio_chip *chip,
					     void *data))
{
	struct gpio_chip *chip;
	unsigned long flags;

	spin_lock_irqsave(&gpio_lock, flags);
	list_for_each_entry(chip, &gpio_chips, list)
		if (match(chip, data))
			break;

	/* No match? */
	if (&chip->list == &gpio_chips)
		chip = NULL;
	spin_unlock_irqrestore(&gpio_lock, flags);

	return chip;
}
EXPORT_SYMBOL_GPL(gpiochip_find);

static int gpiochip_match_name(struct gpio_chip *chip, void *data)
{
	const char *name = data;

	return !strcmp(chip->label, name);
}

static struct gpio_chip *find_chip_by_name(const char *name)
{
	return gpiochip_find((void *)name, gpiochip_match_name);
}

#ifdef CONFIG_GPIOLIB_IRQCHIP

/*
 * The following is irqchip helper code for gpiochips.
 */

/**
 * gpiochip_set_chained_irqchip() - sets a chained irqchip to a gpiochip
 * @gpiochip: the gpiochip to set the irqchip chain to
 * @irqchip: the irqchip to chain to the gpiochip
 * @parent_irq: the irq number corresponding to the parent IRQ for this
 * chained irqchip
 * @parent_handler: the parent interrupt handler for the accumulated IRQ
 * coming out of the gpiochip. If the interrupt is nested rather than
 * cascaded, pass NULL in this handler argument
 */
void gpiochip_set_chained_irqchip(struct gpio_chip *gpiochip,
				  struct irq_chip *irqchip,
				  int parent_irq,
				  irq_flow_handler_t parent_handler)
{
	unsigned int offset;

	if (!gpiochip->irqdomain) {
		chip_err(gpiochip, "called %s before setting up irqchip\n",
			 __func__);
		return;
	}
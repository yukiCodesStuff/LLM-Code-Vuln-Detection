
#define DLN2_GPIO_MAX_PINS 32

struct dln2_gpio {
	struct platform_device *pdev;
	struct gpio_chip gpio;

	 */
	DECLARE_BITMAP(output_enabled, DLN2_GPIO_MAX_PINS);

	/* active IRQs - not synced to hardware */
	DECLARE_BITMAP(unmasked_irqs, DLN2_GPIO_MAX_PINS);
	/* active IRQS - synced to hardware */
	DECLARE_BITMAP(enabled_irqs, DLN2_GPIO_MAX_PINS);
	int irq_type[DLN2_GPIO_MAX_PINS];
	struct mutex irq_lock;
};

struct dln2_gpio_pin {
	__le16 pin;
	return !!ret;
}

static int dln2_gpio_pin_set_out_val(struct dln2_gpio *dln2,
				     unsigned int pin, int value)
{
	struct dln2_gpio_pin_val req = {
		.pin = cpu_to_le16(pin),
		.value = value,
	};

	return dln2_transfer_tx(dln2->pdev, DLN2_GPIO_PIN_SET_OUT_VAL, &req,
				sizeof(req));
}

#define DLN2_GPIO_DIRECTION_IN		0
#define DLN2_GPIO_DIRECTION_OUT		1
static int dln2_gpio_direction_output(struct gpio_chip *chip, unsigned offset,
				      int value)
{
	struct dln2_gpio *dln2 = container_of(chip, struct dln2_gpio, gpio);
	int ret;

	ret = dln2_gpio_pin_set_out_val(dln2, offset, value);
	if (ret < 0)
		return ret;

	return dln2_gpio_set_direction(chip, offset, DLN2_GPIO_DIRECTION_OUT);
}

static int dln2_gpio_set_debounce(struct gpio_chip *chip, unsigned offset,
				&req, sizeof(req));
}

static void dln2_irq_unmask(struct irq_data *irqd)
{
	struct gpio_chip *gc = irq_data_get_irq_chip_data(irqd);
	struct dln2_gpio *dln2 = container_of(gc, struct dln2_gpio, gpio);
	int pin = irqd_to_hwirq(irqd);

	set_bit(pin, dln2->unmasked_irqs);
}

static void dln2_irq_mask(struct irq_data *irqd)
{
	struct dln2_gpio *dln2 = container_of(gc, struct dln2_gpio, gpio);
	int pin = irqd_to_hwirq(irqd);

	clear_bit(pin, dln2->unmasked_irqs);
}

static int dln2_irq_set_type(struct irq_data *irqd, unsigned type)
{

	switch (type) {
	case IRQ_TYPE_LEVEL_HIGH:
		dln2->irq_type[pin] = DLN2_GPIO_EVENT_LVL_HIGH;
		break;
	case IRQ_TYPE_LEVEL_LOW:
		dln2->irq_type[pin] = DLN2_GPIO_EVENT_LVL_LOW;
		break;
	case IRQ_TYPE_EDGE_BOTH:
		dln2->irq_type[pin] = DLN2_GPIO_EVENT_CHANGE;
		break;
	case IRQ_TYPE_EDGE_RISING:
		dln2->irq_type[pin] = DLN2_GPIO_EVENT_CHANGE_RISING;
		break;
	case IRQ_TYPE_EDGE_FALLING:
		dln2->irq_type[pin] = DLN2_GPIO_EVENT_CHANGE_FALLING;
		break;
	default:
		return -EINVAL;
	}
	return 0;
}

static void dln2_irq_bus_lock(struct irq_data *irqd)
{
	struct gpio_chip *gc = irq_data_get_irq_chip_data(irqd);
	struct dln2_gpio *dln2 = container_of(gc, struct dln2_gpio, gpio);

	mutex_lock(&dln2->irq_lock);
}

static void dln2_irq_bus_unlock(struct irq_data *irqd)
{
	struct gpio_chip *gc = irq_data_get_irq_chip_data(irqd);
	struct dln2_gpio *dln2 = container_of(gc, struct dln2_gpio, gpio);
	int pin = irqd_to_hwirq(irqd);
	int enabled, unmasked;
	unsigned type;
	int ret;

	enabled = test_bit(pin, dln2->enabled_irqs);
	unmasked = test_bit(pin, dln2->unmasked_irqs);

	if (enabled != unmasked) {
		if (unmasked) {
			type = dln2->irq_type[pin] & DLN2_GPIO_EVENT_MASK;
			set_bit(pin, dln2->enabled_irqs);
		} else {
			type = DLN2_GPIO_EVENT_NONE;
			clear_bit(pin, dln2->enabled_irqs);
		}

		ret = dln2_gpio_set_event_cfg(dln2, pin, type, 0);
		if (ret)
			dev_err(dln2->gpio.dev, "failed to set event\n");
	}

	mutex_unlock(&dln2->irq_lock);
}

static struct irq_chip dln2_gpio_irqchip = {
	.name = "dln2-irq",
	.irq_mask = dln2_irq_mask,
	.irq_unmask = dln2_irq_unmask,
	.irq_set_type = dln2_irq_set_type,
	.irq_bus_lock = dln2_irq_bus_lock,
	.irq_bus_sync_unlock = dln2_irq_bus_unlock,
};

static void dln2_gpio_event(struct platform_device *pdev, u16 echo,
			    const void *data, int len)
		return;
	}

	switch (dln2->irq_type[pin]) {
	case DLN2_GPIO_EVENT_CHANGE_RISING:
		if (event->value)
			generic_handle_irq(irq);
		break;
	struct dln2_gpio *dln2;
	struct device *dev = &pdev->dev;
	int pins;
	int ret;

	pins = dln2_gpio_get_pin_count(pdev);
	if (pins < 0) {
		dev_err(dev, "failed to get pin count: %d\n", pins);
	if (!dln2)
		return -ENOMEM;

	mutex_init(&dln2->irq_lock);

	dln2->pdev = pdev;

	dln2->gpio.label = "dln2";
static int dln2_gpio_remove(struct platform_device *pdev)
{
	struct dln2_gpio *dln2 = platform_get_drvdata(pdev);

	dln2_unregister_event_cb(pdev, DLN2_GPIO_CONDITION_MET_EV);
	gpiochip_remove(&dln2->gpio);

	return 0;
}
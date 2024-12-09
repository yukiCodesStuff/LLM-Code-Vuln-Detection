
#define DLN2_GPIO_MAX_PINS 32

struct dln2_irq_work {
	struct work_struct work;
	struct dln2_gpio *dln2;
	int pin;
	int type;
};

struct dln2_gpio {
	struct platform_device *pdev;
	struct gpio_chip gpio;

	 */
	DECLARE_BITMAP(output_enabled, DLN2_GPIO_MAX_PINS);

	DECLARE_BITMAP(irqs_masked, DLN2_GPIO_MAX_PINS);
	DECLARE_BITMAP(irqs_enabled, DLN2_GPIO_MAX_PINS);
	DECLARE_BITMAP(irqs_pending, DLN2_GPIO_MAX_PINS);
	struct dln2_irq_work *irq_work;
};

struct dln2_gpio_pin {
	__le16 pin;
	return !!ret;
}

static void dln2_gpio_pin_set_out_val(struct dln2_gpio *dln2,
				      unsigned int pin, int value)
{
	struct dln2_gpio_pin_val req = {
		.pin = cpu_to_le16(pin),
		.value = value,
	};

	dln2_transfer_tx(dln2->pdev, DLN2_GPIO_PIN_SET_OUT_VAL, &req,
			 sizeof(req));
}

#define DLN2_GPIO_DIRECTION_IN		0
#define DLN2_GPIO_DIRECTION_OUT		1
static int dln2_gpio_direction_output(struct gpio_chip *chip, unsigned offset,
				      int value)
{
	return dln2_gpio_set_direction(chip, offset, DLN2_GPIO_DIRECTION_OUT);
}

static int dln2_gpio_set_debounce(struct gpio_chip *chip, unsigned offset,
				&req, sizeof(req));
}

static void dln2_irq_work(struct work_struct *w)
{
	struct dln2_irq_work *iw = container_of(w, struct dln2_irq_work, work);
	struct dln2_gpio *dln2 = iw->dln2;
	u8 type = iw->type & DLN2_GPIO_EVENT_MASK;

	if (test_bit(iw->pin, dln2->irqs_enabled))
		dln2_gpio_set_event_cfg(dln2, iw->pin, type, 0);
	else
		dln2_gpio_set_event_cfg(dln2, iw->pin, DLN2_GPIO_EVENT_NONE, 0);
}

static void dln2_irq_enable(struct irq_data *irqd)
{
	struct gpio_chip *gc = irq_data_get_irq_chip_data(irqd);
	struct dln2_gpio *dln2 = container_of(gc, struct dln2_gpio, gpio);
	int pin = irqd_to_hwirq(irqd);

	set_bit(pin, dln2->irqs_enabled);
	schedule_work(&dln2->irq_work[pin].work);
}

static void dln2_irq_disable(struct irq_data *irqd)
{
	struct gpio_chip *gc = irq_data_get_irq_chip_data(irqd);
	struct dln2_gpio *dln2 = container_of(gc, struct dln2_gpio, gpio);
	int pin = irqd_to_hwirq(irqd);

	clear_bit(pin, dln2->irqs_enabled);
	schedule_work(&dln2->irq_work[pin].work);
}

static void dln2_irq_mask(struct irq_data *irqd)
{
	struct dln2_gpio *dln2 = container_of(gc, struct dln2_gpio, gpio);
	int pin = irqd_to_hwirq(irqd);

	set_bit(pin, dln2->irqs_masked);
}

static void dln2_irq_unmask(struct irq_data *irqd)
{
	struct gpio_chip *gc = irq_data_get_irq_chip_data(irqd);
	struct dln2_gpio *dln2 = container_of(gc, struct dln2_gpio, gpio);
	struct device *dev = dln2->gpio.dev;
	int pin = irqd_to_hwirq(irqd);

	if (test_and_clear_bit(pin, dln2->irqs_pending)) {
		int irq;

		irq = irq_find_mapping(dln2->gpio.irqdomain, pin);
		if (!irq) {
			dev_err(dev, "pin %d not mapped to IRQ\n", pin);
			return;
		}

		generic_handle_irq(irq);
	}
}

static int dln2_irq_set_type(struct irq_data *irqd, unsigned type)
{

	switch (type) {
	case IRQ_TYPE_LEVEL_HIGH:
		dln2->irq_work[pin].type = DLN2_GPIO_EVENT_LVL_HIGH;
		break;
	case IRQ_TYPE_LEVEL_LOW:
		dln2->irq_work[pin].type = DLN2_GPIO_EVENT_LVL_LOW;
		break;
	case IRQ_TYPE_EDGE_BOTH:
		dln2->irq_work[pin].type = DLN2_GPIO_EVENT_CHANGE;
		break;
	case IRQ_TYPE_EDGE_RISING:
		dln2->irq_work[pin].type = DLN2_GPIO_EVENT_CHANGE_RISING;
		break;
	case IRQ_TYPE_EDGE_FALLING:
		dln2->irq_work[pin].type = DLN2_GPIO_EVENT_CHANGE_FALLING;
		break;
	default:
		return -EINVAL;
	}
	return 0;
}

static struct irq_chip dln2_gpio_irqchip = {
	.name = "dln2-irq",
	.irq_enable = dln2_irq_enable,
	.irq_disable = dln2_irq_disable,
	.irq_mask = dln2_irq_mask,
	.irq_unmask = dln2_irq_unmask,
	.irq_set_type = dln2_irq_set_type,
};

static void dln2_gpio_event(struct platform_device *pdev, u16 echo,
			    const void *data, int len)
		return;
	}

	if (!test_bit(pin, dln2->irqs_enabled))
		return;
	if (test_bit(pin, dln2->irqs_masked)) {
		set_bit(pin, dln2->irqs_pending);
		return;
	}

	switch (dln2->irq_work[pin].type) {
	case DLN2_GPIO_EVENT_CHANGE_RISING:
		if (event->value)
			generic_handle_irq(irq);
		break;
	struct dln2_gpio *dln2;
	struct device *dev = &pdev->dev;
	int pins;
	int i, ret;

	pins = dln2_gpio_get_pin_count(pdev);
	if (pins < 0) {
		dev_err(dev, "failed to get pin count: %d\n", pins);
	if (!dln2)
		return -ENOMEM;

	dln2->irq_work = devm_kcalloc(&pdev->dev, pins,
				      sizeof(struct dln2_irq_work), GFP_KERNEL);
	if (!dln2->irq_work)
		return -ENOMEM;
	for (i = 0; i < pins; i++) {
		INIT_WORK(&dln2->irq_work[i].work, dln2_irq_work);
		dln2->irq_work[i].pin = i;
		dln2->irq_work[i].dln2 = dln2;
	}

	dln2->pdev = pdev;

	dln2->gpio.label = "dln2";
static int dln2_gpio_remove(struct platform_device *pdev)
{
	struct dln2_gpio *dln2 = platform_get_drvdata(pdev);
	int i;

	dln2_unregister_event_cb(pdev, DLN2_GPIO_CONDITION_MET_EV);
	for (i = 0; i < dln2->gpio.ngpio; i++)
		flush_work(&dln2->irq_work[i].work);
	gpiochip_remove(&dln2->gpio);

	return 0;
}
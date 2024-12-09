 */
struct pcf857x {
	struct gpio_chip	chip;
	struct irq_chip		irqchip;
	struct i2c_client	*client;
	struct mutex		lock;		/* protect 'out' */
	unsigned		out;		/* software latch */
	unsigned		status;		/* current status */
	mutex_unlock(&gpio->lock);
}

/*-------------------------------------------------------------------------*/

static int pcf857x_probe(struct i2c_client *client,
			 const struct i2c_device_id *id)

	/* Enable irqchip if we have an interrupt */
	if (client->irq) {
		gpio->irqchip.name = "pcf857x",
		gpio->irqchip.irq_enable = pcf857x_irq_enable,
		gpio->irqchip.irq_disable = pcf857x_irq_disable,
		gpio->irqchip.irq_ack = noop,
		gpio->irqchip.irq_mask = noop,
		gpio->irqchip.irq_unmask = noop,
		gpio->irqchip.irq_set_wake = pcf857x_irq_set_wake,
		gpio->irqchip.irq_bus_lock = pcf857x_irq_bus_lock,
		gpio->irqchip.irq_bus_sync_unlock = pcf857x_irq_bus_sync_unlock,
		status = gpiochip_irqchip_add_nested(&gpio->chip,
						     &gpio->irqchip,
						     0, handle_level_irq,
						     IRQ_TYPE_NONE);
		if (status) {
			dev_err(&client->dev, "cannot add irqchip\n");
		if (status)
			goto fail;

		gpiochip_set_nested_irqchip(&gpio->chip, &gpio->irqchip,
					    client->irq);
		gpio->irq_parent = client->irq;
	}

 */
struct pcf857x {
	struct gpio_chip	chip;
	struct i2c_client	*client;
	struct mutex		lock;		/* protect 'out' */
	unsigned		out;		/* software latch */
	unsigned		status;		/* current status */
	mutex_unlock(&gpio->lock);
}

static struct irq_chip pcf857x_irq_chip = {
	.name		= "pcf857x",
	.irq_enable	= pcf857x_irq_enable,
	.irq_disable	= pcf857x_irq_disable,
	.irq_ack	= noop,
	.irq_mask	= noop,
	.irq_unmask	= noop,
	.irq_set_wake	= pcf857x_irq_set_wake,
	.irq_bus_lock		= pcf857x_irq_bus_lock,
	.irq_bus_sync_unlock	= pcf857x_irq_bus_sync_unlock,
};

/*-------------------------------------------------------------------------*/

static int pcf857x_probe(struct i2c_client *client,
			 const struct i2c_device_id *id)

	/* Enable irqchip if we have an interrupt */
	if (client->irq) {
		status = gpiochip_irqchip_add_nested(&gpio->chip,
						     &pcf857x_irq_chip,
						     0, handle_level_irq,
						     IRQ_TYPE_NONE);
		if (status) {
			dev_err(&client->dev, "cannot add irqchip\n");
		if (status)
			goto fail;

		gpiochip_set_nested_irqchip(&gpio->chip, &pcf857x_irq_chip,
					    client->irq);
		gpio->irq_parent = client->irq;
	}


static int sprd_eic_get(struct gpio_chip *chip, unsigned int offset)
{
	return sprd_eic_read(chip, offset, SPRD_EIC_DBNC_DATA);
}

static int sprd_eic_direction_input(struct gpio_chip *chip, unsigned int offset)
{
			irq_set_handler_locked(data, handle_edge_irq);
			break;
		case IRQ_TYPE_EDGE_BOTH:
			sprd_eic_update(chip, offset, SPRD_EIC_ASYNC_INTBOTH, 1);
			irq_set_handler_locked(data, handle_edge_irq);
			break;
		case IRQ_TYPE_LEVEL_HIGH:
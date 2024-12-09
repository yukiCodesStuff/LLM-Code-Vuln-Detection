
static int sprd_eic_get(struct gpio_chip *chip, unsigned int offset)
{
	struct sprd_eic *sprd_eic = gpiochip_get_data(chip);

	switch (sprd_eic->type) {
	case SPRD_EIC_DEBOUNCE:
		return sprd_eic_read(chip, offset, SPRD_EIC_DBNC_DATA);
	case SPRD_EIC_ASYNC:
		return sprd_eic_read(chip, offset, SPRD_EIC_ASYNC_DATA);
	case SPRD_EIC_SYNC:
		return sprd_eic_read(chip, offset, SPRD_EIC_SYNC_DATA);
	default:
		return -ENOTSUPP;
	}
}

static int sprd_eic_direction_input(struct gpio_chip *chip, unsigned int offset)
{
			irq_set_handler_locked(data, handle_edge_irq);
			break;
		case IRQ_TYPE_EDGE_BOTH:
			sprd_eic_update(chip, offset, SPRD_EIC_ASYNC_INTMODE, 0);
			sprd_eic_update(chip, offset, SPRD_EIC_ASYNC_INTBOTH, 1);
			irq_set_handler_locked(data, handle_edge_irq);
			break;
		case IRQ_TYPE_LEVEL_HIGH:
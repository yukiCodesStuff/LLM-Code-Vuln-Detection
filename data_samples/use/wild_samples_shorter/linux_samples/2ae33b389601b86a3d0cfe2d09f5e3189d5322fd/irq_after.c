
void at91_irq_suspend(void)
{
	int bit = -1;

	if (has_aic5()) {
		/* disable enabled irqs */
		while ((bit = find_next_bit(backups, n_irqs, bit + 1)) < n_irqs) {
			at91_aic_write(AT91_AIC5_SSR,
				       bit & AT91_AIC5_INTSEL_MSK);
			at91_aic_write(AT91_AIC5_IDCR, 1);
		}
		/* enable wakeup irqs */
		bit = -1;
		while ((bit = find_next_bit(wakeups, n_irqs, bit + 1)) < n_irqs) {
			at91_aic_write(AT91_AIC5_SSR,
				       bit & AT91_AIC5_INTSEL_MSK);
			at91_aic_write(AT91_AIC5_IECR, 1);
		}
	} else {
		at91_aic_write(AT91_AIC_IDCR, *backups);
		at91_aic_write(AT91_AIC_IECR, *wakeups);

void at91_irq_resume(void)
{
	int bit = -1;

	if (has_aic5()) {
		/* disable wakeup irqs */
		while ((bit = find_next_bit(wakeups, n_irqs, bit + 1)) < n_irqs) {
			at91_aic_write(AT91_AIC5_SSR,
				       bit & AT91_AIC5_INTSEL_MSK);
			at91_aic_write(AT91_AIC5_IDCR, 1);
		}
		/* enable irqs disabled for suspend */
		bit = -1;
		while ((bit = find_next_bit(backups, n_irqs, bit + 1)) < n_irqs) {
			at91_aic_write(AT91_AIC5_SSR,
				       bit & AT91_AIC5_INTSEL_MSK);
			at91_aic_write(AT91_AIC5_IECR, 1);
		}
	} else {
		at91_aic_write(AT91_AIC_IDCR, *wakeups);
		at91_aic_write(AT91_AIC_IECR, *backups);
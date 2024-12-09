
static int at91_pm_enter(suspend_state_t state)
{
	at91_gpio_suspend();
	at91_irq_suspend();

	pr_debug("AT91: PM - wake mask %08x, pm state %d\n",
			/* remember all the always-wake irqs */
error:
	target_state = PM_SUSPEND_ON;
	at91_irq_resume();
	at91_gpio_resume();
	return 0;
}

/*
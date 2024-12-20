{
	return (target_state == PM_SUSPEND_MEM);
}
EXPORT_SYMBOL(at91_suspend_entering_slow_clock);


static void (*slow_clock)(void __iomem *pmc, void __iomem *ramc0,
			  void __iomem *ramc1, int memctrl);

#ifdef CONFIG_AT91_SLOW_CLOCK
extern void at91_slow_clock(void __iomem *pmc, void __iomem *ramc0,
			    void __iomem *ramc1, int memctrl);
extern u32 at91_slow_clock_sz;
#endif

static int at91_pm_enter(suspend_state_t state)
{
	at91_gpio_suspend();
	at91_irq_suspend();

	pr_debug("AT91: PM - wake mask %08x, pm state %d\n",
			/* remember all the always-wake irqs */
			(at91_pmc_read(AT91_PMC_PCSR)
					| (1 << AT91_ID_FIQ)
					| (1 << AT91_ID_SYS)
					| (at91_extern_irq))
				& at91_aic_read(AT91_AIC_IMR),
			state);

	switch (state) {
		/*
		 * Suspend-to-RAM is like STANDBY plus slow clock mode, so
		 * drivers must suspend more deeply:  only the master clock
		 * controller may be using the main oscillator.
		 */
		case PM_SUSPEND_MEM:
			/*
			 * Ensure that clocks are in a valid state.
			 */
			if (!at91_pm_verify_clocks())
				goto error;

			/*
			 * Enter slow clock mode by switching over to clk32k and
			 * turning off the main oscillator; reverse on wakeup.
			 */
			if (slow_clock) {
				int memctrl = AT91_MEMCTRL_SDRAMC;

				if (cpu_is_at91rm9200())
					memctrl = AT91_MEMCTRL_MC;
				else if (cpu_is_at91sam9g45())
					memctrl = AT91_MEMCTRL_DDRSDR;
#ifdef CONFIG_AT91_SLOW_CLOCK
				/* copy slow_clock handler to SRAM, and call it */
				memcpy(slow_clock, at91_slow_clock, at91_slow_clock_sz);
#endif
				slow_clock(at91_pmc_base, at91_ramc_base[0],
					   at91_ramc_base[1], memctrl);
				break;
			} else {
				pr_info("AT91: PM - no slow clock mode enabled ...\n");
				/* FALLTHROUGH leaving master clock alone */
			}

		/*
		 * STANDBY mode has *all* drivers suspended; ignores irqs not
		 * marked as 'wakeup' event sources; and reduces DRAM power.
		 * But otherwise it's identical to PM_SUSPEND_ON:  cpu idle, and
		 * nothing fancy done with main or cpu clocks.
		 */
		case PM_SUSPEND_STANDBY:
			/*
			 * NOTE: the Wait-for-Interrupt instruction needs to be
			 * in icache so no SDRAM accesses are needed until the
			 * wakeup IRQ occurs and self-refresh is terminated.
			 * For ARM 926 based chips, this requirement is weaker
			 * as at91sam9 can access a RAM in self-refresh mode.
			 */
			if (cpu_is_at91rm9200())
				at91rm9200_standby();
			else if (cpu_is_at91sam9g45())
				at91sam9g45_standby();
			else
				at91sam9_standby();
			break;

		case PM_SUSPEND_ON:
			cpu_do_idle();
			break;

		default:
			pr_debug("AT91: PM - bogus suspend state %d\n", state);
			goto error;
	}

	pr_debug("AT91: PM - wakeup %08x\n",
			at91_aic_read(AT91_AIC_IPR) & at91_aic_read(AT91_AIC_IMR));

error:
	target_state = PM_SUSPEND_ON;
	at91_irq_resume();
	at91_gpio_resume();
	return 0;
}

/*
 * Called right prior to thawing processes.
 */
static void at91_pm_end(void)
{
	target_state = PM_SUSPEND_ON;
}


static const struct platform_suspend_ops at91_pm_ops = {
	.valid	= at91_pm_valid_state,
	.begin	= at91_pm_begin,
	.enter	= at91_pm_enter,
	.end	= at91_pm_end,
};

static int __init at91_pm_init(void)
{
#ifdef CONFIG_AT91_SLOW_CLOCK
	slow_clock = (void *) (AT91_IO_VIRT_BASE - at91_slow_clock_sz);
#endif

	pr_info("AT91: Power Management%s\n", (slow_clock ? " (with slow clock mode)" : ""));

	/* AT91RM9200 SDRAM low-power mode cannot be used with self-refresh. */
	if (cpu_is_at91rm9200())
		at91_ramc_write(0, AT91RM9200_SDRAMC_LPR, 0);

	suspend_set_ops(&at91_pm_ops);

	show_reset_status();
	return 0;
}
arch_initcall(at91_pm_init);
			} else {
				pr_info("AT91: PM - no slow clock mode enabled ...\n");
				/* FALLTHROUGH leaving master clock alone */
			}

		/*
		 * STANDBY mode has *all* drivers suspended; ignores irqs not
		 * marked as 'wakeup' event sources; and reduces DRAM power.
		 * But otherwise it's identical to PM_SUSPEND_ON:  cpu idle, and
		 * nothing fancy done with main or cpu clocks.
		 */
		case PM_SUSPEND_STANDBY:
			/*
			 * NOTE: the Wait-for-Interrupt instruction needs to be
			 * in icache so no SDRAM accesses are needed until the
			 * wakeup IRQ occurs and self-refresh is terminated.
			 * For ARM 926 based chips, this requirement is weaker
			 * as at91sam9 can access a RAM in self-refresh mode.
			 */
			if (cpu_is_at91rm9200())
				at91rm9200_standby();
			else if (cpu_is_at91sam9g45())
				at91sam9g45_standby();
			else
				at91sam9_standby();
			break;

		case PM_SUSPEND_ON:
			cpu_do_idle();
			break;

		default:
			pr_debug("AT91: PM - bogus suspend state %d\n", state);
			goto error;
	}

	pr_debug("AT91: PM - wakeup %08x\n",
			at91_aic_read(AT91_AIC_IPR) & at91_aic_read(AT91_AIC_IMR));

error:
	target_state = PM_SUSPEND_ON;
	at91_irq_resume();
	at91_gpio_resume();
	return 0;
}

/*
 * Called right prior to thawing processes.
 */
static void at91_pm_end(void)
{
	target_state = PM_SUSPEND_ON;
}


static const struct platform_suspend_ops at91_pm_ops = {
	.valid	= at91_pm_valid_state,
	.begin	= at91_pm_begin,
	.enter	= at91_pm_enter,
	.end	= at91_pm_end,
};

static int __init at91_pm_init(void)
{
#ifdef CONFIG_AT91_SLOW_CLOCK
	slow_clock = (void *) (AT91_IO_VIRT_BASE - at91_slow_clock_sz);
#endif

	pr_info("AT91: Power Management%s\n", (slow_clock ? " (with slow clock mode)" : ""));

	/* AT91RM9200 SDRAM low-power mode cannot be used with self-refresh. */
	if (cpu_is_at91rm9200())
		at91_ramc_write(0, AT91RM9200_SDRAMC_LPR, 0);

	suspend_set_ops(&at91_pm_ops);

	show_reset_status();
	return 0;
}

static inline notrace unsigned long arch_local_save_flags(void)
{
	return __arch_local_irq_stosm(0x00);
}

static inline notrace unsigned long arch_local_irq_save(void)
{
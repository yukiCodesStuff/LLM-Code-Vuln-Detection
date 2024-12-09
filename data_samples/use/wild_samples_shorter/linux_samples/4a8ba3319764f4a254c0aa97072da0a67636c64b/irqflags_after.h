
static inline notrace unsigned long arch_local_save_flags(void)
{
	return __arch_local_irq_stnsm(0xff);
}

static inline notrace unsigned long arch_local_irq_save(void)
{
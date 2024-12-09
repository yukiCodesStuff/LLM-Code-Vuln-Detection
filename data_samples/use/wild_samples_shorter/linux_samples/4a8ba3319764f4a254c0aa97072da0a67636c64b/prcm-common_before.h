	u8 nr_irqs;
	const struct omap_prcm_irq *irqs;
	int irq;
	void (*read_pending_irqs)(unsigned long *events);
	void (*ocp_barrier)(void);
	void (*save_and_clear_irqen)(u32 *saved_mask);
	void (*restore_irqen)(u32 *saved_mask);
struct omap_prcm_irq_setup {
	u16 ack;
	u16 mask;
	u8 nr_regs;
	u8 nr_irqs;
	const struct omap_prcm_irq *irqs;
	int irq;
	unsigned int (*xlate_irq)(unsigned int);
	void (*read_pending_irqs)(unsigned long *events);
	void (*ocp_barrier)(void);
	void (*save_and_clear_irqen)(u32 *saved_mask);
	void (*restore_irqen)(u32 *saved_mask);
	void (*reconfigure_io_chain)(void);
	u32 *saved_mask;
	u32 *priority_mask;
	int base_irq;
	bool suspended;
	bool suspend_save_flag;
};

/* OMAP_PRCM_IRQ: convenience macro for creating struct omap_prcm_irq records */
#define OMAP_PRCM_IRQ(_name, _offset, _priority) {	\
	.name = _name,					\
	.offset = _offset,				\
	.priority = _priority				\
	}

extern void omap_prcm_irq_cleanup(void);
extern int omap_prcm_register_chain_handler(
	struct omap_prcm_irq_setup *irq_setup);
extern int omap_prcm_event_to_irq(const char *event);
extern void omap_prcm_irq_prepare(void);
extern void omap_prcm_irq_complete(void);

# endif

#endif

#define SEL_INT_PENDING		(1 << 6)
#define SEL_INT_NUM_MASK	0x3f

struct icu_chip_data {
	int			nr_irqs;
	unsigned int		virq_base;
	unsigned int		cascade_irq;
static const struct mmp_intc_conf mmp2_conf = {
	.conf_enable	= 0x20,
	.conf_disable	= 0x0,
	.conf_mask	= 0x7f,
};

static void __exception_irq_entry mmp_handle_irq(struct pt_regs *regs)
{
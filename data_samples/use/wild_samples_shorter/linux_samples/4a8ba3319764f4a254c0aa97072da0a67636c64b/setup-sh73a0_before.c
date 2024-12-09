
static struct renesas_intc_irqpin_config irqpin0_platform_data = {
	.irq_base = irq_pin(0), /* IRQ0 -> IRQ7 */
};

static struct resource irqpin0_resources[] = {
	DEFINE_RES_MEM(0xe6900000, 4), /* ICR1A */

static struct renesas_intc_irqpin_config irqpin2_platform_data = {
	.irq_base = irq_pin(16), /* IRQ16 -> IRQ23 */
};

static struct resource irqpin2_resources[] = {
	DEFINE_RES_MEM(0xe6900008, 4), /* ICR3A */

static struct renesas_intc_irqpin_config irqpin3_platform_data = {
	.irq_base = irq_pin(24), /* IRQ24 -> IRQ31 */
};

static struct resource irqpin3_resources[] = {
	DEFINE_RES_MEM(0xe690000c, 4), /* ICR4A */
#include <asm/fixmap.h>
#include <asm/mpspec.h>
#include <asm/msr.h>

#define ARCH_APICTIMER_STOPS_ON_C3	1

/*

#endif /* CONFIG_X86_LOCAL_APIC */

extern void irq_enter(void);
extern void irq_exit(void);

static inline void entering_irq(void)
{
	irq_enter();
}

static inline void entering_ack_irq(void)
{
{
	irq_enter();
	ack_APIC_irq();
}

static inline void exiting_irq(void)
{
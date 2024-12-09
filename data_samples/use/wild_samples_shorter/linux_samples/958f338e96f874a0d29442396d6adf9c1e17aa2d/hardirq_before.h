#define _ASM_X86_HARDIRQ_H

#include <linux/threads.h>
#include <linux/irq.h>

typedef struct {
	unsigned int __softirq_pending;
	unsigned int __nmi_count;	/* arch dependent */
#ifdef CONFIG_X86_LOCAL_APIC
	unsigned int apic_timer_irqs;	/* arch dependent */
	unsigned int irq_spurious_count;
extern u64 arch_irq_stat(void);
#define arch_irq_stat		arch_irq_stat

#endif /* _ASM_X86_HARDIRQ_H */
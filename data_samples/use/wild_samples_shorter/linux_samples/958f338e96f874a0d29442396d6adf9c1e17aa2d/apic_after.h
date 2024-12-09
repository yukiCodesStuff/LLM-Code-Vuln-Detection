#include <asm/fixmap.h>
#include <asm/mpspec.h>
#include <asm/msr.h>
#include <asm/hardirq.h>

#define ARCH_APICTIMER_STOPS_ON_C3	1

/*

#endif /* CONFIG_X86_LOCAL_APIC */

#ifdef CONFIG_SMP
bool apic_id_is_primary_thread(unsigned int id);
#else
static inline bool apic_id_is_primary_thread(unsigned int id) { return false; }
#endif

extern void irq_enter(void);
extern void irq_exit(void);

static inline void entering_irq(void)
{
	irq_enter();
	kvm_set_cpu_l1tf_flush_l1d();
}

static inline void entering_ack_irq(void)
{
{
	irq_enter();
	ack_APIC_irq();
	kvm_set_cpu_l1tf_flush_l1d();
}

static inline void exiting_irq(void)
{
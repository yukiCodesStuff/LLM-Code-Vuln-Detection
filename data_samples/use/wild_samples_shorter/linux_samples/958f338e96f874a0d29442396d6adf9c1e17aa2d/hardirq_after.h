#define _ASM_X86_HARDIRQ_H

#include <linux/threads.h>

typedef struct {
	u16	     __softirq_pending;
#if IS_ENABLED(CONFIG_KVM_INTEL)
	u8	     kvm_cpu_l1tf_flush_l1d;
#endif
	unsigned int __nmi_count;	/* arch dependent */
#ifdef CONFIG_X86_LOCAL_APIC
	unsigned int apic_timer_irqs;	/* arch dependent */
	unsigned int irq_spurious_count;
extern u64 arch_irq_stat(void);
#define arch_irq_stat		arch_irq_stat


#if IS_ENABLED(CONFIG_KVM_INTEL)
static inline void kvm_set_cpu_l1tf_flush_l1d(void)
{
	__this_cpu_write(irq_stat.kvm_cpu_l1tf_flush_l1d, 1);
}

static inline void kvm_clear_cpu_l1tf_flush_l1d(void)
{
	__this_cpu_write(irq_stat.kvm_cpu_l1tf_flush_l1d, 0);
}

static inline bool kvm_get_cpu_l1tf_flush_l1d(void)
{
	return __this_cpu_read(irq_stat.kvm_cpu_l1tf_flush_l1d);
}
#else /* !IS_ENABLED(CONFIG_KVM_INTEL) */
static inline void kvm_set_cpu_l1tf_flush_l1d(void) { }
#endif /* IS_ENABLED(CONFIG_KVM_INTEL) */

#endif /* _ASM_X86_HARDIRQ_H */
#ifndef _ASM_X86_APIC_H
#define _ASM_X86_APIC_H

#include <linux/cpumask.h>

#include <asm/alternative.h>
#include <asm/cpufeature.h>
#include <asm/apicdef.h>
#include <linux/atomic.h>
#include <asm/fixmap.h>
#include <asm/mpspec.h>
#include <asm/msr.h>
#include <asm/hardirq.h>

#define ARCH_APICTIMER_STOPS_ON_C3	1

/*
 * Debugging macros
 */
#define APIC_QUIET   0
#define APIC_VERBOSE 1
#define APIC_DEBUG   2

/* Macros for apic_extnmi which controls external NMI masking */
#define APIC_EXTNMI_BSP		0 /* Default */
#define APIC_EXTNMI_ALL		1
#define APIC_EXTNMI_NONE	2

/*
 * Define the default level of output to be very little
 * This can be turned up by using apic=verbose for more
 * information and apic=debug for _lots_ of information.
 * apic_verbosity is defined in apic.c
 */
#define apic_printk(v, s, a...) do {       \
		if ((v) <= apic_verbosity) \
			printk(s, ##a);    \
	} while (0)


#if defined(CONFIG_X86_LOCAL_APIC) && defined(CONFIG_X86_32)
extern void generic_apic_probe(void);
#else
static inline void generic_apic_probe(void)
{
}
{
	unsigned int reg = apic_read(APIC_ID);

	return apic->get_apic_id(reg);
}

extern int default_apic_id_valid(u32 apicid);
extern int default_acpi_madt_oem_check(char *, char *);
extern void default_setup_apic_routing(void);

extern u32 apic_default_calc_apicid(unsigned int cpu);
extern u32 apic_flat_calc_apicid(unsigned int cpu);

extern bool default_check_apicid_used(physid_mask_t *map, int apicid);
extern void default_ioapic_phys_id_map(physid_mask_t *phys_map, physid_mask_t *retmap);
extern int default_cpu_present_to_apicid(int mps_cpu);
extern int default_check_phys_apicid_present(int phys_apicid);

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
	entering_irq();
	ack_APIC_irq();
}

static inline void ipi_entering_ack_irq(void)
{
	irq_enter();
	ack_APIC_irq();
	kvm_set_cpu_l1tf_flush_l1d();
}

static inline void exiting_irq(void)
{
	irq_exit();
}

static inline void exiting_ack_irq(void)
{
	ack_APIC_irq();
	irq_exit();
}

extern void ioapic_zap_locks(void);

#endif /* _ASM_X86_APIC_H */
{
	irq_enter();
	ack_APIC_irq();
	kvm_set_cpu_l1tf_flush_l1d();
}

static inline void exiting_irq(void)
{
	irq_exit();
}

static inline void exiting_ack_irq(void)
{
	ack_APIC_irq();
	irq_exit();
}

extern void ioapic_zap_locks(void);

#endif /* _ASM_X86_APIC_H */
/*
 *	Local APIC handling, local APIC timers
 *
 *	(c) 1999, 2000, 2009 Ingo Molnar <mingo@redhat.com>
 *
 *	Fixes
 *	Maciej W. Rozycki	:	Bits for genuine 82489DX APICs;
 *					thanks to Eric Gilmore
 *					and Rolf G. Tews
 *					for testing these extensively.
 *	Maciej W. Rozycki	:	Various updates and fixes.
 *	Mikael Pettersson	:	Power Management for UP-APIC.
 *	Pavel Machek and
 *	Mikael Pettersson	:	PM converted to driver model.
 */

#include <linux/perf_event.h>
#include <linux/kernel_stat.h>
#include <linux/mc146818rtc.h>
#include <linux/acpi_pmtmr.h>
#include <linux/clockchips.h>
#include <linux/interrupt.h>
#include <linux/bootmem.h>
#include <linux/ftrace.h>
#include <linux/ioport.h>
#include <linux/export.h>
#include <linux/syscore_ops.h>
#include <linux/delay.h>
#include <linux/timex.h>
#include <linux/i8253.h>
#include <linux/dmar.h>
#include <linux/init.h>
#include <linux/cpu.h>
#include <linux/dmi.h>
#include <linux/smp.h>
#include <linux/mm.h>

#include <asm/trace/irq_vectors.h>
#include <asm/irq_remapping.h>
#include <asm/perf_event.h>
#include <asm/x86_init.h>
#include <asm/pgalloc.h>
#include <linux/atomic.h>
#include <asm/mpspec.h>
#include <asm/i8259.h>
#include <asm/proto.h>
#include <asm/apic.h>
#include <asm/io_apic.h>
#include <asm/desc.h>
#include <asm/hpet.h>
#include <asm/mtrr.h>
#include <asm/time.h>
#include <asm/smp.h>
#include <asm/mce.h>
#include <asm/tsc.h>
#include <asm/hypervisor.h>
#include <asm/cpu_device_id.h>
#include <asm/intel-family.h>
#include <asm/irq_regs.h>

unsigned int num_processors;

unsigned disabled_cpus;

/* Processor that is doing the boot up */
unsigned int boot_cpu_physical_apicid = -1U;
EXPORT_SYMBOL_GPL(boot_cpu_physical_apicid);

u8 boot_cpu_apic_version;

/*
 * The highest APIC ID seen during enumeration.
 */
static unsigned int max_physical_apicid;

/*
 * Bitmask of physically existing CPUs:
 */
physid_mask_t phys_cpu_present_map;

/*
 * Processor to be disabled specified by kernel parameter
 * disable_cpu_apicid=<int>, mostly used for the kdump 2nd kernel to
 * avoid undefined behaviour caused by sending INIT from AP to BSP.
 */
static unsigned int disabled_cpu_apicid __read_mostly = BAD_APICID;

/*
 * This variable controls which CPUs receive external NMIs.  By default,
 * external NMIs are delivered only to the BSP.
 */
static int apic_extnmi = APIC_EXTNMI_BSP;

/*
 * Map cpu index to physical APIC ID
 */
DEFINE_EARLY_PER_CPU_READ_MOSTLY(u16, x86_cpu_to_apicid, BAD_APICID);
DEFINE_EARLY_PER_CPU_READ_MOSTLY(u16, x86_bios_cpu_apicid, BAD_APICID);
DEFINE_EARLY_PER_CPU_READ_MOSTLY(u32, x86_cpu_to_acpiid, U32_MAX);
EXPORT_EARLY_PER_CPU_SYMBOL(x86_cpu_to_apicid);
EXPORT_EARLY_PER_CPU_SYMBOL(x86_bios_cpu_apicid);
EXPORT_EARLY_PER_CPU_SYMBOL(x86_cpu_to_acpiid);

#ifdef CONFIG_X86_32

/*
 * On x86_32, the mapping between cpu and logical apicid may vary
 * depending on apic in use.  The following early percpu variable is
 * used for the mapping.  This is where the behaviors of x86_64 and 32
 * actually diverge.  Let's keep it ugly for now.
 */
DEFINE_EARLY_PER_CPU_READ_MOSTLY(int, x86_cpu_to_logical_apicid, BAD_APICID);

/* Local APIC was disabled by the BIOS and enabled by the kernel */
static int enabled_via_apicbase;

/*
 * Handle interrupt mode configuration register (IMCR).
 * This register controls whether the interrupt signals
 * that reach the BSP come from the master PIC or from the
 * local APIC. Before entering Symmetric I/O Mode, either
 * the BIOS or the operating system must switch out of
 * PIC Mode by changing the IMCR.
 */
static inline void imcr_pic_to_apic(void)
{
	/* select IMCR register */
	outb(0x70, 0x22);
	/* NMI and 8259 INTR go through APIC */
	outb(0x01, 0x23);
}
static int cpuid_to_apicid[] = {
	[0 ... NR_CPUS - 1] = -1,
};

/**
 * apic_id_is_primary_thread - Check whether APIC ID belongs to a primary thread
 * @id:	APIC ID to check
 */
bool apic_id_is_primary_thread(unsigned int apicid)
{
	u32 mask;

	if (smp_num_siblings == 1)
		return true;
	/* Isolate the SMT bit(s) in the APICID and check for 0 */
	mask = (1U << (fls(smp_num_siblings) - 1)) - 1;
	return !(apicid & mask);
}
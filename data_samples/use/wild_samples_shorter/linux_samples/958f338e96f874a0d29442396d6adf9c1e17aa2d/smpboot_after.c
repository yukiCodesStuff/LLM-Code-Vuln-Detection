#include <asm/intel-family.h>
#include <asm/cpu_device_id.h>
#include <asm/spec-ctrl.h>
#include <asm/hw_irq.h>

/* representing HT siblings of each logical CPU */
DEFINE_PER_CPU_READ_MOSTLY(cpumask_var_t, cpu_sibling_map);
EXPORT_PER_CPU_SYMBOL(cpu_sibling_map);
	cpu_startup_entry(CPUHP_AP_ONLINE_IDLE);
}

/**
 * topology_is_primary_thread - Check whether CPU is the primary SMT thread
 * @cpu:	CPU to check
 */
bool topology_is_primary_thread(unsigned int cpu)
{
	return apic_id_is_primary_thread(per_cpu(x86_cpu_to_apicid, cpu));
}

/**
 * topology_smt_supported - Check whether SMT is supported by the CPUs
 */
bool topology_smt_supported(void)
{
	return smp_num_siblings > 1;
}

/**
 * topology_phys_to_logical_pkg - Map a physical package id to a logical
 *
 * Returns logical package id or -1 if not found
#include <asm/intel-family.h>
#include <asm/cpu_device_id.h>
#include <asm/spec-ctrl.h>

/* representing HT siblings of each logical CPU */
DEFINE_PER_CPU_READ_MOSTLY(cpumask_var_t, cpu_sibling_map);
EXPORT_PER_CPU_SYMBOL(cpu_sibling_map);
	cpu_startup_entry(CPUHP_AP_ONLINE_IDLE);
}

/**
 * topology_phys_to_logical_pkg - Map a physical package id to a logical
 *
 * Returns logical package id or -1 if not found
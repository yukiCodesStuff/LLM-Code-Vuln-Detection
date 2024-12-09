#include <asm/hypervisor.h>
#include <asm/cpu_device_id.h>
#include <asm/intel-family.h>
#include <asm/irq_regs.h>

unsigned int num_processors;

unsigned disabled_cpus;
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

/*
 * Should use this API to allocate logical CPU IDs to keep nr_logical_cpuids
 * and cpuid_to_apicid[] synchronized.
 */
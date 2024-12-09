#include <asm/hypervisor.h>
#include <asm/cpu_device_id.h>
#include <asm/intel-family.h>

unsigned int num_processors;

unsigned disabled_cpus;
	[0 ... NR_CPUS - 1] = -1,
};

/*
 * Should use this API to allocate logical CPU IDs to keep nr_logical_cpuids
 * and cpuid_to_apicid[] synchronized.
 */
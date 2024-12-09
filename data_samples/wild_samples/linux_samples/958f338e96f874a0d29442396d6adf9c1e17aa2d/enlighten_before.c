#ifdef CONFIG_XEN_BALLOON_MEMORY_HOTPLUG
#include <linux/bootmem.h>
#endif
#include <linux/cpu.h>
#include <linux/kexec.h>

#include <xen/features.h>
#include <xen/page.h>
#include <xen/interface/memory.h>

#include <asm/xen/hypercall.h>
#include <asm/xen/hypervisor.h>
#include <asm/cpu.h>
#include <asm/e820/api.h> 

#include "xen-ops.h"
#include "smp.h"
#include "pmu.h"

EXPORT_SYMBOL_GPL(hypercall_page);

/*
 * Pointer to the xen_vcpu_info structure or
 * &HYPERVISOR_shared_info->vcpu_info[cpu]. See xen_hvm_init_shared_info
 * and xen_vcpu_setup for details. By default it points to share_info->vcpu_info
 * but if the hypervisor supports VCPUOP_register_vcpu_info then it can point
 * to xen_vcpu_info. The pointer is used in __xen_evtchn_do_upcall to
 * acknowledge pending events.
 * Also more subtly it is used by the patched version of irq enable/disable
 * e.g. xen_irq_enable_direct and xen_iret in PV mode.
 *
 * The desire to be able to do those mask/unmask operations as a single
 * instruction by using the per-cpu offset held in %gs is the real reason
 * vcpu info is in a per-cpu pointer and the original reason for this
 * hypercall.
 *
 */
DEFINE_PER_CPU(struct vcpu_info *, xen_vcpu);

/*
 * Per CPU pages used if hypervisor supports VCPUOP_register_vcpu_info
 * hypercall. This can be used both in PV and PVHVM mode. The structure
 * overrides the default per_cpu(xen_vcpu, cpu) value.
 */
DEFINE_PER_CPU(struct vcpu_info, xen_vcpu_info);

/* Linux <-> Xen vCPU id mapping */
DEFINE_PER_CPU(uint32_t, xen_vcpu_id);
EXPORT_PER_CPU_SYMBOL(xen_vcpu_id);

enum xen_domain_type xen_domain_type = XEN_NATIVE;
EXPORT_SYMBOL_GPL(xen_domain_type);

unsigned long *machine_to_phys_mapping = (void *)MACH2PHYS_VIRT_START;
EXPORT_SYMBOL(machine_to_phys_mapping);
unsigned long  machine_to_phys_nr;
EXPORT_SYMBOL(machine_to_phys_nr);

struct start_info *xen_start_info;
EXPORT_SYMBOL_GPL(xen_start_info);

struct shared_info xen_dummy_shared_info;

__read_mostly int xen_have_vector_callback;
EXPORT_SYMBOL_GPL(xen_have_vector_callback);

/*
 * NB: needs to live in .data because it's used by xen_prepare_pvh which runs
 * before clearing the bss.
 */
uint32_t xen_start_flags __attribute__((section(".data"))) = 0;
EXPORT_SYMBOL(xen_start_flags);

/*
 * Point at some empty memory to start with. We map the real shared_info
 * page as soon as fixmap is up and running.
 */
struct shared_info *HYPERVISOR_shared_info = &xen_dummy_shared_info;

/*
 * Flag to determine whether vcpu info placement is available on all
 * VCPUs.  We assume it is to start with, and then set it to zero on
 * the first failure.  This is because it can succeed on some VCPUs
 * and not others, since it can involve hypervisor memory allocation,
 * or because the guest failed to guarantee all the appropriate
 * constraints on all VCPUs (ie buffer can't cross a page boundary).
 *
 * Note that any particular CPU may be using a placed vcpu structure,
 * but we can only optimise if the all are.
 *
 * 0: not available, 1: available
 */
int xen_have_vcpu_info_placement = 1;

static int xen_cpu_up_online(unsigned int cpu)
{
	xen_init_lock_cpu(cpu);
	return 0;
}
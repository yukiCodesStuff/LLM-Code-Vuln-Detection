#include <xen/interface/physdev.h>
#include <xen/interface/vcpu.h>
#include <xen/interface/memory.h>
#include <xen/interface/nmi.h>
#include <xen/interface/xen-mca.h>
#include <xen/features.h>
#include <xen/page.h>
#include <xen/hvm.h>
#include <asm/reboot.h>
#include <asm/stackprotector.h>
#include <asm/hypervisor.h>
#include <asm/mach_traps.h>
#include <asm/mwait.h>
#include <asm/pci_x86.h>
#include <asm/pat.h>

	.emergency_restart = xen_emergency_restart,
};

static unsigned char xen_get_nmi_reason(void)
{
	unsigned char reason = 0;

	/* Construct a value which looks like it came from port 0x61. */
	if (test_bit(_XEN_NMIREASON_io_error,
		     &HYPERVISOR_shared_info->arch.nmi_reason))
		reason |= NMI_REASON_IOCHK;
	if (test_bit(_XEN_NMIREASON_pci_serr,
		     &HYPERVISOR_shared_info->arch.nmi_reason))
		reason |= NMI_REASON_SERR;

	return reason;
}

static void __init xen_boot_params_init_edd(void)
{
#if IS_ENABLED(CONFIG_EDD)
	struct xen_platform_op op;
	pv_info = xen_info;
	pv_init_ops = xen_init_ops;
	pv_apic_ops = xen_apic_ops;
	if (!xen_pvh_domain()) {
		pv_cpu_ops = xen_cpu_ops;

		x86_platform.get_nmi_reason = xen_get_nmi_reason;
	}

	if (xen_feature(XENFEAT_auto_translated_physmap))
		x86_init.resources.memory_setup = xen_auto_xlated_memory_setup;
	else
		x86_init.resources.memory_setup = xen_memory_setup;
#endif
}

#ifdef CONFIG_XEN_PVHVM
#define HVM_SHARED_INFO_ADDR 0xFE700000UL
static struct shared_info *xen_hvm_shared_info;
static unsigned long xen_hvm_sip_phys;
static int xen_major, xen_minor;

static void xen_hvm_connect_shared_info(unsigned long pfn)
{
	struct xen_add_to_physmap xatp;

	xatp.domid = DOMID_SELF;
	xatp.idx = 0;
	xatp.space = XENMAPSPACE_shared_info;
	xatp.gpfn = pfn;
	if (HYPERVISOR_memory_op(XENMEM_add_to_physmap, &xatp))
		BUG();

}
static void __init xen_hvm_set_shared_info(struct shared_info *sip)
{
	int cpu;

	HYPERVISOR_shared_info = sip;

	/* xen_vcpu is a pointer to the vcpu_info struct in the shared_info
	 * page, we use it in the event channel upcall and in some pvclock
	 * related functions. We don't need the vcpu_info placement
	 * optimizations because we don't use any pv_mmu or pv_irq op on
	 * HVM. */
	for_each_online_cpu(cpu)
		per_cpu(xen_vcpu, cpu) = &HYPERVISOR_shared_info->vcpu_info[cpu];
}

/* Reconnect the shared_info pfn to a (new) mfn */
void xen_hvm_resume_shared_info(void)
{
	xen_hvm_connect_shared_info(xen_hvm_sip_phys >> PAGE_SHIFT);
}

/* Xen tools prior to Xen 4 do not provide a E820_Reserved area for guest usage.
 * On these old tools the shared info page will be placed in E820_Ram.
 * Xen 4 provides a E820_Reserved area at 0xFC000000, and this code expects
 * that nothing is mapped up to HVM_SHARED_INFO_ADDR.
 * Xen 4.3+ provides an explicit 1MB area at HVM_SHARED_INFO_ADDR which is used
 * here for the shared info page. */
static void __init xen_hvm_init_shared_info(void)
{
	if (xen_major < 4) {
		xen_hvm_shared_info = extend_brk(PAGE_SIZE, PAGE_SIZE);
		xen_hvm_sip_phys = __pa(xen_hvm_shared_info);
	} else {
		xen_hvm_sip_phys = HVM_SHARED_INFO_ADDR;
		set_fixmap(FIX_PARAVIRT_BOOTMAP, xen_hvm_sip_phys);
		xen_hvm_shared_info =
		(struct shared_info *)fix_to_virt(FIX_PARAVIRT_BOOTMAP);
	}
	xen_hvm_connect_shared_info(xen_hvm_sip_phys >> PAGE_SHIFT);
	xen_hvm_set_shared_info(xen_hvm_shared_info);
}

static void __init init_hvm_pv_info(void)
{
	uint32_t ecx, edx, pages, msr, base;
	u64 pfn;

	base = xen_cpuid_base();
	cpuid(base + 2, &pages, &msr, &ecx, &edx);

	pfn = __pa(hypercall_page);
	wrmsr_safe(msr, (u32)pfn, (u32)(pfn >> 32));

static bool __init xen_hvm_platform(void)
{
	uint32_t eax, ebx, ecx, edx, base;

	if (xen_pv_domain())
		return false;

	base = xen_cpuid_base();
	if (!base)
		return false;

	cpuid(base + 1, &eax, &ebx, &ecx, &edx);

	xen_major = eax >> 16;
	xen_minor = eax & 0xffff;

	printk(KERN_INFO "Xen version %d.%d.\n", xen_major, xen_minor);

	return true;
}

bool xen_hvm_need_lapic(void)
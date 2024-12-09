{
	u32 cus_per_node;

	if (c->x86 >= 0x17)
		return;

	cus_per_node = c->x86_max_cores / nodes_per_socket;
	c->cpu_core_id %= cus_per_node;
}


static void amd_get_topology_early(struct cpuinfo_x86 *c)
{
	if (cpu_has(c, X86_FEATURE_TOPOEXT))
		smp_num_siblings = ((cpuid_ebx(0x8000001e) >> 8) & 0xff) + 1;
}

/*
 * Fixup core topology information for
 * (1) AMD multi-node processors
 *     Assumption: Number of cores in each internal node is the same.
 * (2) AMD processors supporting compute units
 */
static void amd_get_topology(struct cpuinfo_x86 *c)
{
	u8 node_id;
	int cpu = smp_processor_id();

	/* get information required for multi-node processors */
	if (boot_cpu_has(X86_FEATURE_TOPOEXT)) {
		int err;
		u32 eax, ebx, ecx, edx;

		cpuid(0x8000001e, &eax, &ebx, &ecx, &edx);

		node_id  = ecx & 0xff;

		if (c->x86 == 0x15)
			c->cu_id = ebx & 0xff;

		if (c->x86 >= 0x17) {
			c->cpu_core_id = ebx & 0xff;

			if (smp_num_siblings > 1)
				c->x86_max_cores /= smp_num_siblings;
		}

		/*
		 * In case leaf B is available, use it to derive
		 * topology information.
		 */
		err = detect_extended_topology(c);
		if (!err)
			c->x86_coreid_bits = get_count_order(c->x86_max_cores);

		cacheinfo_amd_init_llc_id(c, cpu, node_id);

	} else if (cpu_has(c, X86_FEATURE_NODEID_MSR)) {
		u64 value;

		rdmsrl(MSR_FAM10H_NODE_ID, value);
		node_id = value & 7;

		per_cpu(cpu_llc_id, cpu) = node_id;
	} else
		return;

	if (nodes_per_socket > 1) {
		set_cpu_cap(c, X86_FEATURE_AMD_DCM);
		legacy_fixup_core_id(c);
	}
}

/*
 * On a AMD dual core setup the lower bits of the APIC id distinguish the cores.
 * Assumes number of cores is a power of two.
 */
static void amd_detect_cmp(struct cpuinfo_x86 *c)
{
	unsigned bits;
	int cpu = smp_processor_id();

	bits = c->x86_coreid_bits;
	/* Low order bits define the core id (index of core in socket) */
	c->cpu_core_id = c->initial_apicid & ((1 << bits)-1);
	/* Convert the initial APIC ID into the socket ID */
	c->phys_proc_id = c->initial_apicid >> bits;
	/* use socket ID also for last level cache */
	per_cpu(cpu_llc_id, cpu) = c->phys_proc_id;
}

u16 amd_get_nb_id(int cpu)
{
	return per_cpu(cpu_llc_id, cpu);
}
EXPORT_SYMBOL_GPL(amd_get_nb_id);

u32 amd_get_nodes_per_socket(void)
{
	return nodes_per_socket;
}
EXPORT_SYMBOL_GPL(amd_get_nodes_per_socket);

static void srat_detect_node(struct cpuinfo_x86 *c)
{
#ifdef CONFIG_NUMA
	int cpu = smp_processor_id();
	int node;
	unsigned apicid = c->apicid;

	node = numa_cpu_node(cpu);
	if (node == NUMA_NO_NODE)
		node = per_cpu(cpu_llc_id, cpu);

	/*
	 * On multi-fabric platform (e.g. Numascale NumaChip) a
	 * platform-specific handler needs to be called to fixup some
	 * IDs of the CPU.
	 */
	if (x86_cpuinit.fixup_cpu_id)
		x86_cpuinit.fixup_cpu_id(c, node);

	if (!node_online(node)) {
		/*
		 * Two possibilities here:
		 *
		 * - The CPU is missing memory and no node was created.  In
		 *   that case try picking one from a nearby CPU.
		 *
		 * - The APIC IDs differ from the HyperTransport node IDs
		 *   which the K8 northbridge parsing fills in.  Assume
		 *   they are all increased by a constant offset, but in
		 *   the same order as the HT nodeids.  If that doesn't
		 *   result in a usable node fall back to the path for the
		 *   previous case.
		 *
		 * This workaround operates directly on the mapping between
		 * APIC ID and NUMA node, assuming certain relationship
		 * between APIC ID, HT node ID and NUMA topology.  As going
		 * through CPU mapping may alter the outcome, directly
		 * access __apicid_to_node[].
		 */
		int ht_nodeid = c->initial_apicid;

		if (__apicid_to_node[ht_nodeid] != NUMA_NO_NODE)
			node = __apicid_to_node[ht_nodeid];
		/* Pick a nearby node */
		if (!node_online(node))
			node = nearby_node(apicid);
	}
	numa_set_node(cpu, node);
#endif
}

static void early_init_amd_mc(struct cpuinfo_x86 *c)
{
#ifdef CONFIG_SMP
	unsigned bits, ecx;

	/* Multi core CPU? */
	if (c->extended_cpuid_level < 0x80000008)
		return;

	ecx = cpuid_ecx(0x80000008);

	c->x86_max_cores = (ecx & 0xff) + 1;

	/* CPU telling us the core id bits shift? */
	bits = (ecx >> 12) & 0xF;

	/* Otherwise recompute */
	if (bits == 0) {
		while ((1 << bits) < c->x86_max_cores)
			bits++;
	}

	c->x86_coreid_bits = bits;
#endif
}

static void bsp_init_amd(struct cpuinfo_x86 *c)
{

#ifdef CONFIG_X86_64
	if (c->x86 >= 0xf) {
		unsigned long long tseg;

		/*
		 * Split up direct mapping around the TSEG SMM area.
		 * Don't do it for gbpages because there seems very little
		 * benefit in doing so.
		 */
		if (!rdmsrl_safe(MSR_K8_TSEG_ADDR, &tseg)) {
			unsigned long pfn = tseg >> PAGE_SHIFT;

			pr_debug("tseg: %010llx\n", tseg);
			if (pfn_range_is_mapped(pfn, pfn + 1))
				set_memory_4k((unsigned long)__va(tseg), 1);
		}
	}
#endif

	if (cpu_has(c, X86_FEATURE_CONSTANT_TSC)) {

		if (c->x86 > 0x10 ||
		    (c->x86 == 0x10 && c->x86_model >= 0x2)) {
			u64 val;

			rdmsrl(MSR_K7_HWCR, val);
			if (!(val & BIT(24)))
				pr_warn(FW_BUG "TSC doesn't count with P0 frequency!\n");
		}
	}

	if (c->x86 == 0x15) {
		unsigned long upperbit;
		u32 cpuid, assoc;

		cpuid	 = cpuid_edx(0x80000005);
		assoc	 = cpuid >> 16 & 0xff;
		upperbit = ((cpuid >> 24) << 10) / assoc;

		va_align.mask	  = (upperbit - 1) & PAGE_MASK;
		va_align.flags    = ALIGN_VA_32 | ALIGN_VA_64;

		/* A random value per boot for bit slice [12:upper_bit) */
		va_align.bits = get_random_int() & va_align.mask;
	}

	if (cpu_has(c, X86_FEATURE_MWAITX))
		use_mwaitx_delay();

	if (boot_cpu_has(X86_FEATURE_TOPOEXT)) {
		u32 ecx;

		ecx = cpuid_ecx(0x8000001e);
		nodes_per_socket = ((ecx >> 8) & 7) + 1;
	} else if (boot_cpu_has(X86_FEATURE_NODEID_MSR)) {
		u64 value;

		rdmsrl(MSR_FAM10H_NODE_ID, value);
		nodes_per_socket = ((value >> 3) & 7) + 1;
	}

	if (!boot_cpu_has(X86_FEATURE_AMD_SSBD) &&
	    !boot_cpu_has(X86_FEATURE_VIRT_SSBD) &&
	    c->x86 >= 0x15 && c->x86 <= 0x17) {
		unsigned int bit;

		switch (c->x86) {
		case 0x15: bit = 54; break;
		case 0x16: bit = 33; break;
		case 0x17: bit = 10; break;
		default: return;
		}
		/*
		 * Try to cache the base value so further operations can
		 * avoid RMW. If that faults, do not enable SSBD.
		 */
		if (!rdmsrl_safe(MSR_AMD64_LS_CFG, &x86_amd_ls_cfg_base)) {
			setup_force_cpu_cap(X86_FEATURE_LS_CFG_SSBD);
			setup_force_cpu_cap(X86_FEATURE_SSBD);
			x86_amd_ls_cfg_ssbd_mask = 1ULL << bit;
		}
	}
}

static void early_detect_mem_encrypt(struct cpuinfo_x86 *c)
{
	u64 msr;

	/*
	 * BIOS support is required for SME and SEV.
	 *   For SME: If BIOS has enabled SME then adjust x86_phys_bits by
	 *	      the SME physical address space reduction value.
	 *	      If BIOS has not enabled SME then don't advertise the
	 *	      SME feature (set in scattered.c).
	 *   For SEV: If BIOS has not enabled SEV then don't advertise the
	 *            SEV feature (set in scattered.c).
	 *
	 *   In all cases, since support for SME and SEV requires long mode,
	 *   don't advertise the feature under CONFIG_X86_32.
	 */
	if (cpu_has(c, X86_FEATURE_SME) || cpu_has(c, X86_FEATURE_SEV)) {
		/* Check if memory encryption is enabled */
		rdmsrl(MSR_K8_SYSCFG, msr);
		if (!(msr & MSR_K8_SYSCFG_MEM_ENCRYPT))
			goto clear_all;

		/*
		 * Always adjust physical address bits. Even though this
		 * will be a value above 32-bits this is still done for
		 * CONFIG_X86_32 so that accurate values are reported.
		 */
		c->x86_phys_bits -= (cpuid_ebx(0x8000001f) >> 6) & 0x3f;

		if (IS_ENABLED(CONFIG_X86_32))
			goto clear_all;

		rdmsrl(MSR_K7_HWCR, msr);
		if (!(msr & MSR_K7_HWCR_SMMLOCK))
			goto clear_sev;

		return;

clear_all:
		clear_cpu_cap(c, X86_FEATURE_SME);
clear_sev:
		clear_cpu_cap(c, X86_FEATURE_SEV);
	}
}

static void early_init_amd(struct cpuinfo_x86 *c)
{
	u64 value;
	u32 dummy;

	early_init_amd_mc(c);

#ifdef CONFIG_X86_32
	if (c->x86 == 6)
		set_cpu_cap(c, X86_FEATURE_K7);
#endif

	if (c->x86 >= 0xf)
		set_cpu_cap(c, X86_FEATURE_K8);

	rdmsr_safe(MSR_AMD64_PATCH_LEVEL, &c->microcode, &dummy);

	/*
	 * c->x86_power is 8000_0007 edx. Bit 8 is TSC runs at constant rate
	 * with P/T states and does not stop in deep C-states
	 */
	if (c->x86_power & (1 << 8)) {
		set_cpu_cap(c, X86_FEATURE_CONSTANT_TSC);
		set_cpu_cap(c, X86_FEATURE_NONSTOP_TSC);
	}

	/* Bit 12 of 8000_0007 edx is accumulated power mechanism. */
	if (c->x86_power & BIT(12))
		set_cpu_cap(c, X86_FEATURE_ACC_POWER);

#ifdef CONFIG_X86_64
	set_cpu_cap(c, X86_FEATURE_SYSCALL32);
#else
	/*  Set MTRR capability flag if appropriate */
	if (c->x86 == 5)
		if (c->x86_model == 13 || c->x86_model == 9 ||
		    (c->x86_model == 8 && c->x86_stepping >= 8))
			set_cpu_cap(c, X86_FEATURE_K6_MTRR);
#endif
#if defined(CONFIG_X86_LOCAL_APIC) && defined(CONFIG_PCI)
	/*
	 * ApicID can always be treated as an 8-bit value for AMD APIC versions
	 * >= 0x10, but even old K8s came out of reset with version 0x10. So, we
	 * can safely set X86_FEATURE_EXTD_APICID unconditionally for families
	 * after 16h.
	 */
	if (boot_cpu_has(X86_FEATURE_APIC)) {
		if (c->x86 > 0x16)
			set_cpu_cap(c, X86_FEATURE_EXTD_APICID);
		else if (c->x86 >= 0xf) {
			/* check CPU config space for extended APIC ID */
			unsigned int val;

			val = read_pci_config(0, 24, 0, 0x68);
			if ((val >> 17 & 0x3) == 0x3)
				set_cpu_cap(c, X86_FEATURE_EXTD_APICID);
		}
	}
#endif

	/*
	 * This is only needed to tell the kernel whether to use VMCALL
	 * and VMMCALL.  VMMCALL is never executed except under virt, so
	 * we can set it unconditionally.
	 */
	set_cpu_cap(c, X86_FEATURE_VMMCALL);

	/* F16h erratum 793, CVE-2013-6885 */
	if (c->x86 == 0x16 && c->x86_model <= 0xf)
		msr_set_bit(MSR_AMD64_LS_CFG, 15);

	/*
	 * Check whether the machine is affected by erratum 400. This is
	 * used to select the proper idle routine and to enable the check
	 * whether the machine is affected in arch_post_acpi_init(), which
	 * sets the X86_BUG_AMD_APIC_C1E bug depending on the MSR check.
	 */
	if (cpu_has_amd_erratum(c, amd_erratum_400))
		set_cpu_bug(c, X86_BUG_AMD_E400);

	early_detect_mem_encrypt(c);

	/* Re-enable TopologyExtensions if switched off by BIOS */
	if (c->x86 == 0x15 &&
	    (c->x86_model >= 0x10 && c->x86_model <= 0x6f) &&
	    !cpu_has(c, X86_FEATURE_TOPOEXT)) {

		if (msr_set_bit(0xc0011005, 54) > 0) {
			rdmsrl(0xc0011005, value);
			if (value & BIT_64(54)) {
				set_cpu_cap(c, X86_FEATURE_TOPOEXT);
				pr_info_once(FW_INFO "CPU: Re-enabling disabled Topology Extensions Support.\n");
			}
		}
	}

	amd_get_topology_early(c);
}

static void init_amd_k8(struct cpuinfo_x86 *c)
{
	u32 level;
	u64 value;

	/* On C+ stepping K8 rep microcode works well for copy/memset */
	level = cpuid_eax(1);
	if ((level >= 0x0f48 && level < 0x0f50) || level >= 0x0f58)
		set_cpu_cap(c, X86_FEATURE_REP_GOOD);

	/*
	 * Some BIOSes incorrectly force this feature, but only K8 revision D
	 * (model = 0x14) and later actually support it.
	 * (AMD Erratum #110, docId: 25759).
	 */
	if (c->x86_model < 0x14 && cpu_has(c, X86_FEATURE_LAHF_LM)) {
		clear_cpu_cap(c, X86_FEATURE_LAHF_LM);
		if (!rdmsrl_amd_safe(0xc001100d, &value)) {
			value &= ~BIT_64(32);
			wrmsrl_amd_safe(0xc001100d, value);
		}
	}

	if (!c->x86_model_id[0])
		strcpy(c->x86_model_id, "Hammer");

#ifdef CONFIG_SMP
	/*
	 * Disable TLB flush filter by setting HWCR.FFDIS on K8
	 * bit 6 of msr C001_0015
	 *
	 * Errata 63 for SH-B3 steppings
	 * Errata 122 for all steppings (F+ have it disabled by default)
	 */
	msr_set_bit(MSR_K7_HWCR, 6);
#endif
	set_cpu_bug(c, X86_BUG_SWAPGS_FENCE);
}

static void init_amd_gh(struct cpuinfo_x86 *c)
{
#ifdef CONFIG_MMCONF_FAM10H
	/* do this for boot cpu */
	if (c == &boot_cpu_data)
		check_enable_amd_mmconf_dmi();

	fam10h_check_enable_mmcfg();
#endif

	/*
	 * Disable GART TLB Walk Errors on Fam10h. We do this here because this
	 * is always needed when GART is enabled, even in a kernel which has no
	 * MCE support built in. BIOS should disable GartTlbWlk Errors already.
	 * If it doesn't, we do it here as suggested by the BKDG.
	 *
	 * Fixes: https://bugzilla.kernel.org/show_bug.cgi?id=33012
	 */
	msr_set_bit(MSR_AMD64_MCx_MASK(4), 10);

	/*
	 * On family 10h BIOS may not have properly enabled WC+ support, causing
	 * it to be converted to CD memtype. This may result in performance
	 * degradation for certain nested-paging guests. Prevent this conversion
	 * by clearing bit 24 in MSR_AMD64_BU_CFG2.
	 *
	 * NOTE: we want to use the _safe accessors so as not to #GP kvm
	 * guests on older kvm hosts.
	 */
	msr_clear_bit(MSR_AMD64_BU_CFG2, 24);

	if (cpu_has_amd_erratum(c, amd_erratum_383))
		set_cpu_bug(c, X86_BUG_AMD_TLB_MMATCH);
}

#define MSR_AMD64_DE_CFG	0xC0011029

static void init_amd_ln(struct cpuinfo_x86 *c)
{
	/*
	 * Apply erratum 665 fix unconditionally so machines without a BIOS
	 * fix work.
	 */
	msr_set_bit(MSR_AMD64_DE_CFG, 31);
}

static void init_amd_bd(struct cpuinfo_x86 *c)
{
	u64 value;

	/*
	 * The way access filter has a performance penalty on some workloads.
	 * Disable it on the affected CPUs.
	 */
	if ((c->x86_model >= 0x02) && (c->x86_model < 0x20)) {
		if (!rdmsrl_safe(MSR_F15H_IC_CFG, &value) && !(value & 0x1E)) {
			value |= 0x1E;
			wrmsrl_safe(MSR_F15H_IC_CFG, value);
		}
	}
}

static void init_amd_zn(struct cpuinfo_x86 *c)
{
	set_cpu_cap(c, X86_FEATURE_ZEN);
	/*
	 * Fix erratum 1076: CPB feature bit not being set in CPUID. It affects
	 * all up to and including B1.
	 */
	if (c->x86_model <= 1 && c->x86_stepping <= 1)
		set_cpu_cap(c, X86_FEATURE_CPB);
}

static void init_amd(struct cpuinfo_x86 *c)
{
	early_init_amd(c);

	/*
	 * Bit 31 in normal CPUID used for nonstandard 3DNow ID;
	 * 3DNow is IDd by bit 31 in extended CPUID (1*32+31) anyway
	 */
	clear_cpu_cap(c, 0*32+31);

	if (c->x86 >= 0x10)
		set_cpu_cap(c, X86_FEATURE_REP_GOOD);

	/* get apicid instead of initial apic id from cpuid */
	c->apicid = hard_smp_processor_id();

	/* K6s reports MCEs but don't actually have all the MSRs */
	if (c->x86 < 6)
		clear_cpu_cap(c, X86_FEATURE_MCE);

	switch (c->x86) {
	case 4:    init_amd_k5(c); break;
	case 5:    init_amd_k6(c); break;
	case 6:	   init_amd_k7(c); break;
	case 0xf:  init_amd_k8(c); break;
	case 0x10: init_amd_gh(c); break;
	case 0x12: init_amd_ln(c); break;
	case 0x15: init_amd_bd(c); break;
	case 0x17: init_amd_zn(c); break;
	}

	/*
	 * Enable workaround for FXSAVE leak on CPUs
	 * without a XSaveErPtr feature
	 */
	if ((c->x86 >= 6) && (!cpu_has(c, X86_FEATURE_XSAVEERPTR)))
		set_cpu_bug(c, X86_BUG_FXSAVE_LEAK);

	cpu_detect_cache_sizes(c);

	amd_detect_cmp(c);
	amd_get_topology(c);
	srat_detect_node(c);

	init_amd_cacheinfo(c);

	if (cpu_has(c, X86_FEATURE_XMM2)) {
		unsigned long long val;
		int ret;

		/*
		 * A serializing LFENCE has less overhead than MFENCE, so
		 * use it for execution serialization.  On families which
		 * don't have that MSR, LFENCE is already serializing.
		 * msr_set_bit() uses the safe accessors, too, even if the MSR
		 * is not present.
		 */
		msr_set_bit(MSR_F10H_DECFG,
			    MSR_F10H_DECFG_LFENCE_SERIALIZE_BIT);

		/*
		 * Verify that the MSR write was successful (could be running
		 * under a hypervisor) and only then assume that LFENCE is
		 * serializing.
		 */
		ret = rdmsrl_safe(MSR_F10H_DECFG, &val);
		if (!ret && (val & MSR_F10H_DECFG_LFENCE_SERIALIZE)) {
			/* A serializing LFENCE stops RDTSC speculation */
			set_cpu_cap(c, X86_FEATURE_LFENCE_RDTSC);
		} else {
			/* MFENCE stops RDTSC speculation */
			set_cpu_cap(c, X86_FEATURE_MFENCE_RDTSC);
		}
	}

	/*
	 * Family 0x12 and above processors have APIC timer
	 * running in deep C states.
	 */
	if (c->x86 > 0x11)
		set_cpu_cap(c, X86_FEATURE_ARAT);

	/* 3DNow or LM implies PREFETCHW */
	if (!cpu_has(c, X86_FEATURE_3DNOWPREFETCH))
		if (cpu_has(c, X86_FEATURE_3DNOW) || cpu_has(c, X86_FEATURE_LM))
			set_cpu_cap(c, X86_FEATURE_3DNOWPREFETCH);

	/* AMD CPUs don't reset SS attributes on SYSRET, Xen does. */
	if (!cpu_has(c, X86_FEATURE_XENPV))
		set_cpu_bug(c, X86_BUG_SYSRET_SS_ATTRS);
}

#ifdef CONFIG_X86_32
static unsigned int amd_size_cache(struct cpuinfo_x86 *c, unsigned int size)
{
	/* AMD errata T13 (order #21922) */
	if ((c->x86 == 6)) {
		/* Duron Rev A0 */
		if (c->x86_model == 3 && c->x86_stepping == 0)
			size = 64;
		/* Tbird rev A1/A2 */
		if (c->x86_model == 4 &&
			(c->x86_stepping == 0 || c->x86_stepping == 1))
			size = 256;
	}
	return size;
}
#endif

static void cpu_detect_tlb_amd(struct cpuinfo_x86 *c)
{
	u32 ebx, eax, ecx, edx;
	u16 mask = 0xfff;

	if (c->x86 < 0xf)
		return;

	if (c->extended_cpuid_level < 0x80000006)
		return;

	cpuid(0x80000006, &eax, &ebx, &ecx, &edx);

	tlb_lld_4k[ENTRIES] = (ebx >> 16) & mask;
	tlb_lli_4k[ENTRIES] = ebx & mask;

	/*
	 * K8 doesn't have 2M/4M entries in the L2 TLB so read out the L1 TLB
	 * characteristics from the CPUID function 0x80000005 instead.
	 */
	if (c->x86 == 0xf) {
		cpuid(0x80000005, &eax, &ebx, &ecx, &edx);
		mask = 0xff;
	}

	/* Handle DTLB 2M and 4M sizes, fall back to L1 if L2 is disabled */
	if (!((eax >> 16) & mask))
		tlb_lld_2m[ENTRIES] = (cpuid_eax(0x80000005) >> 16) & 0xff;
	else
		tlb_lld_2m[ENTRIES] = (eax >> 16) & mask;

	/* a 4M entry uses two 2M entries */
	tlb_lld_4m[ENTRIES] = tlb_lld_2m[ENTRIES] >> 1;

	/* Handle ITLB 2M and 4M sizes, fall back to L1 if L2 is disabled */
	if (!(eax & mask)) {
		/* Erratum 658 */
		if (c->x86 == 0x15 && c->x86_model <= 0x1f) {
			tlb_lli_2m[ENTRIES] = 1024;
		} else {
			cpuid(0x80000005, &eax, &ebx, &ecx, &edx);
			tlb_lli_2m[ENTRIES] = eax & 0xff;
		}
	} else
		tlb_lli_2m[ENTRIES] = eax & mask;

	tlb_lli_4m[ENTRIES] = tlb_lli_2m[ENTRIES] >> 1;
}

static const struct cpu_dev amd_cpu_dev = {
	.c_vendor	= "AMD",
	.c_ident	= { "AuthenticAMD" },
#ifdef CONFIG_X86_32
	.legacy_models = {
		{ .family = 4, .model_names =
		  {
			  [3] = "486 DX/2",
			  [7] = "486 DX/2-WB",
			  [8] = "486 DX/4",
			  [9] = "486 DX/4-WB",
			  [14] = "Am5x86-WT",
			  [15] = "Am5x86-WB"
		  }
		},
	},
	.legacy_cache_size = amd_size_cache,
#endif
	.c_early_init   = early_init_amd,
	.c_detect_tlb	= cpu_detect_tlb_amd,
	.c_bsp_init	= bsp_init_amd,
	.c_init		= init_amd,
	.c_x86_vendor	= X86_VENDOR_AMD,
};

cpu_dev_register(amd_cpu_dev);

/*
 * AMD errata checking
 *
 * Errata are defined as arrays of ints using the AMD_LEGACY_ERRATUM() or
 * AMD_OSVW_ERRATUM() macros. The latter is intended for newer errata that
 * have an OSVW id assigned, which it takes as first argument. Both take a
 * variable number of family-specific model-stepping ranges created by
 * AMD_MODEL_RANGE().
 *
 * Example:
 *
 * const int amd_erratum_319[] =
 *	AMD_LEGACY_ERRATUM(AMD_MODEL_RANGE(0x10, 0x2, 0x1, 0x4, 0x2),
 *			   AMD_MODEL_RANGE(0x10, 0x8, 0x0, 0x8, 0x0),
 *			   AMD_MODEL_RANGE(0x10, 0x9, 0x0, 0x9, 0x0));
 */

#define AMD_LEGACY_ERRATUM(...)		{ -1, __VA_ARGS__, 0 }
#define AMD_OSVW_ERRATUM(osvw_id, ...)	{ osvw_id, __VA_ARGS__, 0 }
#define AMD_MODEL_RANGE(f, m_start, s_start, m_end, s_end) \
	((f << 24) | (m_start << 16) | (s_start << 12) | (m_end << 4) | (s_end))
#define AMD_MODEL_RANGE_FAMILY(range)	(((range) >> 24) & 0xff)
#define AMD_MODEL_RANGE_START(range)	(((range) >> 12) & 0xfff)
#define AMD_MODEL_RANGE_END(range)	((range) & 0xfff)

static const int amd_erratum_400[] =
	AMD_OSVW_ERRATUM(1, AMD_MODEL_RANGE(0xf, 0x41, 0x2, 0xff, 0xf),
			    AMD_MODEL_RANGE(0x10, 0x2, 0x1, 0xff, 0xf));

static const int amd_erratum_383[] =
	AMD_OSVW_ERRATUM(3, AMD_MODEL_RANGE(0x10, 0, 0, 0xff, 0xf));


static bool cpu_has_amd_erratum(struct cpuinfo_x86 *cpu, const int *erratum)
{
	int osvw_id = *erratum++;
	u32 range;
	u32 ms;

	if (osvw_id >= 0 && osvw_id < 65536 &&
	    cpu_has(cpu, X86_FEATURE_OSVW)) {
		u64 osvw_len;

		rdmsrl(MSR_AMD64_OSVW_ID_LENGTH, osvw_len);
		if (osvw_id < osvw_len) {
			u64 osvw_bits;

			rdmsrl(MSR_AMD64_OSVW_STATUS + (osvw_id >> 6),
			    osvw_bits);
			return osvw_bits & (1ULL << (osvw_id & 0x3f));
		}
	}

	/* OSVW unavailable or ID unknown, match family-model-stepping range */
	ms = (cpu->x86_model << 4) | cpu->x86_stepping;
	while ((range = *erratum++))
		if ((cpu->x86 == AMD_MODEL_RANGE_FAMILY(range)) &&
		    (ms >= AMD_MODEL_RANGE_START(range)) &&
		    (ms <= AMD_MODEL_RANGE_END(range)))
			return true;

	return false;
}

void set_dr_addr_mask(unsigned long mask, int dr)
{
	if (!boot_cpu_has(X86_FEATURE_BPEXT))
		return;

	switch (dr) {
	case 0:
		wrmsr(MSR_F16H_DR0_ADDR_MASK, mask, 0);
		break;
	case 1:
	case 2:
	case 3:
		wrmsr(MSR_F16H_DR1_ADDR_MASK - 1 + dr, mask, 0);
		break;
	default:
		break;
	}
}
	if (boot_cpu_has(X86_FEATURE_TOPOEXT)) {
		int err;
		u32 eax, ebx, ecx, edx;

		cpuid(0x8000001e, &eax, &ebx, &ecx, &edx);

		node_id  = ecx & 0xff;

		if (c->x86 == 0x15)
			c->cu_id = ebx & 0xff;

		if (c->x86 >= 0x17) {
			c->cpu_core_id = ebx & 0xff;

			if (smp_num_siblings > 1)
				c->x86_max_cores /= smp_num_siblings;
		}
	if (cpu_has(c, X86_FEATURE_SME) || cpu_has(c, X86_FEATURE_SEV)) {
		/* Check if memory encryption is enabled */
		rdmsrl(MSR_K8_SYSCFG, msr);
		if (!(msr & MSR_K8_SYSCFG_MEM_ENCRYPT))
			goto clear_all;

		/*
		 * Always adjust physical address bits. Even though this
		 * will be a value above 32-bits this is still done for
		 * CONFIG_X86_32 so that accurate values are reported.
		 */
		c->x86_phys_bits -= (cpuid_ebx(0x8000001f) >> 6) & 0x3f;

		if (IS_ENABLED(CONFIG_X86_32))
			goto clear_all;

		rdmsrl(MSR_K7_HWCR, msr);
		if (!(msr & MSR_K7_HWCR_SMMLOCK))
			goto clear_sev;

		return;

clear_all:
		clear_cpu_cap(c, X86_FEATURE_SME);
clear_sev:
		clear_cpu_cap(c, X86_FEATURE_SEV);
	}
}

static void early_init_amd(struct cpuinfo_x86 *c)
{
	u64 value;
	u32 dummy;

	early_init_amd_mc(c);

#ifdef CONFIG_X86_32
	if (c->x86 == 6)
		set_cpu_cap(c, X86_FEATURE_K7);
#endif

	if (c->x86 >= 0xf)
		set_cpu_cap(c, X86_FEATURE_K8);

	rdmsr_safe(MSR_AMD64_PATCH_LEVEL, &c->microcode, &dummy);

	/*
	 * c->x86_power is 8000_0007 edx. Bit 8 is TSC runs at constant rate
	 * with P/T states and does not stop in deep C-states
	 */
	if (c->x86_power & (1 << 8)) {
		set_cpu_cap(c, X86_FEATURE_CONSTANT_TSC);
		set_cpu_cap(c, X86_FEATURE_NONSTOP_TSC);
	}

	/* Bit 12 of 8000_0007 edx is accumulated power mechanism. */
	if (c->x86_power & BIT(12))
		set_cpu_cap(c, X86_FEATURE_ACC_POWER);

#ifdef CONFIG_X86_64
	set_cpu_cap(c, X86_FEATURE_SYSCALL32);
#else
	/*  Set MTRR capability flag if appropriate */
	if (c->x86 == 5)
		if (c->x86_model == 13 || c->x86_model == 9 ||
		    (c->x86_model == 8 && c->x86_stepping >= 8))
			set_cpu_cap(c, X86_FEATURE_K6_MTRR);
#endif
#if defined(CONFIG_X86_LOCAL_APIC) && defined(CONFIG_PCI)
	/*
	 * ApicID can always be treated as an 8-bit value for AMD APIC versions
	 * >= 0x10, but even old K8s came out of reset with version 0x10. So, we
	 * can safely set X86_FEATURE_EXTD_APICID unconditionally for families
	 * after 16h.
	 */
	if (boot_cpu_has(X86_FEATURE_APIC)) {
		if (c->x86 > 0x16)
			set_cpu_cap(c, X86_FEATURE_EXTD_APICID);
		else if (c->x86 >= 0xf) {
			/* check CPU config space for extended APIC ID */
			unsigned int val;

			val = read_pci_config(0, 24, 0, 0x68);
			if ((val >> 17 & 0x3) == 0x3)
				set_cpu_cap(c, X86_FEATURE_EXTD_APICID);
		}
	}
#endif

	/*
	 * This is only needed to tell the kernel whether to use VMCALL
	 * and VMMCALL.  VMMCALL is never executed except under virt, so
	 * we can set it unconditionally.
	 */
	set_cpu_cap(c, X86_FEATURE_VMMCALL);

	/* F16h erratum 793, CVE-2013-6885 */
	if (c->x86 == 0x16 && c->x86_model <= 0xf)
		msr_set_bit(MSR_AMD64_LS_CFG, 15);

	/*
	 * Check whether the machine is affected by erratum 400. This is
	 * used to select the proper idle routine and to enable the check
	 * whether the machine is affected in arch_post_acpi_init(), which
	 * sets the X86_BUG_AMD_APIC_C1E bug depending on the MSR check.
	 */
	if (cpu_has_amd_erratum(c, amd_erratum_400))
		set_cpu_bug(c, X86_BUG_AMD_E400);

	early_detect_mem_encrypt(c);

	/* Re-enable TopologyExtensions if switched off by BIOS */
	if (c->x86 == 0x15 &&
	    (c->x86_model >= 0x10 && c->x86_model <= 0x6f) &&
	    !cpu_has(c, X86_FEATURE_TOPOEXT)) {

		if (msr_set_bit(0xc0011005, 54) > 0) {
			rdmsrl(0xc0011005, value);
			if (value & BIT_64(54)) {
				set_cpu_cap(c, X86_FEATURE_TOPOEXT);
				pr_info_once(FW_INFO "CPU: Re-enabling disabled Topology Extensions Support.\n");
			}
		}
	}

	amd_get_topology_early(c);
}
		else if (c->x86 >= 0xf) {
			/* check CPU config space for extended APIC ID */
			unsigned int val;

			val = read_pci_config(0, 24, 0, 0x68);
			if ((val >> 17 & 0x3) == 0x3)
				set_cpu_cap(c, X86_FEATURE_EXTD_APICID);
		}
	}
#endif

	/*
	 * This is only needed to tell the kernel whether to use VMCALL
	 * and VMMCALL.  VMMCALL is never executed except under virt, so
	 * we can set it unconditionally.
	 */
	set_cpu_cap(c, X86_FEATURE_VMMCALL);

	/* F16h erratum 793, CVE-2013-6885 */
	if (c->x86 == 0x16 && c->x86_model <= 0xf)
		msr_set_bit(MSR_AMD64_LS_CFG, 15);

	/*
	 * Check whether the machine is affected by erratum 400. This is
	 * used to select the proper idle routine and to enable the check
	 * whether the machine is affected in arch_post_acpi_init(), which
	 * sets the X86_BUG_AMD_APIC_C1E bug depending on the MSR check.
	 */
	if (cpu_has_amd_erratum(c, amd_erratum_400))
		set_cpu_bug(c, X86_BUG_AMD_E400);

	early_detect_mem_encrypt(c);

	/* Re-enable TopologyExtensions if switched off by BIOS */
	if (c->x86 == 0x15 &&
	    (c->x86_model >= 0x10 && c->x86_model <= 0x6f) &&
	    !cpu_has(c, X86_FEATURE_TOPOEXT)) {

		if (msr_set_bit(0xc0011005, 54) > 0) {
			rdmsrl(0xc0011005, value);
			if (value & BIT_64(54)) {
				set_cpu_cap(c, X86_FEATURE_TOPOEXT);
				pr_info_once(FW_INFO "CPU: Re-enabling disabled Topology Extensions Support.\n");
			}
		}
	}
{
	u64 value;

	/*
	 * The way access filter has a performance penalty on some workloads.
	 * Disable it on the affected CPUs.
	 */
	if ((c->x86_model >= 0x02) && (c->x86_model < 0x20)) {
		if (!rdmsrl_safe(MSR_F15H_IC_CFG, &value) && !(value & 0x1E)) {
			value |= 0x1E;
			wrmsrl_safe(MSR_F15H_IC_CFG, value);
		}
	}
	switch (c->x86) {
	case 4:    init_amd_k5(c); break;
	case 5:    init_amd_k6(c); break;
	case 6:	   init_amd_k7(c); break;
	case 0xf:  init_amd_k8(c); break;
	case 0x10: init_amd_gh(c); break;
	case 0x12: init_amd_ln(c); break;
	case 0x15: init_amd_bd(c); break;
	case 0x17: init_amd_zn(c); break;
	}

	/*
	 * Enable workaround for FXSAVE leak on CPUs
	 * without a XSaveErPtr feature
	 */
	if ((c->x86 >= 6) && (!cpu_has(c, X86_FEATURE_XSAVEERPTR)))
		set_cpu_bug(c, X86_BUG_FXSAVE_LEAK);

	cpu_detect_cache_sizes(c);

	amd_detect_cmp(c);
	amd_get_topology(c);
	srat_detect_node(c);

	init_amd_cacheinfo(c);

	if (cpu_has(c, X86_FEATURE_XMM2)) {
		unsigned long long val;
		int ret;

		/*
		 * A serializing LFENCE has less overhead than MFENCE, so
		 * use it for execution serialization.  On families which
		 * don't have that MSR, LFENCE is already serializing.
		 * msr_set_bit() uses the safe accessors, too, even if the MSR
		 * is not present.
		 */
		msr_set_bit(MSR_F10H_DECFG,
			    MSR_F10H_DECFG_LFENCE_SERIALIZE_BIT);

		/*
		 * Verify that the MSR write was successful (could be running
		 * under a hypervisor) and only then assume that LFENCE is
		 * serializing.
		 */
		ret = rdmsrl_safe(MSR_F10H_DECFG, &val);
		if (!ret && (val & MSR_F10H_DECFG_LFENCE_SERIALIZE)) {
			/* A serializing LFENCE stops RDTSC speculation */
			set_cpu_cap(c, X86_FEATURE_LFENCE_RDTSC);
		} else {
			/* MFENCE stops RDTSC speculation */
			set_cpu_cap(c, X86_FEATURE_MFENCE_RDTSC);
		}
	}

	/*
	 * Family 0x12 and above processors have APIC timer
	 * running in deep C states.
	 */
	if (c->x86 > 0x11)
		set_cpu_cap(c, X86_FEATURE_ARAT);

	/* 3DNow or LM implies PREFETCHW */
	if (!cpu_has(c, X86_FEATURE_3DNOWPREFETCH))
		if (cpu_has(c, X86_FEATURE_3DNOW) || cpu_has(c, X86_FEATURE_LM))
			set_cpu_cap(c, X86_FEATURE_3DNOWPREFETCH);

	/* AMD CPUs don't reset SS attributes on SYSRET, Xen does. */
	if (!cpu_has(c, X86_FEATURE_XENPV))
		set_cpu_bug(c, X86_BUG_SYSRET_SS_ATTRS);
}
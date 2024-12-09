		tlb_lld_4m[ENTRIES], tlb_lld_1g[ENTRIES]);
}

int detect_ht_early(struct cpuinfo_x86 *c)
{
#ifdef CONFIG_SMP
	u32 eax, ebx, ecx, edx;

	if (!cpu_has(c, X86_FEATURE_HT))
		return -1;

	if (cpu_has(c, X86_FEATURE_CMP_LEGACY))
		return -1;

	if (cpu_has(c, X86_FEATURE_XTOPOLOGY))
		return -1;

	cpuid(1, &eax, &ebx, &ecx, &edx);

	smp_num_siblings = (ebx & 0xff0000) >> 16;
	if (smp_num_siblings == 1)
		pr_info_once("CPU0: Hyper-Threading is disabled\n");
#endif
	return 0;
}

void detect_ht(struct cpuinfo_x86 *c)
{
#ifdef CONFIG_SMP
	int index_msb, core_bits;

	if (detect_ht_early(c) < 0)
		return;

	index_msb = get_count_order(smp_num_siblings);
	c->phys_proc_id = apic->phys_pkg_id(c->initial_apicid, index_msb);


	c->cpu_core_id = apic->phys_pkg_id(c->initial_apicid, index_msb) &
				       ((1 << core_bits) - 1);
#endif
}

static void get_cpu_vendor(struct cpuinfo_x86 *c)
	{}
};

static const __initconst struct x86_cpu_id cpu_no_l1tf[] = {
	/* in addition to cpu_no_speculation */
	{ X86_VENDOR_INTEL,	6,	INTEL_FAM6_ATOM_SILVERMONT1	},
	{ X86_VENDOR_INTEL,	6,	INTEL_FAM6_ATOM_SILVERMONT2	},
	{ X86_VENDOR_INTEL,	6,	INTEL_FAM6_ATOM_AIRMONT		},
	{ X86_VENDOR_INTEL,	6,	INTEL_FAM6_ATOM_MERRIFIELD	},
	{ X86_VENDOR_INTEL,	6,	INTEL_FAM6_ATOM_MOOREFIELD	},
	{ X86_VENDOR_INTEL,	6,	INTEL_FAM6_ATOM_GOLDMONT	},
	{ X86_VENDOR_INTEL,	6,	INTEL_FAM6_ATOM_DENVERTON	},
	{ X86_VENDOR_INTEL,	6,	INTEL_FAM6_ATOM_GEMINI_LAKE	},
	{ X86_VENDOR_INTEL,	6,	INTEL_FAM6_XEON_PHI_KNL		},
	{ X86_VENDOR_INTEL,	6,	INTEL_FAM6_XEON_PHI_KNM		},
	{}
};

static void __init cpu_set_bug_bits(struct cpuinfo_x86 *c)
{
	u64 ia32_cap = 0;

		return;

	setup_force_cpu_bug(X86_BUG_CPU_MELTDOWN);

	if (x86_match_cpu(cpu_no_l1tf))
		return;

	setup_force_cpu_bug(X86_BUG_L1TF);
}

/*
 * The NOPL instruction is supposed to exist on all CPUs of family >= 6;
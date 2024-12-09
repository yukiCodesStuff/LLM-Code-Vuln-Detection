#endif
}

static const __initconst struct x86_cpu_id cpu_no_speculation[] = {
	{ X86_VENDOR_INTEL,	6, INTEL_FAM6_ATOM_SALTWELL,	X86_FEATURE_ANY },
	{ X86_VENDOR_INTEL,	6, INTEL_FAM6_ATOM_SALTWELL_TABLET,	X86_FEATURE_ANY },
	{ X86_VENDOR_INTEL,	6, INTEL_FAM6_ATOM_BONNELL_MID,	X86_FEATURE_ANY },
	{ X86_VENDOR_INTEL,	6, INTEL_FAM6_ATOM_SALTWELL_MID,	X86_FEATURE_ANY },
	{ X86_VENDOR_INTEL,	6, INTEL_FAM6_ATOM_BONNELL,	X86_FEATURE_ANY },
	{ X86_VENDOR_CENTAUR,	5 },
	{ X86_VENDOR_INTEL,	5 },
	{ X86_VENDOR_NSC,	5 },
	{ X86_VENDOR_ANY,	4 },
	{}
};

static const __initconst struct x86_cpu_id cpu_no_meltdown[] = {
	{ X86_VENDOR_AMD },
	{ X86_VENDOR_HYGON },
	{}
};

/* Only list CPUs which speculate but are non susceptible to SSB */
static const __initconst struct x86_cpu_id cpu_no_spec_store_bypass[] = {
	{ X86_VENDOR_INTEL,	6,	INTEL_FAM6_ATOM_SILVERMONT	},
	{ X86_VENDOR_INTEL,	6,	INTEL_FAM6_ATOM_AIRMONT		},
	{ X86_VENDOR_INTEL,	6,	INTEL_FAM6_ATOM_SILVERMONT_X	},
	{ X86_VENDOR_INTEL,	6,	INTEL_FAM6_ATOM_SILVERMONT_MID	},
	{ X86_VENDOR_INTEL,	6,	INTEL_FAM6_CORE_YONAH		},
	{ X86_VENDOR_INTEL,	6,	INTEL_FAM6_XEON_PHI_KNL		},
	{ X86_VENDOR_INTEL,	6,	INTEL_FAM6_XEON_PHI_KNM		},
	{ X86_VENDOR_AMD,	0x12,					},
	{ X86_VENDOR_AMD,	0x11,					},
	{ X86_VENDOR_AMD,	0x10,					},
	{ X86_VENDOR_AMD,	0xf,					},
	{}
};

static const __initconst struct x86_cpu_id cpu_no_l1tf[] = {
	/* in addition to cpu_no_speculation */
	{ X86_VENDOR_INTEL,	6,	INTEL_FAM6_ATOM_SILVERMONT	},
	{ X86_VENDOR_INTEL,	6,	INTEL_FAM6_ATOM_SILVERMONT_X	},
	{ X86_VENDOR_INTEL,	6,	INTEL_FAM6_ATOM_AIRMONT		},
	{ X86_VENDOR_INTEL,	6,	INTEL_FAM6_ATOM_SILVERMONT_MID	},
	{ X86_VENDOR_INTEL,	6,	INTEL_FAM6_ATOM_AIRMONT_MID	},
	{ X86_VENDOR_INTEL,	6,	INTEL_FAM6_ATOM_GOLDMONT	},
	{ X86_VENDOR_INTEL,	6,	INTEL_FAM6_ATOM_GOLDMONT_X	},
	{ X86_VENDOR_INTEL,	6,	INTEL_FAM6_ATOM_GOLDMONT_PLUS	},
	{ X86_VENDOR_INTEL,	6,	INTEL_FAM6_XEON_PHI_KNL		},
	{ X86_VENDOR_INTEL,	6,	INTEL_FAM6_XEON_PHI_KNM		},
	{}
};

static void __init cpu_set_bug_bits(struct cpuinfo_x86 *c)
{
	u64 ia32_cap = 0;

	if (x86_match_cpu(cpu_no_speculation))
		return;

	setup_force_cpu_bug(X86_BUG_SPECTRE_V1);
	setup_force_cpu_bug(X86_BUG_SPECTRE_V2);
	if (cpu_has(c, X86_FEATURE_ARCH_CAPABILITIES))
		rdmsrl(MSR_IA32_ARCH_CAPABILITIES, ia32_cap);

	if (!x86_match_cpu(cpu_no_spec_store_bypass) &&
	   !(ia32_cap & ARCH_CAP_SSB_NO) &&
	   !cpu_has(c, X86_FEATURE_AMD_SSB_NO))
		setup_force_cpu_bug(X86_BUG_SPEC_STORE_BYPASS);

	if (ia32_cap & ARCH_CAP_IBRS_ALL)
		setup_force_cpu_cap(X86_FEATURE_IBRS_ENHANCED);

	if (x86_match_cpu(cpu_no_meltdown))
		return;

	/* Rogue Data Cache Load? No! */
	if (ia32_cap & ARCH_CAP_RDCL_NO)

	setup_force_cpu_bug(X86_BUG_CPU_MELTDOWN);

	if (x86_match_cpu(cpu_no_l1tf))
		return;

	setup_force_cpu_bug(X86_BUG_L1TF);
}
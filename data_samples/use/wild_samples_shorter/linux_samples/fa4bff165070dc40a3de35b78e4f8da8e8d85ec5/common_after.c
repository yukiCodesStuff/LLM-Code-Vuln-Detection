#endif
}

#define NO_SPECULATION	BIT(0)
#define NO_MELTDOWN	BIT(1)
#define NO_SSB		BIT(2)
#define NO_L1TF		BIT(3)
#define NO_MDS		BIT(4)
#define MSBDS_ONLY	BIT(5)

#define VULNWL(_vendor, _family, _model, _whitelist)	\
	{ X86_VENDOR_##_vendor, _family, _model, X86_FEATURE_ANY, _whitelist }

#define VULNWL_INTEL(model, whitelist)		\
	VULNWL(INTEL, 6, INTEL_FAM6_##model, whitelist)

#define VULNWL_AMD(family, whitelist)		\
	VULNWL(AMD, family, X86_MODEL_ANY, whitelist)

#define VULNWL_HYGON(family, whitelist)		\
	VULNWL(HYGON, family, X86_MODEL_ANY, whitelist)

static const __initconst struct x86_cpu_id cpu_vuln_whitelist[] = {
	VULNWL(ANY,	4, X86_MODEL_ANY,	NO_SPECULATION),
	VULNWL(CENTAUR,	5, X86_MODEL_ANY,	NO_SPECULATION),
	VULNWL(INTEL,	5, X86_MODEL_ANY,	NO_SPECULATION),
	VULNWL(NSC,	5, X86_MODEL_ANY,	NO_SPECULATION),

	/* Intel Family 6 */
	VULNWL_INTEL(ATOM_SALTWELL,		NO_SPECULATION),
	VULNWL_INTEL(ATOM_SALTWELL_TABLET,	NO_SPECULATION),
	VULNWL_INTEL(ATOM_SALTWELL_MID,		NO_SPECULATION),
	VULNWL_INTEL(ATOM_BONNELL,		NO_SPECULATION),
	VULNWL_INTEL(ATOM_BONNELL_MID,		NO_SPECULATION),

	VULNWL_INTEL(ATOM_SILVERMONT,		NO_SSB | NO_L1TF | MSBDS_ONLY),
	VULNWL_INTEL(ATOM_SILVERMONT_X,		NO_SSB | NO_L1TF | MSBDS_ONLY),
	VULNWL_INTEL(ATOM_SILVERMONT_MID,	NO_SSB | NO_L1TF | MSBDS_ONLY),
	VULNWL_INTEL(ATOM_AIRMONT,		NO_SSB | NO_L1TF | MSBDS_ONLY),
	VULNWL_INTEL(XEON_PHI_KNL,		NO_SSB | NO_L1TF | MSBDS_ONLY),
	VULNWL_INTEL(XEON_PHI_KNM,		NO_SSB | NO_L1TF | MSBDS_ONLY),

	VULNWL_INTEL(CORE_YONAH,		NO_SSB),

	VULNWL_INTEL(ATOM_AIRMONT_MID,		NO_L1TF | MSBDS_ONLY),

	VULNWL_INTEL(ATOM_GOLDMONT,		NO_MDS | NO_L1TF),
	VULNWL_INTEL(ATOM_GOLDMONT_X,		NO_MDS | NO_L1TF),
	VULNWL_INTEL(ATOM_GOLDMONT_PLUS,	NO_MDS | NO_L1TF),

	/* AMD Family 0xf - 0x12 */
	VULNWL_AMD(0x0f,	NO_MELTDOWN | NO_SSB | NO_L1TF | NO_MDS),
	VULNWL_AMD(0x10,	NO_MELTDOWN | NO_SSB | NO_L1TF | NO_MDS),
	VULNWL_AMD(0x11,	NO_MELTDOWN | NO_SSB | NO_L1TF | NO_MDS),
	VULNWL_AMD(0x12,	NO_MELTDOWN | NO_SSB | NO_L1TF | NO_MDS),

	/* FAMILY_ANY must be last, otherwise 0x0f - 0x12 matches won't work */
	VULNWL_AMD(X86_FAMILY_ANY,	NO_MELTDOWN | NO_L1TF | NO_MDS),
	VULNWL_HYGON(X86_FAMILY_ANY,	NO_MELTDOWN | NO_L1TF | NO_MDS),
	{}
};

static bool __init cpu_matches(unsigned long which)
{
	const struct x86_cpu_id *m = x86_match_cpu(cpu_vuln_whitelist);

	return m && !!(m->driver_data & which);
}

static void __init cpu_set_bug_bits(struct cpuinfo_x86 *c)
{
	u64 ia32_cap = 0;

	if (cpu_matches(NO_SPECULATION))
		return;

	setup_force_cpu_bug(X86_BUG_SPECTRE_V1);
	setup_force_cpu_bug(X86_BUG_SPECTRE_V2);
	if (cpu_has(c, X86_FEATURE_ARCH_CAPABILITIES))
		rdmsrl(MSR_IA32_ARCH_CAPABILITIES, ia32_cap);

	if (!cpu_matches(NO_SSB) && !(ia32_cap & ARCH_CAP_SSB_NO) &&
	   !cpu_has(c, X86_FEATURE_AMD_SSB_NO))
		setup_force_cpu_bug(X86_BUG_SPEC_STORE_BYPASS);

	if (ia32_cap & ARCH_CAP_IBRS_ALL)
		setup_force_cpu_cap(X86_FEATURE_IBRS_ENHANCED);

	if (!cpu_matches(NO_MDS) && !(ia32_cap & ARCH_CAP_MDS_NO)) {
		setup_force_cpu_bug(X86_BUG_MDS);
		if (cpu_matches(MSBDS_ONLY))
			setup_force_cpu_bug(X86_BUG_MSBDS_ONLY);
	}

	if (cpu_matches(NO_MELTDOWN))
		return;

	/* Rogue Data Cache Load? No! */
	if (ia32_cap & ARCH_CAP_RDCL_NO)

	setup_force_cpu_bug(X86_BUG_CPU_MELTDOWN);

	if (cpu_matches(NO_L1TF))
		return;

	setup_force_cpu_bug(X86_BUG_L1TF);
}
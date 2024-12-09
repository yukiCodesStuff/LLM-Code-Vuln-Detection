			if (c->x86_vendor_id[0]) {
				get_cpu_vendor(c);
				break;
			}
		}
#endif
}

#define NO_SPECULATION	BIT(0)
#define NO_MELTDOWN	BIT(1)
#define NO_SSB		BIT(2)
#define NO_L1TF		BIT(3)
#define NO_MDS		BIT(4)

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

	VULNWL_INTEL(ATOM_SILVERMONT,		NO_SSB | NO_L1TF),
	VULNWL_INTEL(ATOM_SILVERMONT_X,		NO_SSB | NO_L1TF),
	VULNWL_INTEL(ATOM_SILVERMONT_MID,	NO_SSB | NO_L1TF),
	VULNWL_INTEL(ATOM_AIRMONT,		NO_SSB | NO_L1TF),
	VULNWL_INTEL(XEON_PHI_KNL,		NO_SSB | NO_L1TF),
	VULNWL_INTEL(XEON_PHI_KNM,		NO_SSB | NO_L1TF),

	VULNWL_INTEL(CORE_YONAH,		NO_SSB),

	VULNWL_INTEL(ATOM_AIRMONT_MID,		NO_L1TF),

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

	if (!cpu_matches(NO_MDS) && !(ia32_cap & ARCH_CAP_MDS_NO))
		setup_force_cpu_bug(X86_BUG_MDS);

	if (cpu_matches(NO_MELTDOWN))
		return;

	/* Rogue Data Cache Load? No! */
	if (ia32_cap & ARCH_CAP_RDCL_NO)
		return;

	setup_force_cpu_bug(X86_BUG_CPU_MELTDOWN);

	if (cpu_matches(NO_L1TF))
		return;

	setup_force_cpu_bug(X86_BUG_L1TF);
}

/*
 * The NOPL instruction is supposed to exist on all CPUs of family >= 6;
 * unfortunately, that's not true in practice because of early VIA
 * chips and (more importantly) broken virtualizers that are not easy
 * to detect. In the latter case it doesn't even *fail* reliably, so
 * probing for it doesn't even work. Disable it completely on 32-bit
 * unless we can find a reliable way to detect all the broken cases.
 * Enable it explicitly on 64-bit for non-constant inputs of cpu_has().
 */
static void detect_nopl(void)
{
#ifdef CONFIG_X86_32
	setup_clear_cpu_cap(X86_FEATURE_NOPL);
#else
	setup_force_cpu_cap(X86_FEATURE_NOPL);
#endif
}

/*
 * Do minimum CPU detection early.
 * Fields really needed: vendor, cpuid_level, family, model, mask,
 * cache alignment.
 * The others are not touched to avoid unwanted side effects.
 *
 * WARNING: this function is only called on the boot CPU.  Don't add code
 * here that is supposed to run on all CPUs.
 */
static void __init early_identify_cpu(struct cpuinfo_x86 *c)
{
#ifdef CONFIG_X86_64
	c->x86_clflush_size = 64;
	c->x86_phys_bits = 36;
	c->x86_virt_bits = 48;
#else
	c->x86_clflush_size = 32;
	c->x86_phys_bits = 32;
	c->x86_virt_bits = 32;
#endif
	c->x86_cache_alignment = c->x86_clflush_size;

	memset(&c->x86_capability, 0, sizeof(c->x86_capability));
	c->extended_cpuid_level = 0;

	if (!have_cpuid_p())
		identify_cpu_without_cpuid(c);

	/* cyrix could have cpuid enabled via c_identify()*/
	if (have_cpuid_p()) {
		cpu_detect(c);
		get_cpu_vendor(c);
		get_cpu_cap(c);
		get_cpu_address_sizes(c);
		setup_force_cpu_cap(X86_FEATURE_CPUID);

		if (this_cpu->c_early_init)
			this_cpu->c_early_init(c);

		c->cpu_index = 0;
		filter_cpuid_features(c, false);

		if (this_cpu->c_bsp_init)
			this_cpu->c_bsp_init(c);
	} else {
		setup_clear_cpu_cap(X86_FEATURE_CPUID);
	}

	setup_force_cpu_cap(X86_FEATURE_ALWAYS);

	cpu_set_bug_bits(c);

	fpu__init_system(c);

#ifdef CONFIG_X86_32
	/*
	 * Regardless of whether PCID is enumerated, the SDM says
	 * that it can't be enabled in 32-bit mode.
	 */
	setup_clear_cpu_cap(X86_FEATURE_PCID);
#endif

	/*
	 * Later in the boot process pgtable_l5_enabled() relies on
	 * cpu_feature_enabled(X86_FEATURE_LA57). If 5-level paging is not
	 * enabled by this point we need to clear the feature bit to avoid
	 * false-positives at the later stage.
	 *
	 * pgtable_l5_enabled() can be false here for several reasons:
	 *  - 5-level paging is disabled compile-time;
	 *  - it's 32-bit kernel;
	 *  - machine doesn't support 5-level paging;
	 *  - user specified 'no5lvl' in kernel command line.
	 */
	if (!pgtable_l5_enabled())
		setup_clear_cpu_cap(X86_FEATURE_LA57);

	detect_nopl();
}
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

	VULNWL_INTEL(ATOM_SILVERMONT,		NO_SSB | NO_L1TF),
	VULNWL_INTEL(ATOM_SILVERMONT_X,		NO_SSB | NO_L1TF),
	VULNWL_INTEL(ATOM_SILVERMONT_MID,	NO_SSB | NO_L1TF),
	VULNWL_INTEL(ATOM_AIRMONT,		NO_SSB | NO_L1TF),
	VULNWL_INTEL(XEON_PHI_KNL,		NO_SSB | NO_L1TF),
	VULNWL_INTEL(XEON_PHI_KNM,		NO_SSB | NO_L1TF),

	VULNWL_INTEL(CORE_YONAH,		NO_SSB),

	VULNWL_INTEL(ATOM_AIRMONT_MID,		NO_L1TF),

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

	if (!cpu_matches(NO_MDS) && !(ia32_cap & ARCH_CAP_MDS_NO))
		setup_force_cpu_bug(X86_BUG_MDS);

	if (cpu_matches(NO_MELTDOWN))
		return;

	/* Rogue Data Cache Load? No! */
	if (ia32_cap & ARCH_CAP_RDCL_NO)
		return;

	setup_force_cpu_bug(X86_BUG_CPU_MELTDOWN);

	if (cpu_matches(NO_L1TF))
		return;

	setup_force_cpu_bug(X86_BUG_L1TF);
}

/*
 * The NOPL instruction is supposed to exist on all CPUs of family >= 6;
 * unfortunately, that's not true in practice because of early VIA
 * chips and (more importantly) broken virtualizers that are not easy
 * to detect. In the latter case it doesn't even *fail* reliably, so
 * probing for it doesn't even work. Disable it completely on 32-bit
 * unless we can find a reliable way to detect all the broken cases.
 * Enable it explicitly on 64-bit for non-constant inputs of cpu_has().
 */
static void detect_nopl(void)
{
#ifdef CONFIG_X86_32
	setup_clear_cpu_cap(X86_FEATURE_NOPL);
#else
	setup_force_cpu_cap(X86_FEATURE_NOPL);
#endif
}

/*
 * Do minimum CPU detection early.
 * Fields really needed: vendor, cpuid_level, family, model, mask,
 * cache alignment.
 * The others are not touched to avoid unwanted side effects.
 *
 * WARNING: this function is only called on the boot CPU.  Don't add code
 * here that is supposed to run on all CPUs.
 */
static void __init early_identify_cpu(struct cpuinfo_x86 *c)
{
#ifdef CONFIG_X86_64
	c->x86_clflush_size = 64;
	c->x86_phys_bits = 36;
	c->x86_virt_bits = 48;
#else
	c->x86_clflush_size = 32;
	c->x86_phys_bits = 32;
	c->x86_virt_bits = 32;
#endif
	c->x86_cache_alignment = c->x86_clflush_size;

	memset(&c->x86_capability, 0, sizeof(c->x86_capability));
	c->extended_cpuid_level = 0;

	if (!have_cpuid_p())
		identify_cpu_without_cpuid(c);

	/* cyrix could have cpuid enabled via c_identify()*/
	if (have_cpuid_p()) {
		cpu_detect(c);
		get_cpu_vendor(c);
		get_cpu_cap(c);
		get_cpu_address_sizes(c);
		setup_force_cpu_cap(X86_FEATURE_CPUID);

		if (this_cpu->c_early_init)
			this_cpu->c_early_init(c);

		c->cpu_index = 0;
		filter_cpuid_features(c, false);

		if (this_cpu->c_bsp_init)
			this_cpu->c_bsp_init(c);
	} else {
		setup_clear_cpu_cap(X86_FEATURE_CPUID);
	}

	setup_force_cpu_cap(X86_FEATURE_ALWAYS);

	cpu_set_bug_bits(c);

	fpu__init_system(c);

#ifdef CONFIG_X86_32
	/*
	 * Regardless of whether PCID is enumerated, the SDM says
	 * that it can't be enabled in 32-bit mode.
	 */
	setup_clear_cpu_cap(X86_FEATURE_PCID);
#endif

	/*
	 * Later in the boot process pgtable_l5_enabled() relies on
	 * cpu_feature_enabled(X86_FEATURE_LA57). If 5-level paging is not
	 * enabled by this point we need to clear the feature bit to avoid
	 * false-positives at the later stage.
	 *
	 * pgtable_l5_enabled() can be false here for several reasons:
	 *  - 5-level paging is disabled compile-time;
	 *  - it's 32-bit kernel;
	 *  - machine doesn't support 5-level paging;
	 *  - user specified 'no5lvl' in kernel command line.
	 */
	if (!pgtable_l5_enabled())
		setup_clear_cpu_cap(X86_FEATURE_LA57);

	detect_nopl();
}
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

	VULNWL_INTEL(ATOM_SILVERMONT,		NO_SSB | NO_L1TF),
	VULNWL_INTEL(ATOM_SILVERMONT_X,		NO_SSB | NO_L1TF),
	VULNWL_INTEL(ATOM_SILVERMONT_MID,	NO_SSB | NO_L1TF),
	VULNWL_INTEL(ATOM_AIRMONT,		NO_SSB | NO_L1TF),
	VULNWL_INTEL(XEON_PHI_KNL,		NO_SSB | NO_L1TF),
	VULNWL_INTEL(XEON_PHI_KNM,		NO_SSB | NO_L1TF),

	VULNWL_INTEL(CORE_YONAH,		NO_SSB),

	VULNWL_INTEL(ATOM_AIRMONT_MID,		NO_L1TF),

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

	if (!cpu_matches(NO_MDS) && !(ia32_cap & ARCH_CAP_MDS_NO))
		setup_force_cpu_bug(X86_BUG_MDS);

	if (cpu_matches(NO_MELTDOWN))
		return;

	/* Rogue Data Cache Load? No! */
	if (ia32_cap & ARCH_CAP_RDCL_NO)
		return;

	setup_force_cpu_bug(X86_BUG_CPU_MELTDOWN);

	if (cpu_matches(NO_L1TF))
		return;

	setup_force_cpu_bug(X86_BUG_L1TF);
}

/*
 * The NOPL instruction is supposed to exist on all CPUs of family >= 6;
 * unfortunately, that's not true in practice because of early VIA
 * chips and (more importantly) broken virtualizers that are not easy
 * to detect. In the latter case it doesn't even *fail* reliably, so
 * probing for it doesn't even work. Disable it completely on 32-bit
 * unless we can find a reliable way to detect all the broken cases.
 * Enable it explicitly on 64-bit for non-constant inputs of cpu_has().
 */
static void detect_nopl(void)
{
#ifdef CONFIG_X86_32
	setup_clear_cpu_cap(X86_FEATURE_NOPL);
#else
	setup_force_cpu_cap(X86_FEATURE_NOPL);
#endif
}

/*
 * Do minimum CPU detection early.
 * Fields really needed: vendor, cpuid_level, family, model, mask,
 * cache alignment.
 * The others are not touched to avoid unwanted side effects.
 *
 * WARNING: this function is only called on the boot CPU.  Don't add code
 * here that is supposed to run on all CPUs.
 */
static void __init early_identify_cpu(struct cpuinfo_x86 *c)
{
#ifdef CONFIG_X86_64
	c->x86_clflush_size = 64;
	c->x86_phys_bits = 36;
	c->x86_virt_bits = 48;
#else
	c->x86_clflush_size = 32;
	c->x86_phys_bits = 32;
	c->x86_virt_bits = 32;
#endif
	c->x86_cache_alignment = c->x86_clflush_size;

	memset(&c->x86_capability, 0, sizeof(c->x86_capability));
	c->extended_cpuid_level = 0;

	if (!have_cpuid_p())
		identify_cpu_without_cpuid(c);

	/* cyrix could have cpuid enabled via c_identify()*/
	if (have_cpuid_p()) {
		cpu_detect(c);
		get_cpu_vendor(c);
		get_cpu_cap(c);
		get_cpu_address_sizes(c);
		setup_force_cpu_cap(X86_FEATURE_CPUID);

		if (this_cpu->c_early_init)
			this_cpu->c_early_init(c);

		c->cpu_index = 0;
		filter_cpuid_features(c, false);

		if (this_cpu->c_bsp_init)
			this_cpu->c_bsp_init(c);
	} else {
		setup_clear_cpu_cap(X86_FEATURE_CPUID);
	}

	setup_force_cpu_cap(X86_FEATURE_ALWAYS);

	cpu_set_bug_bits(c);

	fpu__init_system(c);

#ifdef CONFIG_X86_32
	/*
	 * Regardless of whether PCID is enumerated, the SDM says
	 * that it can't be enabled in 32-bit mode.
	 */
	setup_clear_cpu_cap(X86_FEATURE_PCID);
#endif

	/*
	 * Later in the boot process pgtable_l5_enabled() relies on
	 * cpu_feature_enabled(X86_FEATURE_LA57). If 5-level paging is not
	 * enabled by this point we need to clear the feature bit to avoid
	 * false-positives at the later stage.
	 *
	 * pgtable_l5_enabled() can be false here for several reasons:
	 *  - 5-level paging is disabled compile-time;
	 *  - it's 32-bit kernel;
	 *  - machine doesn't support 5-level paging;
	 *  - user specified 'no5lvl' in kernel command line.
	 */
	if (!pgtable_l5_enabled())
		setup_clear_cpu_cap(X86_FEATURE_LA57);

	detect_nopl();
}
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

	if (!cpu_matches(NO_MDS) && !(ia32_cap & ARCH_CAP_MDS_NO))
		setup_force_cpu_bug(X86_BUG_MDS);

	if (cpu_matches(NO_MELTDOWN))
		return;

	/* Rogue Data Cache Load? No! */
	if (ia32_cap & ARCH_CAP_RDCL_NO)
		return;

	setup_force_cpu_bug(X86_BUG_CPU_MELTDOWN);

	if (cpu_matches(NO_L1TF))
		return;

	setup_force_cpu_bug(X86_BUG_L1TF);
}

/*
 * The NOPL instruction is supposed to exist on all CPUs of family >= 6;
 * unfortunately, that's not true in practice because of early VIA
 * chips and (more importantly) broken virtualizers that are not easy
 * to detect. In the latter case it doesn't even *fail* reliably, so
 * probing for it doesn't even work. Disable it completely on 32-bit
 * unless we can find a reliable way to detect all the broken cases.
 * Enable it explicitly on 64-bit for non-constant inputs of cpu_has().
 */
static void detect_nopl(void)
{
#ifdef CONFIG_X86_32
	setup_clear_cpu_cap(X86_FEATURE_NOPL);
#else
	setup_force_cpu_cap(X86_FEATURE_NOPL);
#endif
}

/*
 * Do minimum CPU detection early.
 * Fields really needed: vendor, cpuid_level, family, model, mask,
 * cache alignment.
 * The others are not touched to avoid unwanted side effects.
 *
 * WARNING: this function is only called on the boot CPU.  Don't add code
 * here that is supposed to run on all CPUs.
 */
static void __init early_identify_cpu(struct cpuinfo_x86 *c)
{
#ifdef CONFIG_X86_64
	c->x86_clflush_size = 64;
	c->x86_phys_bits = 36;
	c->x86_virt_bits = 48;
#else
	c->x86_clflush_size = 32;
	c->x86_phys_bits = 32;
	c->x86_virt_bits = 32;
#endif
	c->x86_cache_alignment = c->x86_clflush_size;

	memset(&c->x86_capability, 0, sizeof(c->x86_capability));
	c->extended_cpuid_level = 0;

	if (!have_cpuid_p())
		identify_cpu_without_cpuid(c);

	/* cyrix could have cpuid enabled via c_identify()*/
	if (have_cpuid_p()) {
		cpu_detect(c);
		get_cpu_vendor(c);
		get_cpu_cap(c);
		get_cpu_address_sizes(c);
		setup_force_cpu_cap(X86_FEATURE_CPUID);

		if (this_cpu->c_early_init)
			this_cpu->c_early_init(c);

		c->cpu_index = 0;
		filter_cpuid_features(c, false);

		if (this_cpu->c_bsp_init)
			this_cpu->c_bsp_init(c);
	} else {
		setup_clear_cpu_cap(X86_FEATURE_CPUID);
	}

	setup_force_cpu_cap(X86_FEATURE_ALWAYS);

	cpu_set_bug_bits(c);

	fpu__init_system(c);

#ifdef CONFIG_X86_32
	/*
	 * Regardless of whether PCID is enumerated, the SDM says
	 * that it can't be enabled in 32-bit mode.
	 */
	setup_clear_cpu_cap(X86_FEATURE_PCID);
#endif

	/*
	 * Later in the boot process pgtable_l5_enabled() relies on
	 * cpu_feature_enabled(X86_FEATURE_LA57). If 5-level paging is not
	 * enabled by this point we need to clear the feature bit to avoid
	 * false-positives at the later stage.
	 *
	 * pgtable_l5_enabled() can be false here for several reasons:
	 *  - 5-level paging is disabled compile-time;
	 *  - it's 32-bit kernel;
	 *  - machine doesn't support 5-level paging;
	 *  - user specified 'no5lvl' in kernel command line.
	 */
	if (!pgtable_l5_enabled())
		setup_clear_cpu_cap(X86_FEATURE_LA57);

	detect_nopl();
}
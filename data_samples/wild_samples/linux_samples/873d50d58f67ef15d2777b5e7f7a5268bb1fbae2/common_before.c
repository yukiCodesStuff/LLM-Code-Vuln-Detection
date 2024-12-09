{
	/* Check the boot processor, plus build option for UMIP. */
	if (!cpu_feature_enabled(X86_FEATURE_UMIP))
		goto out;

	/* Check the current processor's cpuid bits. */
	if (!cpu_has(c, X86_FEATURE_UMIP))
		goto out;

	cr4_set_bits(X86_CR4_UMIP);

	pr_info_once("x86/cpu: User Mode Instruction Prevention (UMIP) activated\n");

	return;

out:
	/*
	 * Make sure UMIP is disabled in case it was enabled in a
	 * previous boot (e.g., via kexec).
	 */
	cr4_clear_bits(X86_CR4_UMIP);
}

/*
 * Protection Keys are not available in 32-bit mode.
 */
static bool pku_disabled;

static __always_inline void setup_pku(struct cpuinfo_x86 *c)
{
	struct pkru_state *pk;

	/* check the boot processor, plus compile options for PKU: */
	if (!cpu_feature_enabled(X86_FEATURE_PKU))
		return;
	/* checks the actual processor's cpuid bits: */
	if (!cpu_has(c, X86_FEATURE_PKU))
		return;
	if (pku_disabled)
		return;

	cr4_set_bits(X86_CR4_PKE);
	pk = get_xsave_addr(&init_fpstate.xsave, XFEATURE_PKRU);
	if (pk)
		pk->pkru = init_pkru_value;
	/*
	 * Seting X86_CR4_PKE will cause the X86_FEATURE_OSPKE
	 * cpuid bit to be set.  We need to ensure that we
	 * update that bit in this CPU's "cpu_info".
	 */
	get_cpu_cap(c);
}

#ifdef CONFIG_X86_INTEL_MEMORY_PROTECTION_KEYS
static __init int setup_disable_pku(char *arg)
{
	/*
	 * Do not clear the X86_FEATURE_PKU bit.  All of the
	 * runtime checks are against OSPKE so clearing the
	 * bit does nothing.
	 *
	 * This way, we will see "pku" in cpuinfo, but not
	 * "ospke", which is exactly what we want.  It shows
	 * that the CPU has PKU, but the OS has not enabled it.
	 * This happens to be exactly how a system would look
	 * if we disabled the config option.
	 */
	pr_info("x86: 'nopku' specified, disabling Memory Protection Keys\n");
	pku_disabled = true;
	return 1;
}
__setup("nopku", setup_disable_pku);
#endif /* CONFIG_X86_64 */

/*
 * Some CPU features depend on higher CPUID levels, which may not always
 * be available due to CPUID level capping or broken virtualization
 * software.  Add those features to this table to auto-disable them.
 */
struct cpuid_dependent_feature {
	u32 feature;
	u32 level;
};

static const struct cpuid_dependent_feature
cpuid_dependent_features[] = {
	{ X86_FEATURE_MWAIT,		0x00000005 },
	{ X86_FEATURE_DCA,		0x00000009 },
	{ X86_FEATURE_XSAVE,		0x0000000d },
	{ 0, 0 }
{
	identify_cpu(&boot_cpu_data);
#ifdef CONFIG_X86_32
	sysenter_setup();
	enable_sep_cpu();
#endif
	cpu_detect_tlb(&boot_cpu_data);
}

void identify_secondary_cpu(struct cpuinfo_x86 *c)
{
	BUG_ON(c == &boot_cpu_data);
	identify_cpu(c);
#ifdef CONFIG_X86_32
	enable_sep_cpu();
#endif
	mtrr_ap_init();
	validate_apic_and_package_id(c);
	x86_spec_ctrl_setup_ap();
}

static __init int setup_noclflush(char *arg)
{
	setup_clear_cpu_cap(X86_FEATURE_CLFLUSH);
	setup_clear_cpu_cap(X86_FEATURE_CLFLUSHOPT);
	return 1;
}
__setup("noclflush", setup_noclflush);

void print_cpu_info(struct cpuinfo_x86 *c)
{
	const char *vendor = NULL;

	if (c->x86_vendor < X86_VENDOR_NUM) {
		vendor = this_cpu->c_vendor;
	} else {
		if (c->cpuid_level >= 0)
			vendor = c->x86_vendor_id;
	}

	if (vendor && !strstr(c->x86_model_id, vendor))
		pr_cont("%s ", vendor);

	if (c->x86_model_id[0])
		pr_cont("%s", c->x86_model_id);
	else
		pr_cont("%d86", c->x86);

	pr_cont(" (family: 0x%x, model: 0x%x", c->x86, c->x86_model);

	if (c->x86_stepping || c->cpuid_level >= 0)
		pr_cont(", stepping: 0x%x)\n", c->x86_stepping);
	else
		pr_cont(")\n");
}
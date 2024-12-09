	if (cpu_has_apic && c->x86 >= 0xf) {
		unsigned int val;
		val = read_pci_config(0, 24, 0, 0x68);
		if ((val & ((1 << 17) | (1 << 18))) == ((1 << 17) | (1 << 18)))
			set_cpu_cap(c, X86_FEATURE_EXTD_APICID);
	}
#endif

	/* F16h erratum 793, CVE-2013-6885 */
	if (c->x86 == 0x16 && c->x86_model <= 0xf) {
		u64 val;

		rdmsrl(MSR_AMD64_LS_CFG, val);
		if (!(val & BIT(15)))
			wrmsrl(MSR_AMD64_LS_CFG, val | BIT(15));
	}

}

static const int amd_erratum_383[];
static const int amd_erratum_400[];
static bool cpu_has_amd_erratum(struct cpuinfo_x86 *cpu, const int *erratum);

static void init_amd(struct cpuinfo_x86 *c)
{
	u32 dummy;
	unsigned long long value;

#ifdef CONFIG_SMP
	/*
	 * Disable TLB flush filter by setting HWCR.FFDIS on K8
	 * bit 6 of msr C001_0015
	 *
	 * Errata 63 for SH-B3 steppings
	 * Errata 122 for all steppings (F+ have it disabled by default)
	 */
	if (c->x86 == 0xf) {
		rdmsrl(MSR_K7_HWCR, value);
		value |= 1 << 6;
		wrmsrl(MSR_K7_HWCR, value);
	}
#endif

	early_init_amd(c);

	/*
	 * Bit 31 in normal CPUID used for nonstandard 3DNow ID;
	 * 3DNow is IDd by bit 31 in extended CPUID (1*32+31) anyway
	 */
	clear_cpu_cap(c, 0*32+31);

#ifdef CONFIG_X86_64
	/* On C+ stepping K8 rep microcode works well for copy/memset */
	if (c->x86 == 0xf) {
		u32 level;

		level = cpuid_eax(1);
		if ((level >= 0x0f48 && level < 0x0f50) || level >= 0x0f58)
			set_cpu_cap(c, X86_FEATURE_REP_GOOD);

		/*
		 * Some BIOSes incorrectly force this feature, but only K8
		 * revision D (model = 0x14) and later actually support it.
		 * (AMD Erratum #110, docId: 25759).
		 */
		if (c->x86_model < 0x14 && cpu_has(c, X86_FEATURE_LAHF_LM)) {
			clear_cpu_cap(c, X86_FEATURE_LAHF_LM);
			if (!rdmsrl_amd_safe(0xc001100d, &value)) {
				value &= ~(1ULL << 32);
				wrmsrl_amd_safe(0xc001100d, value);
			}
		}
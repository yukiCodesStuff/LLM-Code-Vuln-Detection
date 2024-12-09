	if (__kpti_forced) {
		pr_info_once("kernel page table isolation forced %s by command line option\n",
			     __kpti_forced > 0 ? "ON" : "OFF");
		return __kpti_forced > 0;
	}

	/* Useful for KASLR robustness */
	if (IS_ENABLED(CONFIG_RANDOMIZE_BASE))
		return true;

	/* Defer to CPU feature registers */
	return !cpuid_feature_extract_unsigned_field(pfr0,
						     ID_AA64PFR0_CSV3_SHIFT);
}

static int __init parse_kpti(char *str)
{
	bool enabled;
	int ret = strtobool(str, &enabled);

	if (ret)
		return ret;

	__kpti_forced = enabled ? 1 : -1;
	return 0;
}
__setup("kpti=", parse_kpti);
#endif	/* CONFIG_UNMAP_KERNEL_AT_EL0 */

static int cpu_copy_el2regs(void *__unused)
{
	/*
	 * Copy register values that aren't redirected by hardware.
	 *
	 * Before code patching, we only set tpidr_el1, all CPUs need to copy
	 * this value to tpidr_el2 before we patch the code. Once we've done
	 * that, freshly-onlined CPUs will set tpidr_el2, so we don't need to
	 * do anything here.
	 */
	if (!alternatives_applied)
		write_sysreg(read_sysreg(tpidr_el1), tpidr_el2);

	return 0;
}

static const struct arm64_cpu_capabilities arm64_features[] = {
	{
		.desc = "GIC system register CPU interface",
		.capability = ARM64_HAS_SYSREG_GIC_CPUIF,
		.def_scope = SCOPE_SYSTEM,
		.matches = has_useable_gicv3_cpuif,
		.sys_reg = SYS_ID_AA64PFR0_EL1,
		.field_pos = ID_AA64PFR0_GIC_SHIFT,
		.sign = FTR_UNSIGNED,
		.min_field_value = 1,
	},
#ifdef CONFIG_ARM64_PAN
	{
		.desc = "Privileged Access Never",
		.capability = ARM64_HAS_PAN,
		.def_scope = SCOPE_SYSTEM,
		.matches = has_cpuid_feature,
		.sys_reg = SYS_ID_AA64MMFR1_EL1,
		.field_pos = ID_AA64MMFR1_PAN_SHIFT,
		.sign = FTR_UNSIGNED,
		.min_field_value = 1,
		.enable = cpu_enable_pan,
	},
#endif /* CONFIG_ARM64_PAN */
#if defined(CONFIG_AS_LSE) && defined(CONFIG_ARM64_LSE_ATOMICS)
	{
		.desc = "LSE atomic instructions",
		.capability = ARM64_HAS_LSE_ATOMICS,
		.def_scope = SCOPE_SYSTEM,
		.matches = has_cpuid_feature,
		.sys_reg = SYS_ID_AA64ISAR0_EL1,
		.field_pos = ID_AA64ISAR0_ATOMICS_SHIFT,
		.sign = FTR_UNSIGNED,
		.min_field_value = 2,
	},
#endif /* CONFIG_AS_LSE && CONFIG_ARM64_LSE_ATOMICS */
	{
		.desc = "Software prefetching using PRFM",
		.capability = ARM64_HAS_NO_HW_PREFETCH,
		.def_scope = SCOPE_SYSTEM,
		.matches = has_no_hw_prefetch,
	},
#ifdef CONFIG_ARM64_UAO
	{
		.desc = "User Access Override",
		.capability = ARM64_HAS_UAO,
		.def_scope = SCOPE_SYSTEM,
		.matches = has_cpuid_feature,
		.sys_reg = SYS_ID_AA64MMFR2_EL1,
		.field_pos = ID_AA64MMFR2_UAO_SHIFT,
		.min_field_value = 1,
		/*
		 * We rely on stop_machine() calling uao_thread_switch() to set
		 * UAO immediately after patching.
		 */
	},
#endif /* CONFIG_ARM64_UAO */
#ifdef CONFIG_ARM64_PAN
	{
		.capability = ARM64_ALT_PAN_NOT_UAO,
		.def_scope = SCOPE_SYSTEM,
		.matches = cpufeature_pan_not_uao,
	},
#endif /* CONFIG_ARM64_PAN */
	{
		.desc = "Virtualization Host Extensions",
		.capability = ARM64_HAS_VIRT_HOST_EXTN,
		.def_scope = SCOPE_SYSTEM,
		.matches = runs_at_el2,
		.enable = cpu_copy_el2regs,
	},
	{
		.desc = "32-bit EL0 Support",
		.capability = ARM64_HAS_32BIT_EL0,
		.def_scope = SCOPE_SYSTEM,
		.matches = has_cpuid_feature,
		.sys_reg = SYS_ID_AA64PFR0_EL1,
		.sign = FTR_UNSIGNED,
		.field_pos = ID_AA64PFR0_EL0_SHIFT,
		.min_field_value = ID_AA64PFR0_EL0_32BIT_64BIT,
	},
	{
		.desc = "Reduced HYP mapping offset",
		.capability = ARM64_HYP_OFFSET_LOW,
		.def_scope = SCOPE_SYSTEM,
		.matches = hyp_offset_low,
	},
#ifdef CONFIG_UNMAP_KERNEL_AT_EL0
	{
		.desc = "Kernel page table isolation (KPTI)",
		.capability = ARM64_UNMAP_KERNEL_AT_EL0,
		.def_scope = SCOPE_SYSTEM,
		.matches = unmap_kernel_at_el0,
	},
#endif
	{
		/* FP/SIMD is not implemented */
		.capability = ARM64_HAS_NO_FPSIMD,
		.def_scope = SCOPE_SYSTEM,
		.min_field_value = 0,
		.matches = has_no_fpsimd,
	},
#ifdef CONFIG_ARM64_PMEM
	{
		.desc = "Data cache clean to Point of Persistence",
		.capability = ARM64_HAS_DCPOP,
		.def_scope = SCOPE_SYSTEM,
		.matches = has_cpuid_feature,
		.sys_reg = SYS_ID_AA64ISAR1_EL1,
		.field_pos = ID_AA64ISAR1_DPB_SHIFT,
		.min_field_value = 1,
	},
#endif
#ifdef CONFIG_ARM64_SVE
	{
		.desc = "Scalable Vector Extension",
		.capability = ARM64_SVE,
		.def_scope = SCOPE_SYSTEM,
		.sys_reg = SYS_ID_AA64PFR0_EL1,
		.sign = FTR_UNSIGNED,
		.field_pos = ID_AA64PFR0_SVE_SHIFT,
		.min_field_value = ID_AA64PFR0_SVE,
		.matches = has_cpuid_feature,
		.enable = sve_kernel_enable,
	},
#endif /* CONFIG_ARM64_SVE */
#ifdef CONFIG_ARM64_RAS_EXTN
	{
		.desc = "RAS Extension Support",
		.capability = ARM64_HAS_RAS_EXTN,
		.def_scope = SCOPE_SYSTEM,
		.matches = has_cpuid_feature,
		.sys_reg = SYS_ID_AA64PFR0_EL1,
		.sign = FTR_UNSIGNED,
		.field_pos = ID_AA64PFR0_RAS_SHIFT,
		.min_field_value = ID_AA64PFR0_RAS_V1,
		.enable = cpu_clear_disr,
	},
#endif /* CONFIG_ARM64_RAS_EXTN */
	{},
};

#define HWCAP_CAP(reg, field, s, min_value, type, cap)	\
	{							\
		.desc = #cap,					\
		.def_scope = SCOPE_SYSTEM,			\
		.matches = has_cpuid_feature,			\
		.sys_reg = reg,					\
		.field_pos = field,				\
		.sign = s,					\
		.min_field_value = min_value,			\
		.hwcap_type = type,				\
		.hwcap = cap,					\
	}

static const struct arm64_cpu_capabilities arm64_elf_hwcaps[] = {
	HWCAP_CAP(SYS_ID_AA64ISAR0_EL1, ID_AA64ISAR0_AES_SHIFT, FTR_UNSIGNED, 2, CAP_HWCAP, HWCAP_PMULL),
	HWCAP_CAP(SYS_ID_AA64ISAR0_EL1, ID_AA64ISAR0_AES_SHIFT, FTR_UNSIGNED, 1, CAP_HWCAP, HWCAP_AES),
	HWCAP_CAP(SYS_ID_AA64ISAR0_EL1, ID_AA64ISAR0_SHA1_SHIFT, FTR_UNSIGNED, 1, CAP_HWCAP, HWCAP_SHA1),
	HWCAP_CAP(SYS_ID_AA64ISAR0_EL1, ID_AA64ISAR0_SHA2_SHIFT, FTR_UNSIGNED, 1, CAP_HWCAP, HWCAP_SHA2),
	HWCAP_CAP(SYS_ID_AA64ISAR0_EL1, ID_AA64ISAR0_SHA2_SHIFT, FTR_UNSIGNED, 2, CAP_HWCAP, HWCAP_SHA512),
	HWCAP_CAP(SYS_ID_AA64ISAR0_EL1, ID_AA64ISAR0_CRC32_SHIFT, FTR_UNSIGNED, 1, CAP_HWCAP, HWCAP_CRC32),
	HWCAP_CAP(SYS_ID_AA64ISAR0_EL1, ID_AA64ISAR0_ATOMICS_SHIFT, FTR_UNSIGNED, 2, CAP_HWCAP, HWCAP_ATOMICS),
	HWCAP_CAP(SYS_ID_AA64ISAR0_EL1, ID_AA64ISAR0_RDM_SHIFT, FTR_UNSIGNED, 1, CAP_HWCAP, HWCAP_ASIMDRDM),
	HWCAP_CAP(SYS_ID_AA64ISAR0_EL1, ID_AA64ISAR0_SHA3_SHIFT, FTR_UNSIGNED, 1, CAP_HWCAP, HWCAP_SHA3),
	HWCAP_CAP(SYS_ID_AA64ISAR0_EL1, ID_AA64ISAR0_SM3_SHIFT, FTR_UNSIGNED, 1, CAP_HWCAP, HWCAP_SM3),
	HWCAP_CAP(SYS_ID_AA64ISAR0_EL1, ID_AA64ISAR0_SM4_SHIFT, FTR_UNSIGNED, 1, CAP_HWCAP, HWCAP_SM4),
	HWCAP_CAP(SYS_ID_AA64ISAR0_EL1, ID_AA64ISAR0_DP_SHIFT, FTR_UNSIGNED, 1, CAP_HWCAP, HWCAP_ASIMDDP),
	HWCAP_CAP(SYS_ID_AA64ISAR0_EL1, ID_AA64ISAR0_FHM_SHIFT, FTR_UNSIGNED, 1, CAP_HWCAP, HWCAP_ASIMDFHM),
	HWCAP_CAP(SYS_ID_AA64PFR0_EL1, ID_AA64PFR0_FP_SHIFT, FTR_SIGNED, 0, CAP_HWCAP, HWCAP_FP),
	HWCAP_CAP(SYS_ID_AA64PFR0_EL1, ID_AA64PFR0_FP_SHIFT, FTR_SIGNED, 1, CAP_HWCAP, HWCAP_FPHP),
	HWCAP_CAP(SYS_ID_AA64PFR0_EL1, ID_AA64PFR0_ASIMD_SHIFT, FTR_SIGNED, 0, CAP_HWCAP, HWCAP_ASIMD),
	HWCAP_CAP(SYS_ID_AA64PFR0_EL1, ID_AA64PFR0_ASIMD_SHIFT, FTR_SIGNED, 1, CAP_HWCAP, HWCAP_ASIMDHP),
	HWCAP_CAP(SYS_ID_AA64ISAR1_EL1, ID_AA64ISAR1_DPB_SHIFT, FTR_UNSIGNED, 1, CAP_HWCAP, HWCAP_DCPOP),
	HWCAP_CAP(SYS_ID_AA64ISAR1_EL1, ID_AA64ISAR1_JSCVT_SHIFT, FTR_UNSIGNED, 1, CAP_HWCAP, HWCAP_JSCVT),
	HWCAP_CAP(SYS_ID_AA64ISAR1_EL1, ID_AA64ISAR1_FCMA_SHIFT, FTR_UNSIGNED, 1, CAP_HWCAP, HWCAP_FCMA),
	HWCAP_CAP(SYS_ID_AA64ISAR1_EL1, ID_AA64ISAR1_LRCPC_SHIFT, FTR_UNSIGNED, 1, CAP_HWCAP, HWCAP_LRCPC),
#ifdef CONFIG_ARM64_SVE
	HWCAP_CAP(SYS_ID_AA64PFR0_EL1, ID_AA64PFR0_SVE_SHIFT, FTR_UNSIGNED, ID_AA64PFR0_SVE, CAP_HWCAP, HWCAP_SVE),
#endif
	{},
};

static const struct arm64_cpu_capabilities compat_elf_hwcaps[] = {
#ifdef CONFIG_COMPAT
	HWCAP_CAP(SYS_ID_ISAR5_EL1, ID_ISAR5_AES_SHIFT, FTR_UNSIGNED, 2, CAP_COMPAT_HWCAP2, COMPAT_HWCAP2_PMULL),
	HWCAP_CAP(SYS_ID_ISAR5_EL1, ID_ISAR5_AES_SHIFT, FTR_UNSIGNED, 1, CAP_COMPAT_HWCAP2, COMPAT_HWCAP2_AES),
	HWCAP_CAP(SYS_ID_ISAR5_EL1, ID_ISAR5_SHA1_SHIFT, FTR_UNSIGNED, 1, CAP_COMPAT_HWCAP2, COMPAT_HWCAP2_SHA1),
	HWCAP_CAP(SYS_ID_ISAR5_EL1, ID_ISAR5_SHA2_SHIFT, FTR_UNSIGNED, 1, CAP_COMPAT_HWCAP2, COMPAT_HWCAP2_SHA2),
	HWCAP_CAP(SYS_ID_ISAR5_EL1, ID_ISAR5_CRC32_SHIFT, FTR_UNSIGNED, 1, CAP_COMPAT_HWCAP2, COMPAT_HWCAP2_CRC32),
#endif
	{},
};

static void __init cap_set_elf_hwcap(const struct arm64_cpu_capabilities *cap)
{
	switch (cap->hwcap_type) {
	case CAP_HWCAP:
		elf_hwcap |= cap->hwcap;
		break;
#ifdef CONFIG_COMPAT
	case CAP_COMPAT_HWCAP:
		compat_elf_hwcap |= (u32)cap->hwcap;
		break;
	case CAP_COMPAT_HWCAP2:
		compat_elf_hwcap2 |= (u32)cap->hwcap;
		break;
#endif
	default:
		WARN_ON(1);
		break;
	}
}

/* Check if we have a particular HWCAP enabled */
static bool cpus_have_elf_hwcap(const struct arm64_cpu_capabilities *cap)
{
	bool rc;

	switch (cap->hwcap_type) {
	case CAP_HWCAP:
		rc = (elf_hwcap & cap->hwcap) != 0;
		break;
#ifdef CONFIG_COMPAT
	case CAP_COMPAT_HWCAP:
		rc = (compat_elf_hwcap & (u32)cap->hwcap) != 0;
		break;
	case CAP_COMPAT_HWCAP2:
		rc = (compat_elf_hwcap2 & (u32)cap->hwcap) != 0;
		break;
#endif
	default:
		WARN_ON(1);
		rc = false;
	}

	return rc;
}

static void __init setup_elf_hwcaps(const struct arm64_cpu_capabilities *hwcaps)
{
	/* We support emulation of accesses to CPU ID feature registers */
	elf_hwcap |= HWCAP_CPUID;
	for (; hwcaps->matches; hwcaps++)
		if (hwcaps->matches(hwcaps, hwcaps->def_scope))
			cap_set_elf_hwcap(hwcaps);
}

/*
 * Check if the current CPU has a given feature capability.
 * Should be called from non-preemptible context.
 */
static bool __this_cpu_has_cap(const struct arm64_cpu_capabilities *cap_array,
			       unsigned int cap)
{
	const struct arm64_cpu_capabilities *caps;

	if (WARN_ON(preemptible()))
		return false;

	for (caps = cap_array; caps->matches; caps++)
		if (caps->capability == cap &&
		    caps->matches(caps, SCOPE_LOCAL_CPU))
			return true;
	return false;
}

void update_cpu_capabilities(const struct arm64_cpu_capabilities *caps,
			    const char *info)
{
	for (; caps->matches; caps++) {
		if (!caps->matches(caps, caps->def_scope))
			continue;

		if (!cpus_have_cap(caps->capability) && caps->desc)
			pr_info("%s %s\n", info, caps->desc);
		cpus_set_cap(caps->capability);
	}
}

/*
 * Run through the enabled capabilities and enable() it on all active
 * CPUs
 */
void __init enable_cpu_capabilities(const struct arm64_cpu_capabilities *caps)
{
	for (; caps->matches; caps++) {
		unsigned int num = caps->capability;

		if (!cpus_have_cap(num))
			continue;

		/* Ensure cpus_have_const_cap(num) works */
		static_branch_enable(&cpu_hwcap_keys[num]);

		if (caps->enable) {
			/*
			 * Use stop_machine() as it schedules the work allowing
			 * us to modify PSTATE, instead of on_each_cpu() which
			 * uses an IPI, giving us a PSTATE that disappears when
			 * we return.
			 */
			stop_machine(caps->enable, (void *)caps, cpu_online_mask);
		}
	}
}

/*
 * Check for CPU features that are used in early boot
 * based on the Boot CPU value.
 */
static void check_early_cpu_features(void)
{
	verify_cpu_run_el();
	verify_cpu_asid_bits();
}

static void
verify_local_elf_hwcaps(const struct arm64_cpu_capabilities *caps)
{

	for (; caps->matches; caps++)
		if (cpus_have_elf_hwcap(caps) && !caps->matches(caps, SCOPE_LOCAL_CPU)) {
			pr_crit("CPU%d: missing HWCAP: %s\n",
					smp_processor_id(), caps->desc);
			cpu_die_early();
		}
}

static void
verify_local_cpu_features(const struct arm64_cpu_capabilities *caps_list)
{
	const struct arm64_cpu_capabilities *caps = caps_list;
	for (; caps->matches; caps++) {
		if (!cpus_have_cap(caps->capability))
			continue;
		/*
		 * If the new CPU misses an advertised feature, we cannot proceed
		 * further, park the cpu.
		 */
		if (!__this_cpu_has_cap(caps_list, caps->capability)) {
			pr_crit("CPU%d: missing feature: %s\n",
					smp_processor_id(), caps->desc);
			cpu_die_early();
		}
		if (caps->enable)
			caps->enable((void *)caps);
	}
}

static void verify_sve_features(void)
{
	u64 safe_zcr = read_sanitised_ftr_reg(SYS_ZCR_EL1);
	u64 zcr = read_zcr_features();

	unsigned int safe_len = safe_zcr & ZCR_ELx_LEN_MASK;
	unsigned int len = zcr & ZCR_ELx_LEN_MASK;

	if (len < safe_len || sve_verify_vq_map()) {
		pr_crit("CPU%d: SVE: required vector length(s) missing\n",
			smp_processor_id());
		cpu_die_early();
	}

	/* Add checks on other ZCR bits here if necessary */
}

/*
 * Run through the enabled system capabilities and enable() it on this CPU.
 * The capabilities were decided based on the available CPUs at the boot time.
 * Any new CPU should match the system wide status of the capability. If the
 * new CPU doesn't have a capability which the system now has enabled, we
 * cannot do anything to fix it up and could cause unexpected failures. So
 * we park the CPU.
 */
static void verify_local_cpu_capabilities(void)
{
	verify_local_cpu_errata_workarounds();
	verify_local_cpu_features(arm64_features);
	verify_local_elf_hwcaps(arm64_elf_hwcaps);

	if (system_supports_32bit_el0())
		verify_local_elf_hwcaps(compat_elf_hwcaps);

	if (system_supports_sve())
		verify_sve_features();

	if (system_uses_ttbr0_pan())
		pr_info("Emulating Privileged Access Never (PAN) using TTBR0_EL1 switching\n");
}

void check_local_cpu_capabilities(void)
{
	/*
	 * All secondary CPUs should conform to the early CPU features
	 * in use by the kernel based on boot CPU.
	 */
	check_early_cpu_features();

	/*
	 * If we haven't finalised the system capabilities, this CPU gets
	 * a chance to update the errata work arounds.
	 * Otherwise, this CPU should verify that it has all the system
	 * advertised capabilities.
	 */
	if (!sys_caps_initialised)
		update_cpu_errata_workarounds();
	else
		verify_local_cpu_capabilities();
}

static void __init setup_feature_capabilities(void)
{
	update_cpu_capabilities(arm64_features, "detected feature:");
	enable_cpu_capabilities(arm64_features);
}

DEFINE_STATIC_KEY_FALSE(arm64_const_caps_ready);
EXPORT_SYMBOL(arm64_const_caps_ready);

static void __init mark_const_caps_ready(void)
{
	static_branch_enable(&arm64_const_caps_ready);
}

extern const struct arm64_cpu_capabilities arm64_errata[];

bool this_cpu_has_cap(unsigned int cap)
{
	return (__this_cpu_has_cap(arm64_features, cap) ||
		__this_cpu_has_cap(arm64_errata, cap));
}

void __init setup_cpu_features(void)
{
	u32 cwg;
	int cls;

	/* Set the CPU feature capabilies */
	setup_feature_capabilities();
	enable_errata_workarounds();
	mark_const_caps_ready();
	setup_elf_hwcaps(arm64_elf_hwcaps);

	if (system_supports_32bit_el0())
		setup_elf_hwcaps(compat_elf_hwcaps);

	sve_setup();

	/* Advertise that we have computed the system capabilities */
	set_sys_caps_initialised();

	/*
	 * Check for sane CTR_EL0.CWG value.
	 */
	cwg = cache_type_cwg();
	cls = cache_line_size();
	if (!cwg)
		pr_warn("No Cache Writeback Granule information, assuming cache line size %d\n",
			cls);
	if (L1_CACHE_BYTES < cls)
		pr_warn("L1_CACHE_BYTES smaller than the Cache Writeback Granule (%d < %d)\n",
			L1_CACHE_BYTES, cls);
}

static bool __maybe_unused
cpufeature_pan_not_uao(const struct arm64_cpu_capabilities *entry, int __unused)
{
	return (cpus_have_const_cap(ARM64_HAS_PAN) && !cpus_have_const_cap(ARM64_HAS_UAO));
}

/*
 * We emulate only the following system register space.
 * Op0 = 0x3, CRn = 0x0, Op1 = 0x0, CRm = [0, 4 - 7]
 * See Table C5-6 System instruction encodings for System register accesses,
 * ARMv8 ARM(ARM DDI 0487A.f) for more details.
 */
static inline bool __attribute_const__ is_emulated(u32 id)
{
	return (sys_reg_Op0(id) == 0x3 &&
		sys_reg_CRn(id) == 0x0 &&
		sys_reg_Op1(id) == 0x0 &&
		(sys_reg_CRm(id) == 0 ||
		 ((sys_reg_CRm(id) >= 4) && (sys_reg_CRm(id) <= 7))));
}

/*
 * With CRm == 0, reg should be one of :
 * MIDR_EL1, MPIDR_EL1 or REVIDR_EL1.
 */
static inline int emulate_id_reg(u32 id, u64 *valp)
{
	switch (id) {
	case SYS_MIDR_EL1:
		*valp = read_cpuid_id();
		break;
	case SYS_MPIDR_EL1:
		*valp = SYS_MPIDR_SAFE_VAL;
		break;
	case SYS_REVIDR_EL1:
		/* IMPLEMENTATION DEFINED values are emulated with 0 */
		*valp = 0;
		break;
	default:
		return -EINVAL;
	}

	return 0;
}

static int emulate_sys_reg(u32 id, u64 *valp)
{
	struct arm64_ftr_reg *regp;

	if (!is_emulated(id))
		return -EINVAL;

	if (sys_reg_CRm(id) == 0)
		return emulate_id_reg(id, valp);

	regp = get_arm64_ftr_reg(id);
	if (regp)
		*valp = arm64_ftr_reg_user_value(regp);
	else
		/*
		 * The untracked registers are either IMPLEMENTATION DEFINED
		 * (e.g, ID_AFR0_EL1) or reserved RAZ.
		 */
		*valp = 0;
	return 0;
}

static int emulate_mrs(struct pt_regs *regs, u32 insn)
{
	int rc;
	u32 sys_reg, dst;
	u64 val;

	/*
	 * sys_reg values are defined as used in mrs/msr instruction.
	 * shift the imm value to get the encoding.
	 */
	sys_reg = (u32)aarch64_insn_decode_immediate(AARCH64_INSN_IMM_16, insn) << 5;
	rc = emulate_sys_reg(sys_reg, &val);
	if (!rc) {
		dst = aarch64_insn_decode_register(AARCH64_INSN_REGTYPE_RT, insn);
		pt_regs_write_reg(regs, dst, val);
		arm64_skip_faulting_instruction(regs, AARCH64_INSN_SIZE);
	}

	return rc;
}

static struct undef_hook mrs_hook = {
	.instr_mask = 0xfff00000,
	.instr_val  = 0xd5300000,
	.pstate_mask = COMPAT_PSR_MODE_MASK,
	.pstate_val = PSR_MODE_EL0t,
	.fn = emulate_mrs,
};

static int __init enable_mrs_emulation(void)
{
	register_undef_hook(&mrs_hook);
	return 0;
}

core_initcall(enable_mrs_emulation);

int cpu_clear_disr(void *__unused)
{
	/* Firmware may have left a deferred SError in this register. */
	write_sysreg_s(0, SYS_DISR_EL1);

	return 0;
}
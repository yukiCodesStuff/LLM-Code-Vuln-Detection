{
	/* Clear SCTLR_EL1.UCT */
	config_sctlr_el1(SCTLR_EL1_UCT, 0);
}

atomic_t arm64_el2_vector_last_slot = ATOMIC_INIT(-1);

#ifdef CONFIG_HARDEN_BRANCH_PREDICTOR
#include <asm/mmu_context.h>
#include <asm/cacheflush.h>

DEFINE_PER_CPU_READ_MOSTLY(struct bp_hardening_data, bp_hardening_data);

#ifdef CONFIG_KVM
extern char __smccc_workaround_1_smc_start[];
extern char __smccc_workaround_1_smc_end[];
extern char __smccc_workaround_1_hvc_start[];
extern char __smccc_workaround_1_hvc_end[];

static void __copy_hyp_vect_bpi(int slot, const char *hyp_vecs_start,
				const char *hyp_vecs_end)
{
	void *dst = lm_alias(__bp_harden_hyp_vecs_start + slot * SZ_2K);
	int i;

	for (i = 0; i < SZ_2K; i += 0x80)
		memcpy(dst + i, hyp_vecs_start, hyp_vecs_end - hyp_vecs_start);

	flush_icache_range((uintptr_t)dst, (uintptr_t)dst + SZ_2K);
}
	if (slot == -1) {
		slot = atomic_inc_return(&arm64_el2_vector_last_slot);
		BUG_ON(slot >= BP_HARDEN_EL2_SLOTS);
		__copy_hyp_vect_bpi(slot, hyp_vecs_start, hyp_vecs_end);
	}

	__this_cpu_write(bp_hardening_data.hyp_vectors_slot, slot);
	__this_cpu_write(bp_hardening_data.fn, fn);
	spin_unlock(&bp_lock);
}
#else
#define __smccc_workaround_1_smc_start		NULL
#define __smccc_workaround_1_smc_end		NULL
#define __smccc_workaround_1_hvc_start		NULL
#define __smccc_workaround_1_hvc_end		NULL

static void __install_bp_hardening_cb(bp_hardening_cb_t fn,
				      const char *hyp_vecs_start,
				      const char *hyp_vecs_end)
{
	__this_cpu_write(bp_hardening_data.fn, fn);
}
#endif	/* CONFIG_KVM */

static void  install_bp_hardening_cb(const struct arm64_cpu_capabilities *entry,
				     bp_hardening_cb_t fn,
				     const char *hyp_vecs_start,
				     const char *hyp_vecs_end)
{
	u64 pfr0;

	if (!entry->matches(entry, SCOPE_LOCAL_CPU))
		return;

	pfr0 = read_cpuid(ID_AA64PFR0_EL1);
	if (cpuid_feature_extract_unsigned_field(pfr0, ID_AA64PFR0_CSV2_SHIFT))
		return;

	__install_bp_hardening_cb(fn, hyp_vecs_start, hyp_vecs_end);
}

#include <uapi/linux/psci.h>
#include <linux/arm-smccc.h>
#include <linux/psci.h>

static void call_smc_arch_workaround_1(void)
{
	arm_smccc_1_1_smc(ARM_SMCCC_ARCH_WORKAROUND_1, NULL);
}

static void call_hvc_arch_workaround_1(void)
{
	arm_smccc_1_1_hvc(ARM_SMCCC_ARCH_WORKAROUND_1, NULL);
}

static void qcom_link_stack_sanitization(void)
{
	u64 tmp;

	asm volatile("mov	%0, x30		\n"
		     ".rept	16		\n"
		     "bl	. + 4		\n"
		     ".endr			\n"
		     "mov	x30, %0		\n"
		     : "=&r" (tmp));
}

static void
enable_smccc_arch_workaround_1(const struct arm64_cpu_capabilities *entry)
{
	bp_hardening_cb_t cb;
	void *smccc_start, *smccc_end;
	struct arm_smccc_res res;
	u32 midr = read_cpuid_id();

	if (!entry->matches(entry, SCOPE_LOCAL_CPU))
		return;

	if (psci_ops.smccc_version == SMCCC_VERSION_1_0)
		return;

	switch (psci_ops.conduit) {
	case PSCI_CONDUIT_HVC:
		arm_smccc_1_1_hvc(ARM_SMCCC_ARCH_FEATURES_FUNC_ID,
				  ARM_SMCCC_ARCH_WORKAROUND_1, &res);
		if ((int)res.a0 < 0)
			return;
		cb = call_hvc_arch_workaround_1;
		smccc_start = __smccc_workaround_1_hvc_start;
		smccc_end = __smccc_workaround_1_hvc_end;
		break;

	case PSCI_CONDUIT_SMC:
		arm_smccc_1_1_smc(ARM_SMCCC_ARCH_FEATURES_FUNC_ID,
				  ARM_SMCCC_ARCH_WORKAROUND_1, &res);
		if ((int)res.a0 < 0)
			return;
		cb = call_smc_arch_workaround_1;
		smccc_start = __smccc_workaround_1_smc_start;
		smccc_end = __smccc_workaround_1_smc_end;
		break;

	default:
		return;
	}
{
	arm_smccc_1_1_hvc(ARM_SMCCC_ARCH_WORKAROUND_1, NULL);
}

static void qcom_link_stack_sanitization(void)
{
	u64 tmp;

	asm volatile("mov	%0, x30		\n"
		     ".rept	16		\n"
		     "bl	. + 4		\n"
		     ".endr			\n"
		     "mov	x30, %0		\n"
		     : "=&r" (tmp));
}

static void
enable_smccc_arch_workaround_1(const struct arm64_cpu_capabilities *entry)
{
	bp_hardening_cb_t cb;
	void *smccc_start, *smccc_end;
	struct arm_smccc_res res;
	u32 midr = read_cpuid_id();

	if (!entry->matches(entry, SCOPE_LOCAL_CPU))
		return;

	if (psci_ops.smccc_version == SMCCC_VERSION_1_0)
		return;

	switch (psci_ops.conduit) {
	case PSCI_CONDUIT_HVC:
		arm_smccc_1_1_hvc(ARM_SMCCC_ARCH_FEATURES_FUNC_ID,
				  ARM_SMCCC_ARCH_WORKAROUND_1, &res);
		if ((int)res.a0 < 0)
			return;
		cb = call_hvc_arch_workaround_1;
		smccc_start = __smccc_workaround_1_hvc_start;
		smccc_end = __smccc_workaround_1_hvc_end;
		break;

	case PSCI_CONDUIT_SMC:
		arm_smccc_1_1_smc(ARM_SMCCC_ARCH_FEATURES_FUNC_ID,
				  ARM_SMCCC_ARCH_WORKAROUND_1, &res);
		if ((int)res.a0 < 0)
			return;
		cb = call_smc_arch_workaround_1;
		smccc_start = __smccc_workaround_1_smc_start;
		smccc_end = __smccc_workaround_1_smc_end;
		break;

	default:
		return;
	}
	switch (psci_ops.conduit) {
	case PSCI_CONDUIT_HVC:
		arm_smccc_1_1_hvc(ARM_SMCCC_ARCH_FEATURES_FUNC_ID,
				  ARM_SMCCC_ARCH_WORKAROUND_1, &res);
		if ((int)res.a0 < 0)
			return;
		cb = call_hvc_arch_workaround_1;
		smccc_start = __smccc_workaround_1_hvc_start;
		smccc_end = __smccc_workaround_1_hvc_end;
		break;

	case PSCI_CONDUIT_SMC:
		arm_smccc_1_1_smc(ARM_SMCCC_ARCH_FEATURES_FUNC_ID,
				  ARM_SMCCC_ARCH_WORKAROUND_1, &res);
		if ((int)res.a0 < 0)
			return;
		cb = call_smc_arch_workaround_1;
		smccc_start = __smccc_workaround_1_smc_start;
		smccc_end = __smccc_workaround_1_smc_end;
		break;

	default:
		return;
	}

	if (((midr & MIDR_CPU_MODEL_MASK) == MIDR_QCOM_FALKOR) ||
	    ((midr & MIDR_CPU_MODEL_MASK) == MIDR_QCOM_FALKOR_V1))
		cb = qcom_link_stack_sanitization;

	install_bp_hardening_cb(entry, cb, smccc_start, smccc_end);

	return;
}
#endif	/* CONFIG_HARDEN_BRANCH_PREDICTOR */

#define CAP_MIDR_RANGE(model, v_min, r_min, v_max, r_max)	\
	.matches = is_affected_midr_range,			\
	.midr_range = MIDR_RANGE(model, v_min, r_min, v_max, r_max)

#define CAP_MIDR_ALL_VERSIONS(model)					\
	.matches = is_affected_midr_range,				\
	.midr_range = MIDR_ALL_VERSIONS(model)

#define MIDR_FIXED(rev, revidr_mask) \
	.fixed_revs = (struct arm64_midr_revidr[]){{ (rev), (revidr_mask) }, {}}

#define ERRATA_MIDR_RANGE(model, v_min, r_min, v_max, r_max)		\
	.type = ARM64_CPUCAP_LOCAL_CPU_ERRATUM,				\
	CAP_MIDR_RANGE(model, v_min, r_min, v_max, r_max)

#define CAP_MIDR_RANGE_LIST(list)				\
	.matches = is_affected_midr_range_list,			\
	.midr_range_list = list

/* Errata affecting a range of revisions of  given model variant */
#define ERRATA_MIDR_REV_RANGE(m, var, r_min, r_max)	 \
	ERRATA_MIDR_RANGE(m, var, r_min, var, r_max)

/* Errata affecting a single variant/revision of a model */
#define ERRATA_MIDR_REV(model, var, rev)	\
	ERRATA_MIDR_RANGE(model, var, rev, var, rev)

/* Errata affecting all variants/revisions of a given a model */
#define ERRATA_MIDR_ALL_VERSIONS(model)				\
	.type = ARM64_CPUCAP_LOCAL_CPU_ERRATUM,			\
	CAP_MIDR_ALL_VERSIONS(model)

/* Errata affecting a list of midr ranges, with same work around */
#define ERRATA_MIDR_RANGE_LIST(midr_list)			\
	.type = ARM64_CPUCAP_LOCAL_CPU_ERRATUM,			\
	CAP_MIDR_RANGE_LIST(midr_list)

/*
 * Generic helper for handling capabilties with multiple (match,enable) pairs
 * of call backs, sharing the same capability bit.
 * Iterate over each entry to see if at least one matches.
 */
static bool __maybe_unused
multi_entry_cap_matches(const struct arm64_cpu_capabilities *entry, int scope)
{
	const struct arm64_cpu_capabilities *caps;

	for (caps = entry->match_list; caps->matches; caps++)
		if (caps->matches(caps, scope))
			return true;

	return false;
}

/*
 * Take appropriate action for all matching entries in the shared capability
 * entry.
 */
static void __maybe_unused
multi_entry_cap_cpu_enable(const struct arm64_cpu_capabilities *entry)
{
	const struct arm64_cpu_capabilities *caps;

	for (caps = entry->match_list; caps->matches; caps++)
		if (caps->matches(caps, SCOPE_LOCAL_CPU) &&
		    caps->cpu_enable)
			caps->cpu_enable(caps);
}

#ifdef CONFIG_HARDEN_BRANCH_PREDICTOR

/*
 * List of CPUs where we need to issue a psci call to
 * harden the branch predictor.
 */
static const struct midr_range arm64_bp_harden_smccc_cpus[] = {
	MIDR_ALL_VERSIONS(MIDR_CORTEX_A57),
	MIDR_ALL_VERSIONS(MIDR_CORTEX_A72),
	MIDR_ALL_VERSIONS(MIDR_CORTEX_A73),
	MIDR_ALL_VERSIONS(MIDR_CORTEX_A75),
	MIDR_ALL_VERSIONS(MIDR_BRCM_VULCAN),
	MIDR_ALL_VERSIONS(MIDR_CAVIUM_THUNDERX2),
	MIDR_ALL_VERSIONS(MIDR_QCOM_FALKOR_V1),
	MIDR_ALL_VERSIONS(MIDR_QCOM_FALKOR),
	{},
};

#endif

#ifndef ERRATA_MIDR_ALL_VERSIONS
#define	ERRATA_MIDR_ALL_VERSIONS(x)	MIDR_ALL_VERSIONS(x)
#endif

const struct arm64_cpu_capabilities arm64_errata[] = {
#if	defined(CONFIG_ARM64_ERRATUM_826319) || \
	defined(CONFIG_ARM64_ERRATUM_827319) || \
	defined(CONFIG_ARM64_ERRATUM_824069)
	{
	/* Cortex-A53 r0p[012] */
		.desc = "ARM errata 826319, 827319, 824069",
		.capability = ARM64_WORKAROUND_CLEAN_CACHE,
		ERRATA_MIDR_REV_RANGE(MIDR_CORTEX_A53, 0, 0, 2),
		.cpu_enable = cpu_enable_cache_maint_trap,
	},
#endif
#ifdef CONFIG_ARM64_ERRATUM_819472
	{
	/* Cortex-A53 r0p[01] */
		.desc = "ARM errata 819472",
		.capability = ARM64_WORKAROUND_CLEAN_CACHE,
		ERRATA_MIDR_REV_RANGE(MIDR_CORTEX_A53, 0, 0, 1),
		.cpu_enable = cpu_enable_cache_maint_trap,
	},
#endif
#ifdef CONFIG_ARM64_ERRATUM_832075
	{
	/* Cortex-A57 r0p0 - r1p2 */
		.desc = "ARM erratum 832075",
		.capability = ARM64_WORKAROUND_DEVICE_LOAD_ACQUIRE,
		ERRATA_MIDR_RANGE(MIDR_CORTEX_A57,
				  0, 0,
				  1, 2),
	},
#endif
#ifdef CONFIG_ARM64_ERRATUM_834220
	{
	/* Cortex-A57 r0p0 - r1p2 */
		.desc = "ARM erratum 834220",
		.capability = ARM64_WORKAROUND_834220,
		ERRATA_MIDR_RANGE(MIDR_CORTEX_A57,
				  0, 0,
				  1, 2),
	},
#endif
#ifdef CONFIG_ARM64_ERRATUM_843419
	{
	/* Cortex-A53 r0p[01234] */
		.desc = "ARM erratum 843419",
		.capability = ARM64_WORKAROUND_843419,
		ERRATA_MIDR_REV_RANGE(MIDR_CORTEX_A53, 0, 0, 4),
		MIDR_FIXED(0x4, BIT(8)),
	},
#endif
#ifdef CONFIG_ARM64_ERRATUM_845719
	{
	/* Cortex-A53 r0p[01234] */
		.desc = "ARM erratum 845719",
		.capability = ARM64_WORKAROUND_845719,
		ERRATA_MIDR_REV_RANGE(MIDR_CORTEX_A53, 0, 0, 4),
	},
#endif
#ifdef CONFIG_CAVIUM_ERRATUM_23154
	{
	/* Cavium ThunderX, pass 1.x */
		.desc = "Cavium erratum 23154",
		.capability = ARM64_WORKAROUND_CAVIUM_23154,
		ERRATA_MIDR_REV_RANGE(MIDR_THUNDERX, 0, 0, 1),
	},
#endif
#ifdef CONFIG_CAVIUM_ERRATUM_27456
	{
	/* Cavium ThunderX, T88 pass 1.x - 2.1 */
		.desc = "Cavium erratum 27456",
		.capability = ARM64_WORKAROUND_CAVIUM_27456,
		ERRATA_MIDR_RANGE(MIDR_THUNDERX,
				  0, 0,
				  1, 1),
	},
	{
	/* Cavium ThunderX, T81 pass 1.0 */
		.desc = "Cavium erratum 27456",
		.capability = ARM64_WORKAROUND_CAVIUM_27456,
		ERRATA_MIDR_REV(MIDR_THUNDERX_81XX, 0, 0),
	},
#endif
#ifdef CONFIG_CAVIUM_ERRATUM_30115
	{
	/* Cavium ThunderX, T88 pass 1.x - 2.2 */
		.desc = "Cavium erratum 30115",
		.capability = ARM64_WORKAROUND_CAVIUM_30115,
		ERRATA_MIDR_RANGE(MIDR_THUNDERX,
				      0, 0,
				      1, 2),
	},
	{
	/* Cavium ThunderX, T81 pass 1.0 - 1.2 */
		.desc = "Cavium erratum 30115",
		.capability = ARM64_WORKAROUND_CAVIUM_30115,
		ERRATA_MIDR_REV_RANGE(MIDR_THUNDERX_81XX, 0, 0, 2),
	},
	{
	/* Cavium ThunderX, T83 pass 1.0 */
		.desc = "Cavium erratum 30115",
		.capability = ARM64_WORKAROUND_CAVIUM_30115,
		ERRATA_MIDR_REV(MIDR_THUNDERX_83XX, 0, 0),
	},
#endif
	{
		.desc = "Mismatched cache line size",
		.capability = ARM64_MISMATCHED_CACHE_LINE_SIZE,
		.matches = has_mismatched_cache_line_size,
		.type = ARM64_CPUCAP_LOCAL_CPU_ERRATUM,
		.cpu_enable = cpu_enable_trap_ctr_access,
	},
#ifdef CONFIG_QCOM_FALKOR_ERRATUM_1003
	{
		.desc = "Qualcomm Technologies Falkor erratum 1003",
		.capability = ARM64_WORKAROUND_QCOM_FALKOR_E1003,
		ERRATA_MIDR_REV(MIDR_QCOM_FALKOR_V1, 0, 0),
	},
	{
		.desc = "Qualcomm Technologies Kryo erratum 1003",
		.capability = ARM64_WORKAROUND_QCOM_FALKOR_E1003,
		.type = ARM64_CPUCAP_LOCAL_CPU_ERRATUM,
		.midr_range.model = MIDR_QCOM_KRYO,
		.matches = is_kryo_midr,
	},
#endif
#ifdef CONFIG_QCOM_FALKOR_ERRATUM_1009
	{
		.desc = "Qualcomm Technologies Falkor erratum 1009",
		.capability = ARM64_WORKAROUND_REPEAT_TLBI,
		ERRATA_MIDR_REV(MIDR_QCOM_FALKOR_V1, 0, 0),
	},
#endif
#ifdef CONFIG_ARM64_ERRATUM_858921
	{
	/* Cortex-A73 all versions */
		.desc = "ARM erratum 858921",
		.capability = ARM64_WORKAROUND_858921,
		ERRATA_MIDR_ALL_VERSIONS(MIDR_CORTEX_A73),
	},
#endif
#ifdef CONFIG_HARDEN_BRANCH_PREDICTOR
	{
		.capability = ARM64_HARDEN_BRANCH_PREDICTOR,
		.type = ARM64_CPUCAP_LOCAL_CPU_ERRATUM,
		.cpu_enable = enable_smccc_arch_workaround_1,
		ERRATA_MIDR_RANGE_LIST(arm64_bp_harden_smccc_cpus),
	},
#endif
#ifdef CONFIG_HARDEN_EL2_VECTORS
	{
		.desc = "Cortex-A57 EL2 vector hardening",
		.capability = ARM64_HARDEN_EL2_VECTORS,
		ERRATA_MIDR_ALL_VERSIONS(MIDR_CORTEX_A57),
	},
	{
		.desc = "Cortex-A72 EL2 vector hardening",
		.capability = ARM64_HARDEN_EL2_VECTORS,
		ERRATA_MIDR_ALL_VERSIONS(MIDR_CORTEX_A72),
	},
#endif
	{
	}
};
static const struct midr_range arm64_bp_harden_smccc_cpus[] = {
	MIDR_ALL_VERSIONS(MIDR_CORTEX_A57),
	MIDR_ALL_VERSIONS(MIDR_CORTEX_A72),
	MIDR_ALL_VERSIONS(MIDR_CORTEX_A73),
	MIDR_ALL_VERSIONS(MIDR_CORTEX_A75),
	MIDR_ALL_VERSIONS(MIDR_BRCM_VULCAN),
	MIDR_ALL_VERSIONS(MIDR_CAVIUM_THUNDERX2),
	MIDR_ALL_VERSIONS(MIDR_QCOM_FALKOR_V1),
	MIDR_ALL_VERSIONS(MIDR_QCOM_FALKOR),
	{},
};

#endif

#ifndef ERRATA_MIDR_ALL_VERSIONS
#define	ERRATA_MIDR_ALL_VERSIONS(x)	MIDR_ALL_VERSIONS(x)
#endif

const struct arm64_cpu_capabilities arm64_errata[] = {
#if	defined(CONFIG_ARM64_ERRATUM_826319) || \
	defined(CONFIG_ARM64_ERRATUM_827319) || \
	defined(CONFIG_ARM64_ERRATUM_824069)
	{
	/* Cortex-A53 r0p[012] */
		.desc = "ARM errata 826319, 827319, 824069",
		.capability = ARM64_WORKAROUND_CLEAN_CACHE,
		ERRATA_MIDR_REV_RANGE(MIDR_CORTEX_A53, 0, 0, 2),
		.cpu_enable = cpu_enable_cache_maint_trap,
	},
#endif
#ifdef CONFIG_ARM64_ERRATUM_819472
	{
	/* Cortex-A53 r0p[01] */
		.desc = "ARM errata 819472",
		.capability = ARM64_WORKAROUND_CLEAN_CACHE,
		ERRATA_MIDR_REV_RANGE(MIDR_CORTEX_A53, 0, 0, 1),
		.cpu_enable = cpu_enable_cache_maint_trap,
	},
#endif
#ifdef CONFIG_ARM64_ERRATUM_832075
	{
	/* Cortex-A57 r0p0 - r1p2 */
		.desc = "ARM erratum 832075",
		.capability = ARM64_WORKAROUND_DEVICE_LOAD_ACQUIRE,
		ERRATA_MIDR_RANGE(MIDR_CORTEX_A57,
				  0, 0,
				  1, 2),
	},
#endif
#ifdef CONFIG_ARM64_ERRATUM_834220
	{
	/* Cortex-A57 r0p0 - r1p2 */
		.desc = "ARM erratum 834220",
		.capability = ARM64_WORKAROUND_834220,
		ERRATA_MIDR_RANGE(MIDR_CORTEX_A57,
				  0, 0,
				  1, 2),
	},
#endif
#ifdef CONFIG_ARM64_ERRATUM_843419
	{
	/* Cortex-A53 r0p[01234] */
		.desc = "ARM erratum 843419",
		.capability = ARM64_WORKAROUND_843419,
		ERRATA_MIDR_REV_RANGE(MIDR_CORTEX_A53, 0, 0, 4),
		MIDR_FIXED(0x4, BIT(8)),
	},
#endif
#ifdef CONFIG_ARM64_ERRATUM_845719
	{
	/* Cortex-A53 r0p[01234] */
		.desc = "ARM erratum 845719",
		.capability = ARM64_WORKAROUND_845719,
		ERRATA_MIDR_REV_RANGE(MIDR_CORTEX_A53, 0, 0, 4),
	},
#endif
#ifdef CONFIG_CAVIUM_ERRATUM_23154
	{
	/* Cavium ThunderX, pass 1.x */
		.desc = "Cavium erratum 23154",
		.capability = ARM64_WORKAROUND_CAVIUM_23154,
		ERRATA_MIDR_REV_RANGE(MIDR_THUNDERX, 0, 0, 1),
	},
#endif
#ifdef CONFIG_CAVIUM_ERRATUM_27456
	{
	/* Cavium ThunderX, T88 pass 1.x - 2.1 */
		.desc = "Cavium erratum 27456",
		.capability = ARM64_WORKAROUND_CAVIUM_27456,
		ERRATA_MIDR_RANGE(MIDR_THUNDERX,
				  0, 0,
				  1, 1),
	},
	{
	/* Cavium ThunderX, T81 pass 1.0 */
		.desc = "Cavium erratum 27456",
		.capability = ARM64_WORKAROUND_CAVIUM_27456,
		ERRATA_MIDR_REV(MIDR_THUNDERX_81XX, 0, 0),
	},
#endif
#ifdef CONFIG_CAVIUM_ERRATUM_30115
	{
	/* Cavium ThunderX, T88 pass 1.x - 2.2 */
		.desc = "Cavium erratum 30115",
		.capability = ARM64_WORKAROUND_CAVIUM_30115,
		ERRATA_MIDR_RANGE(MIDR_THUNDERX,
				      0, 0,
				      1, 2),
	},
	{
	/* Cavium ThunderX, T81 pass 1.0 - 1.2 */
		.desc = "Cavium erratum 30115",
		.capability = ARM64_WORKAROUND_CAVIUM_30115,
		ERRATA_MIDR_REV_RANGE(MIDR_THUNDERX_81XX, 0, 0, 2),
	},
	{
	/* Cavium ThunderX, T83 pass 1.0 */
		.desc = "Cavium erratum 30115",
		.capability = ARM64_WORKAROUND_CAVIUM_30115,
		ERRATA_MIDR_REV(MIDR_THUNDERX_83XX, 0, 0),
	},
#endif
	{
		.desc = "Mismatched cache line size",
		.capability = ARM64_MISMATCHED_CACHE_LINE_SIZE,
		.matches = has_mismatched_cache_line_size,
		.type = ARM64_CPUCAP_LOCAL_CPU_ERRATUM,
		.cpu_enable = cpu_enable_trap_ctr_access,
	},
#ifdef CONFIG_QCOM_FALKOR_ERRATUM_1003
	{
		.desc = "Qualcomm Technologies Falkor erratum 1003",
		.capability = ARM64_WORKAROUND_QCOM_FALKOR_E1003,
		ERRATA_MIDR_REV(MIDR_QCOM_FALKOR_V1, 0, 0),
	},
	{
		.desc = "Qualcomm Technologies Kryo erratum 1003",
		.capability = ARM64_WORKAROUND_QCOM_FALKOR_E1003,
		.type = ARM64_CPUCAP_LOCAL_CPU_ERRATUM,
		.midr_range.model = MIDR_QCOM_KRYO,
		.matches = is_kryo_midr,
	},
#endif
#ifdef CONFIG_QCOM_FALKOR_ERRATUM_1009
	{
		.desc = "Qualcomm Technologies Falkor erratum 1009",
		.capability = ARM64_WORKAROUND_REPEAT_TLBI,
		ERRATA_MIDR_REV(MIDR_QCOM_FALKOR_V1, 0, 0),
	},
#endif
#ifdef CONFIG_ARM64_ERRATUM_858921
	{
	/* Cortex-A73 all versions */
		.desc = "ARM erratum 858921",
		.capability = ARM64_WORKAROUND_858921,
		ERRATA_MIDR_ALL_VERSIONS(MIDR_CORTEX_A73),
	},
#endif
#ifdef CONFIG_HARDEN_BRANCH_PREDICTOR
	{
		.capability = ARM64_HARDEN_BRANCH_PREDICTOR,
		.type = ARM64_CPUCAP_LOCAL_CPU_ERRATUM,
		.cpu_enable = enable_smccc_arch_workaround_1,
		ERRATA_MIDR_RANGE_LIST(arm64_bp_harden_smccc_cpus),
	},
#endif
#ifdef CONFIG_HARDEN_EL2_VECTORS
	{
		.desc = "Cortex-A57 EL2 vector hardening",
		.capability = ARM64_HARDEN_EL2_VECTORS,
		ERRATA_MIDR_ALL_VERSIONS(MIDR_CORTEX_A57),
	},
	{
		.desc = "Cortex-A72 EL2 vector hardening",
		.capability = ARM64_HARDEN_EL2_VECTORS,
		ERRATA_MIDR_ALL_VERSIONS(MIDR_CORTEX_A72),
	},
#endif
	{
	}
};
	{
		.capability = ARM64_HARDEN_BRANCH_PREDICTOR,
		.type = ARM64_CPUCAP_LOCAL_CPU_ERRATUM,
		.cpu_enable = enable_smccc_arch_workaround_1,
		ERRATA_MIDR_RANGE_LIST(arm64_bp_harden_smccc_cpus),
	},
#endif
#ifdef CONFIG_HARDEN_EL2_VECTORS
	{
		.desc = "Cortex-A57 EL2 vector hardening",
		.capability = ARM64_HARDEN_EL2_VECTORS,
		ERRATA_MIDR_ALL_VERSIONS(MIDR_CORTEX_A57),
	},
	{
		.desc = "Cortex-A72 EL2 vector hardening",
		.capability = ARM64_HARDEN_EL2_VECTORS,
		ERRATA_MIDR_ALL_VERSIONS(MIDR_CORTEX_A72),
	},
#endif
	{
	}
};
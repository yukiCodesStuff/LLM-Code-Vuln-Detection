DEFINE_PER_CPU_READ_MOSTLY(struct bp_hardening_data, bp_hardening_data);

#ifdef CONFIG_KVM
extern char __qcom_hyp_sanitize_link_stack_start[];
extern char __qcom_hyp_sanitize_link_stack_end[];
extern char __smccc_workaround_1_smc_start[];
extern char __smccc_workaround_1_smc_end[];
extern char __smccc_workaround_1_hvc_start[];
extern char __smccc_workaround_1_hvc_end[];
	spin_unlock(&bp_lock);
}
#else
#define __qcom_hyp_sanitize_link_stack_start	NULL
#define __qcom_hyp_sanitize_link_stack_end	NULL
#define __smccc_workaround_1_smc_start		NULL
#define __smccc_workaround_1_smc_end		NULL
#define __smccc_workaround_1_hvc_start		NULL
#define __smccc_workaround_1_hvc_end		NULL
	arm_smccc_1_1_hvc(ARM_SMCCC_ARCH_WORKAROUND_1, NULL);
}

static void
enable_smccc_arch_workaround_1(const struct arm64_cpu_capabilities *entry)
{
	bp_hardening_cb_t cb;
	void *smccc_start, *smccc_end;
	struct arm_smccc_res res;

	if (!entry->matches(entry, SCOPE_LOCAL_CPU))
		return;

		return;
	}

	install_bp_hardening_cb(entry, cb, smccc_start, smccc_end);

	return;
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
qcom_enable_link_stack_sanitization(const struct arm64_cpu_capabilities *entry)
{
	install_bp_hardening_cb(entry, qcom_link_stack_sanitization,
				__qcom_hyp_sanitize_link_stack_start,
				__qcom_hyp_sanitize_link_stack_end);
}
#endif	/* CONFIG_HARDEN_BRANCH_PREDICTOR */

#define CAP_MIDR_RANGE(model, v_min, r_min, v_max, r_max)	\
	.matches = is_affected_midr_range,			\
	MIDR_ALL_VERSIONS(MIDR_CORTEX_A75),
	MIDR_ALL_VERSIONS(MIDR_BRCM_VULCAN),
	MIDR_ALL_VERSIONS(MIDR_CAVIUM_THUNDERX2),
	{},
};

static const struct midr_range qcom_bp_harden_cpus[] = {
	MIDR_ALL_VERSIONS(MIDR_QCOM_FALKOR_V1),
	MIDR_ALL_VERSIONS(MIDR_QCOM_FALKOR),
	{},
};

static const struct arm64_cpu_capabilities arm64_bp_harden_list[] = {
	{
		CAP_MIDR_RANGE_LIST(arm64_bp_harden_smccc_cpus),
		.cpu_enable = enable_smccc_arch_workaround_1,
	},
	{
		CAP_MIDR_RANGE_LIST(qcom_bp_harden_cpus),
		.cpu_enable = qcom_enable_link_stack_sanitization,
	},
	{},
};

#endif

#ifndef ERRATA_MIDR_ALL_VERSIONS
#define	ERRATA_MIDR_ALL_VERSIONS(x)	MIDR_ALL_VERSIONS(x)
	{
		.capability = ARM64_HARDEN_BRANCH_PREDICTOR,
		.type = ARM64_CPUCAP_LOCAL_CPU_ERRATUM,
		.matches = multi_entry_cap_matches,
		.cpu_enable = multi_entry_cap_cpu_enable,
		.match_list = arm64_bp_harden_list,
	},
	{
		.capability = ARM64_HARDEN_BP_POST_GUEST_EXIT,
		ERRATA_MIDR_RANGE_LIST(qcom_bp_harden_cpus),
	},
#endif
#ifdef CONFIG_HARDEN_EL2_VECTORS
	{
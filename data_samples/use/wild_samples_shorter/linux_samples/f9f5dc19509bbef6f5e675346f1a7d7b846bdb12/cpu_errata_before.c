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

static int enable_smccc_arch_workaround_1(void *data)
{
	const struct arm64_cpu_capabilities *entry = data;
	bp_hardening_cb_t cb;
	void *smccc_start, *smccc_end;
	struct arm_smccc_res res;

	if (!entry->matches(entry, SCOPE_LOCAL_CPU))
		return 0;

		return 0;
	}

	install_bp_hardening_cb(entry, cb, smccc_start, smccc_end);

	return 0;
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

static int qcom_enable_link_stack_sanitization(void *data)
{
	const struct arm64_cpu_capabilities *entry = data;

	install_bp_hardening_cb(entry, qcom_link_stack_sanitization,
				__qcom_hyp_sanitize_link_stack_start,
				__qcom_hyp_sanitize_link_stack_end);

	return 0;
}
#endif	/* CONFIG_HARDEN_BRANCH_PREDICTOR */

#define MIDR_RANGE(model, min, max) \
	.def_scope = SCOPE_LOCAL_CPU, \
	{
		.capability = ARM64_HARDEN_BRANCH_PREDICTOR,
		MIDR_ALL_VERSIONS(MIDR_QCOM_FALKOR_V1),
		.enable = qcom_enable_link_stack_sanitization,
	},
	{
		.capability = ARM64_HARDEN_BP_POST_GUEST_EXIT,
		MIDR_ALL_VERSIONS(MIDR_QCOM_FALKOR_V1),
	},
	{
		.capability = ARM64_HARDEN_BRANCH_PREDICTOR,
		MIDR_ALL_VERSIONS(MIDR_QCOM_FALKOR),
		.enable = qcom_enable_link_stack_sanitization,
	},
	{
		.capability = ARM64_HARDEN_BP_POST_GUEST_EXIT,
		MIDR_ALL_VERSIONS(MIDR_QCOM_FALKOR),
	},
	{
		.capability = ARM64_HARDEN_BRANCH_PREDICTOR,
		MIDR_ALL_VERSIONS(MIDR_BRCM_VULCAN),
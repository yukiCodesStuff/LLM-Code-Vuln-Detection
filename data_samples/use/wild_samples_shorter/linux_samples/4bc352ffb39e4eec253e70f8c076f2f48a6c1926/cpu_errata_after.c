DEFINE_PER_CPU_READ_MOSTLY(struct bp_hardening_data, bp_hardening_data);

#ifdef CONFIG_KVM
extern char __smccc_workaround_1_smc_start[];
extern char __smccc_workaround_1_smc_end[];
extern char __smccc_workaround_1_hvc_start[];
extern char __smccc_workaround_1_hvc_end[];
	spin_unlock(&bp_lock);
}
#else
#define __smccc_workaround_1_smc_start		NULL
#define __smccc_workaround_1_smc_end		NULL
#define __smccc_workaround_1_hvc_start		NULL
#define __smccc_workaround_1_hvc_end		NULL
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
	{
		.capability = ARM64_HARDEN_BRANCH_PREDICTOR,
		.type = ARM64_CPUCAP_LOCAL_CPU_ERRATUM,
		.cpu_enable = enable_smccc_arch_workaround_1,
		ERRATA_MIDR_RANGE_LIST(arm64_bp_harden_smccc_cpus),
	},
#endif
#ifdef CONFIG_HARDEN_EL2_VECTORS
	{
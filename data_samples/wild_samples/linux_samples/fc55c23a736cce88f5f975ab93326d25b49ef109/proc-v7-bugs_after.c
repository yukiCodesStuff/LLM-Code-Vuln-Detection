// SPDX-License-Identifier: GPL-2.0
#include <linux/arm-smccc.h>
#include <linux/kernel.h>
#include <linux/smp.h>

#include <asm/cp15.h>
#include <asm/cputype.h>
#include <asm/proc-fns.h>
#include <asm/spectre.h>
#include <asm/system_misc.h>

#ifdef CONFIG_ARM_PSCI
static int __maybe_unused spectre_v2_get_cpu_fw_mitigation_state(void)
{
	struct arm_smccc_res res;

	arm_smccc_1_1_invoke(ARM_SMCCC_ARCH_FEATURES_FUNC_ID,
			     ARM_SMCCC_ARCH_WORKAROUND_1, &res);

	switch ((int)res.a0) {
	case SMCCC_RET_SUCCESS:
		return SPECTRE_MITIGATED;

	case SMCCC_ARCH_WORKAROUND_RET_UNAFFECTED:
		return SPECTRE_UNAFFECTED;

	default:
		return SPECTRE_VULNERABLE;
	}
}
{
	arm_smccc_1_1_hvc(ARM_SMCCC_ARCH_WORKAROUND_1, NULL);
}

static unsigned int spectre_v2_install_workaround(unsigned int method)
{
	const char *spectre_v2_method = NULL;
	int cpu = smp_processor_id();

	if (per_cpu(harden_branch_predictor_fn, cpu))
		return SPECTRE_MITIGATED;

	switch (method) {
	case SPECTRE_V2_METHOD_BPIALL:
		per_cpu(harden_branch_predictor_fn, cpu) =
			harden_branch_predictor_bpiall;
		spectre_v2_method = "BPIALL";
		break;

	case SPECTRE_V2_METHOD_ICIALLU:
		per_cpu(harden_branch_predictor_fn, cpu) =
			harden_branch_predictor_iciallu;
		spectre_v2_method = "ICIALLU";
		break;

	case SPECTRE_V2_METHOD_HVC:
		per_cpu(harden_branch_predictor_fn, cpu) =
			call_hvc_arch_workaround_1;
		cpu_do_switch_mm = cpu_v7_hvc_switch_mm;
		spectre_v2_method = "hypervisor";
		break;

	case SPECTRE_V2_METHOD_SMC:
		per_cpu(harden_branch_predictor_fn, cpu) =
			call_smc_arch_workaround_1;
		cpu_do_switch_mm = cpu_v7_smc_switch_mm;
		spectre_v2_method = "firmware";
		break;
	}
	switch (read_cpuid_part()) {
	case ARM_CPU_PART_CORTEX_A8:
	case ARM_CPU_PART_CORTEX_A9:
	case ARM_CPU_PART_CORTEX_A12:
	case ARM_CPU_PART_CORTEX_A17:
	case ARM_CPU_PART_CORTEX_A73:
	case ARM_CPU_PART_CORTEX_A75:
		state = SPECTRE_MITIGATED;
		method = SPECTRE_V2_METHOD_BPIALL;
		break;

	case ARM_CPU_PART_CORTEX_A15:
	case ARM_CPU_PART_BRAHMA_B15:
		state = SPECTRE_MITIGATED;
		method = SPECTRE_V2_METHOD_ICIALLU;
		break;

	case ARM_CPU_PART_BRAHMA_B53:
		/* Requires no workaround */
		state = SPECTRE_UNAFFECTED;
		break;

	default:
		/* Other ARM CPUs require no workaround */
		if (read_cpuid_implementor() == ARM_CPU_IMP_ARM) {
			state = SPECTRE_UNAFFECTED;
			break;
		}
{
	if (check_spectre_auxcr(this_cpu_ptr(&spectre_warned), BIT(6)))
		cpu_v7_spectre_v2_init();
}
#include <asm/cp15.h>
#include <asm/cputype.h>
#include <asm/proc-fns.h>
#include <asm/system_misc.h>

#ifdef CONFIG_HARDEN_BRANCH_PREDICTOR
DEFINE_PER_CPU(harden_branch_predictor_fn_t, harden_branch_predictor_fn);

extern void cpu_v7_iciallu_switch_mm(phys_addr_t pgd_phys, struct mm_struct *mm);
	arm_smccc_1_1_hvc(ARM_SMCCC_ARCH_WORKAROUND_1, NULL);
}

static void cpu_v7_spectre_init(void)
{
	const char *spectre_v2_method = NULL;
	int cpu = smp_processor_id();

	if (per_cpu(harden_branch_predictor_fn, cpu))
		return;

	switch (read_cpuid_part()) {
	case ARM_CPU_PART_CORTEX_A8:
	case ARM_CPU_PART_CORTEX_A9:
	case ARM_CPU_PART_CORTEX_A17:
	case ARM_CPU_PART_CORTEX_A73:
	case ARM_CPU_PART_CORTEX_A75:
		per_cpu(harden_branch_predictor_fn, cpu) =
			harden_branch_predictor_bpiall;
		spectre_v2_method = "BPIALL";
		break;

	case ARM_CPU_PART_CORTEX_A15:
	case ARM_CPU_PART_BRAHMA_B15:
		per_cpu(harden_branch_predictor_fn, cpu) =
			harden_branch_predictor_iciallu;
		spectre_v2_method = "ICIALLU";
		break;

#ifdef CONFIG_ARM_PSCI
	case ARM_CPU_PART_BRAHMA_B53:
		/* Requires no workaround */
		break;
	default:
		/* Other ARM CPUs require no workaround */
		if (read_cpuid_implementor() == ARM_CPU_IMP_ARM)
			break;
		fallthrough;
		/* Cortex A57/A72 require firmware workaround */
	case ARM_CPU_PART_CORTEX_A57:
	case ARM_CPU_PART_CORTEX_A72: {
		struct arm_smccc_res res;

		arm_smccc_1_1_invoke(ARM_SMCCC_ARCH_FEATURES_FUNC_ID,
				     ARM_SMCCC_ARCH_WORKAROUND_1, &res);
		if ((int)res.a0 != 0)
			return;

		switch (arm_smccc_1_1_get_conduit()) {
		case SMCCC_CONDUIT_HVC:
			per_cpu(harden_branch_predictor_fn, cpu) =
				call_hvc_arch_workaround_1;
			cpu_do_switch_mm = cpu_v7_hvc_switch_mm;
			spectre_v2_method = "hypervisor";
			break;

		case SMCCC_CONDUIT_SMC:
			per_cpu(harden_branch_predictor_fn, cpu) =
				call_smc_arch_workaround_1;
			cpu_do_switch_mm = cpu_v7_smc_switch_mm;
			spectre_v2_method = "firmware";
			break;

		default:
			break;
		}
	}
#endif
	}

	if (spectre_v2_method)
		pr_info("CPU%u: Spectre v2: using %s workaround\n",
			smp_processor_id(), spectre_v2_method);
}
#else
static void cpu_v7_spectre_init(void)
{
}
#endif

static __maybe_unused bool cpu_v7_check_auxcr_set(bool *warned,
						  u32 mask, const char *msg)
{
	u32 aux_cr;
void cpu_v7_ca8_ibe(void)
{
	if (check_spectre_auxcr(this_cpu_ptr(&spectre_warned), BIT(6)))
		cpu_v7_spectre_init();
}

void cpu_v7_ca15_ibe(void)
{
	if (check_spectre_auxcr(this_cpu_ptr(&spectre_warned), BIT(0)))
		cpu_v7_spectre_init();
}

void cpu_v7_bugs_init(void)
{
	cpu_v7_spectre_init();
}
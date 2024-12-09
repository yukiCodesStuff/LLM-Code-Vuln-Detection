static void __init spectre_v2_select_mitigation(void);
static void __init ssb_select_mitigation(void);
static void __init l1tf_select_mitigation(void);

/* The base value of the SPEC_CTRL MSR that always has to be preserved. */
u64 x86_spec_ctrl_base;
EXPORT_SYMBOL_GPL(x86_spec_ctrl_base);
/* Control unconditional IBPB in switch_mm() */
DEFINE_STATIC_KEY_FALSE(switch_mm_always_ibpb);

void __init check_bugs(void)
{
	identify_boot_cpu();


	l1tf_select_mitigation();

#ifdef CONFIG_X86_32
	/*
	 * Check whether we are able to run this kernel safely on SMP.
	 *
		wrmsrl(MSR_AMD64_LS_CFG, msrval);
}

#undef pr_fmt
#define pr_fmt(fmt)     "Spectre V2 : " fmt

static enum spectre_v2_mitigation spectre_v2_enabled __ro_after_init =

	/* Set up IBPB and STIBP depending on the general spectre V2 command */
	spectre_v2_user_select_mitigation(cmd);

	/* Enable STIBP if appropriate */
	arch_smt_update();
}

static void update_stibp_msr(void * __unused)
{
		static_branch_disable(&switch_to_cond_stibp);
}

void arch_smt_update(void)
{
	/* Enhanced IBRS implies STIBP. No update required. */
	if (spectre_v2_enabled == SPECTRE_V2_IBRS_ENHANCED)
		break;
	}

	mutex_unlock(&spec_ctrl_mutex);
}

#undef pr_fmt
		pr_info("You may make it effective by booting the kernel with mem=%llu parameter.\n",
				half_pa);
		pr_info("However, doing so will make a part of your RAM unusable.\n");
		pr_info("Reading https://www.kernel.org/doc/html/latest/admin-guide/l1tf.html might help you decide.\n");
		return;
	}

	setup_force_cpu_cap(X86_FEATURE_L1TF_PTEINV);
early_param("l1tf", l1tf_cmdline);

#undef pr_fmt

#ifdef CONFIG_SYSFS

#define L1TF_DEFAULT_MSG "Mitigation: PTE Inversion"
}
#endif

static char *stibp_state(void)
{
	if (spectre_v2_enabled == SPECTRE_V2_IBRS_ENHANCED)
		return "";
		if (boot_cpu_has(X86_FEATURE_L1TF_PTEINV))
			return l1tf_show_state(buf);
		break;
	default:
		break;
	}

{
	return cpu_show_common(dev, attr, buf, X86_BUG_L1TF);
}
#endif
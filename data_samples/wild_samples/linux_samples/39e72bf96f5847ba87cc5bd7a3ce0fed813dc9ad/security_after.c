enum count_cache_flush_type {
	COUNT_CACHE_FLUSH_NONE	= 0x1,
	COUNT_CACHE_FLUSH_SW	= 0x2,
	COUNT_CACHE_FLUSH_HW	= 0x4,
};
static enum count_cache_flush_type count_cache_flush_type = COUNT_CACHE_FLUSH_NONE;
static bool link_stack_flush_enabled;

bool barrier_nospec_enabled;
static bool no_nospec;
static bool btb_flush_enabled;
#if defined(CONFIG_PPC_FSL_BOOK3E) || defined(CONFIG_PPC_BOOK3S_64)
static bool no_spectrev2;
#endif

static void enable_barrier_nospec(bool enable)
{
	barrier_nospec_enabled = enable;
	do_barrier_nospec_fixups(enable);
}
	if (bcs || ccd) {
		seq_buf_printf(&s, "Mitigation: ");

		if (bcs)
			seq_buf_printf(&s, "Indirect branch serialisation (kernel only)");

		if (bcs && ccd)
			seq_buf_printf(&s, ", ");

		if (ccd)
			seq_buf_printf(&s, "Indirect branch cache disabled");

		if (link_stack_flush_enabled)
			seq_buf_printf(&s, ", Software link stack flush");

	} else if (count_cache_flush_type != COUNT_CACHE_FLUSH_NONE) {
		seq_buf_printf(&s, "Mitigation: Software count cache flush");

		if (count_cache_flush_type == COUNT_CACHE_FLUSH_HW)
			seq_buf_printf(&s, " (hardware accelerated)");

		if (link_stack_flush_enabled)
			seq_buf_printf(&s, ", Software link stack flush");

	} else if (btb_flush_enabled) {
		seq_buf_printf(&s, "Mitigation: Branch predictor state flush");
	} else {
		seq_buf_printf(&s, "Vulnerable");
	}

	seq_buf_printf(&s, "\n");

	return s.len;
}

#ifdef CONFIG_PPC_BOOK3S_64
/*
 * Store-forwarding barrier support.
 */

static enum stf_barrier_type stf_enabled_flush_types;
static bool no_stf_barrier;
bool stf_barrier;

static int __init handle_no_stf_barrier(char *p)
{
	pr_info("stf-barrier: disabled on command line.");
	no_stf_barrier = true;
	return 0;
}

early_param("no_stf_barrier", handle_no_stf_barrier);

/* This is the generic flag used by other architectures */
static int __init handle_ssbd(char *p)
{
	if (!p || strncmp(p, "auto", 5) == 0 || strncmp(p, "on", 2) == 0 ) {
		/* Until firmware tells us, we have the barrier with auto */
		return 0;
	} else if (strncmp(p, "off", 3) == 0) {
		handle_no_stf_barrier(NULL);
		return 0;
	} else
		return 1;

	return 0;
}
early_param("spec_store_bypass_disable", handle_ssbd);

/* This is the generic flag used by other architectures */
static int __init handle_no_ssbd(char *p)
{
	handle_no_stf_barrier(NULL);
	return 0;
}
early_param("nospec_store_bypass_disable", handle_no_ssbd);

static void stf_barrier_enable(bool enable)
{
	if (enable)
		do_stf_barrier_fixups(stf_enabled_flush_types);
	else
		do_stf_barrier_fixups(STF_BARRIER_NONE);

	stf_barrier = enable;
}

void setup_stf_barrier(void)
{
	enum stf_barrier_type type;
	bool enable, hv;

	hv = cpu_has_feature(CPU_FTR_HVMODE);

	/* Default to fallback in case fw-features are not available */
	if (cpu_has_feature(CPU_FTR_ARCH_300))
		type = STF_BARRIER_EIEIO;
	else if (cpu_has_feature(CPU_FTR_ARCH_207S))
		type = STF_BARRIER_SYNC_ORI;
	else if (cpu_has_feature(CPU_FTR_ARCH_206))
		type = STF_BARRIER_FALLBACK;
	else
		type = STF_BARRIER_NONE;

	enable = security_ftr_enabled(SEC_FTR_FAVOUR_SECURITY) &&
		(security_ftr_enabled(SEC_FTR_L1D_FLUSH_PR) ||
		 (security_ftr_enabled(SEC_FTR_L1D_FLUSH_HV) && hv));

	if (type == STF_BARRIER_FALLBACK) {
		pr_info("stf-barrier: fallback barrier available\n");
	} else if (type == STF_BARRIER_SYNC_ORI) {
		pr_info("stf-barrier: hwsync barrier available\n");
	} else if (type == STF_BARRIER_EIEIO) {
		pr_info("stf-barrier: eieio barrier available\n");
	}

	stf_enabled_flush_types = type;

	if (!no_stf_barrier && !cpu_mitigations_off())
		stf_barrier_enable(enable);
}

ssize_t cpu_show_spec_store_bypass(struct device *dev, struct device_attribute *attr, char *buf)
{
	if (stf_barrier && stf_enabled_flush_types != STF_BARRIER_NONE) {
		const char *type;
		switch (stf_enabled_flush_types) {
		case STF_BARRIER_EIEIO:
			type = "eieio";
			break;
		case STF_BARRIER_SYNC_ORI:
			type = "hwsync";
			break;
		case STF_BARRIER_FALLBACK:
			type = "fallback";
			break;
		default:
			type = "unknown";
		}
		return sprintf(buf, "Mitigation: Kernel entry/exit barrier (%s)\n", type);
	}

	if (!security_ftr_enabled(SEC_FTR_L1D_FLUSH_HV) &&
	    !security_ftr_enabled(SEC_FTR_L1D_FLUSH_PR))
		return sprintf(buf, "Not affected\n");

	return sprintf(buf, "Vulnerable\n");
}

#ifdef CONFIG_DEBUG_FS
static int stf_barrier_set(void *data, u64 val)
{
	bool enable;

	if (val == 1)
		enable = true;
	else if (val == 0)
		enable = false;
	else
		return -EINVAL;

	/* Only do anything if we're changing state */
	if (enable != stf_barrier)
		stf_barrier_enable(enable);

	return 0;
}

static int stf_barrier_get(void *data, u64 *val)
{
	*val = stf_barrier ? 1 : 0;
	return 0;
}

DEFINE_SIMPLE_ATTRIBUTE(fops_stf_barrier, stf_barrier_get, stf_barrier_set, "%llu\n");

static __init int stf_barrier_debugfs_init(void)
{
	debugfs_create_file("stf_barrier", 0600, powerpc_debugfs_root, NULL, &fops_stf_barrier);
	return 0;
}
device_initcall(stf_barrier_debugfs_init);
#endif /* CONFIG_DEBUG_FS */

static void no_count_cache_flush(void)
{
	count_cache_flush_type = COUNT_CACHE_FLUSH_NONE;
	pr_info("count-cache-flush: software flush disabled.\n");
}

static void toggle_count_cache_flush(bool enable)
{
	if (!security_ftr_enabled(SEC_FTR_FLUSH_COUNT_CACHE) &&
	    !security_ftr_enabled(SEC_FTR_FLUSH_LINK_STACK))
		enable = false;

	if (!enable) {
		patch_instruction_site(&patch__call_flush_count_cache, PPC_INST_NOP);
		pr_info("link-stack-flush: software flush disabled.\n");
		link_stack_flush_enabled = false;
		no_count_cache_flush();
		return;
	}

	// This enables the branch from _switch to flush_count_cache
	patch_branch_site(&patch__call_flush_count_cache,
			  (u64)&flush_count_cache, BRANCH_SET_LINK);

	pr_info("link-stack-flush: software flush enabled.\n");
	link_stack_flush_enabled = true;

	// If we just need to flush the link stack, patch an early return
	if (!security_ftr_enabled(SEC_FTR_FLUSH_COUNT_CACHE)) {
		patch_instruction_site(&patch__flush_link_stack_return, PPC_INST_BLR);
		no_count_cache_flush();
		return;
	}

	if (!security_ftr_enabled(SEC_FTR_BCCTR_FLUSH_ASSIST)) {
		count_cache_flush_type = COUNT_CACHE_FLUSH_SW;
		pr_info("count-cache-flush: full software flush sequence enabled.\n");
		return;
	}

	patch_instruction_site(&patch__flush_count_cache_return, PPC_INST_BLR);
	count_cache_flush_type = COUNT_CACHE_FLUSH_HW;
	pr_info("count-cache-flush: hardware assisted flush sequence enabled\n");
}

void setup_count_cache_flush(void)
{
	bool enable = true;

	if (no_spectrev2 || cpu_mitigations_off()) {
		if (security_ftr_enabled(SEC_FTR_BCCTRL_SERIALISED) ||
		    security_ftr_enabled(SEC_FTR_COUNT_CACHE_DISABLED))
			pr_warn("Spectre v2 mitigations not fully under software control, can't disable\n");

		enable = false;
	}

	/*
	 * There's no firmware feature flag/hypervisor bit to tell us we need to
	 * flush the link stack on context switch. So we set it here if we see
	 * either of the Spectre v2 mitigations that aim to protect userspace.
	 */
	if (security_ftr_enabled(SEC_FTR_COUNT_CACHE_DISABLED) ||
	    security_ftr_enabled(SEC_FTR_FLUSH_COUNT_CACHE))
		security_ftr_set(SEC_FTR_FLUSH_LINK_STACK);

	toggle_count_cache_flush(enable);
}

#ifdef CONFIG_DEBUG_FS
static int count_cache_flush_set(void *data, u64 val)
{
	bool enable;

	if (val == 1)
		enable = true;
	else if (val == 0)
		enable = false;
	else
		return -EINVAL;

	toggle_count_cache_flush(enable);

	return 0;
}

static int count_cache_flush_get(void *data, u64 *val)
{
	if (count_cache_flush_type == COUNT_CACHE_FLUSH_NONE)
		*val = 0;
	else
		*val = 1;

	return 0;
}

DEFINE_SIMPLE_ATTRIBUTE(fops_count_cache_flush, count_cache_flush_get,
			count_cache_flush_set, "%llu\n");

static __init int count_cache_flush_debugfs_init(void)
{
	debugfs_create_file("count_cache_flush", 0600, powerpc_debugfs_root,
			    NULL, &fops_count_cache_flush);
	return 0;
}
device_initcall(count_cache_flush_debugfs_init);
#endif /* CONFIG_DEBUG_FS */
#endif /* CONFIG_PPC_BOOK3S_64 */
{
	debugfs_create_file("stf_barrier", 0600, powerpc_debugfs_root, NULL, &fops_stf_barrier);
	return 0;
}
device_initcall(stf_barrier_debugfs_init);
#endif /* CONFIG_DEBUG_FS */

static void no_count_cache_flush(void)
{
	count_cache_flush_type = COUNT_CACHE_FLUSH_NONE;
	pr_info("count-cache-flush: software flush disabled.\n");
}
	if (no_spectrev2 || cpu_mitigations_off()) {
		if (security_ftr_enabled(SEC_FTR_BCCTRL_SERIALISED) ||
		    security_ftr_enabled(SEC_FTR_COUNT_CACHE_DISABLED))
			pr_warn("Spectre v2 mitigations not fully under software control, can't disable\n");

		enable = false;
	}

	/*
	 * There's no firmware feature flag/hypervisor bit to tell us we need to
	 * flush the link stack on context switch. So we set it here if we see
	 * either of the Spectre v2 mitigations that aim to protect userspace.
	 */
	if (security_ftr_enabled(SEC_FTR_COUNT_CACHE_DISABLED) ||
	    security_ftr_enabled(SEC_FTR_FLUSH_COUNT_CACHE))
		security_ftr_set(SEC_FTR_FLUSH_LINK_STACK);

	toggle_count_cache_flush(enable);
}

#ifdef CONFIG_DEBUG_FS
static int count_cache_flush_set(void *data, u64 val)
{
	bool enable;

	if (val == 1)
		enable = true;
	else if (val == 0)
		enable = false;
	else
		return -EINVAL;

	toggle_count_cache_flush(enable);

	return 0;
}

static int count_cache_flush_get(void *data, u64 *val)
{
	if (count_cache_flush_type == COUNT_CACHE_FLUSH_NONE)
		*val = 0;
	else
		*val = 1;

	return 0;
}

DEFINE_SIMPLE_ATTRIBUTE(fops_count_cache_flush, count_cache_flush_get,
			count_cache_flush_set, "%llu\n");

static __init int count_cache_flush_debugfs_init(void)
{
	debugfs_create_file("count_cache_flush", 0600, powerpc_debugfs_root,
			    NULL, &fops_count_cache_flush);
	return 0;
}
device_initcall(count_cache_flush_debugfs_init);
#endif /* CONFIG_DEBUG_FS */
#endif /* CONFIG_PPC_BOOK3S_64 */
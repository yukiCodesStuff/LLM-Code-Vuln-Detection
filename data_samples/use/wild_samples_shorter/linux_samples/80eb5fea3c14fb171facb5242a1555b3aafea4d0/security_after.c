	COUNT_CACHE_FLUSH_HW	= 0x4,
};
static enum count_cache_flush_type count_cache_flush_type = COUNT_CACHE_FLUSH_NONE;
static bool link_stack_flush_enabled;

bool barrier_nospec_enabled;
static bool no_nospec;
static bool btb_flush_enabled;

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
#ifdef CONFIG_KVM_BOOK3S_HV_POSSIBLE
		patch_instruction_site(&patch__call_kvm_flush_link_stack, PPC_INST_NOP);
#endif
		pr_info("link-stack-flush: software flush disabled.\n");
		link_stack_flush_enabled = false;
		no_count_cache_flush();
		return;
	}

	// This enables the branch from _switch to flush_count_cache
	patch_branch_site(&patch__call_flush_count_cache,
			  (u64)&flush_count_cache, BRANCH_SET_LINK);

#ifdef CONFIG_KVM_BOOK3S_HV_POSSIBLE
	// This enables the branch from guest_exit_cont to kvm_flush_link_stack
	patch_branch_site(&patch__call_kvm_flush_link_stack,
			  (u64)&kvm_flush_link_stack, BRANCH_SET_LINK);
#endif

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
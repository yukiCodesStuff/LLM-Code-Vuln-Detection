	COUNT_CACHE_FLUSH_HW	= 0x4,
};
static enum count_cache_flush_type count_cache_flush_type = COUNT_CACHE_FLUSH_NONE;

bool barrier_nospec_enabled;
static bool no_nospec;
static bool btb_flush_enabled;

		if (ccd)
			seq_buf_printf(&s, "Indirect branch cache disabled");
	} else if (count_cache_flush_type != COUNT_CACHE_FLUSH_NONE) {
		seq_buf_printf(&s, "Mitigation: Software count cache flush");

		if (count_cache_flush_type == COUNT_CACHE_FLUSH_HW)
			seq_buf_printf(&s, " (hardware accelerated)");
	} else if (btb_flush_enabled) {
		seq_buf_printf(&s, "Mitigation: Branch predictor state flush");
	} else {
		seq_buf_printf(&s, "Vulnerable");
device_initcall(stf_barrier_debugfs_init);
#endif /* CONFIG_DEBUG_FS */

static void toggle_count_cache_flush(bool enable)
{
	if (!enable || !security_ftr_enabled(SEC_FTR_FLUSH_COUNT_CACHE)) {
		patch_instruction_site(&patch__call_flush_count_cache, PPC_INST_NOP);
		count_cache_flush_type = COUNT_CACHE_FLUSH_NONE;
		pr_info("count-cache-flush: software flush disabled.\n");
		return;
	}

	patch_branch_site(&patch__call_flush_count_cache,
			  (u64)&flush_count_cache, BRANCH_SET_LINK);

	if (!security_ftr_enabled(SEC_FTR_BCCTR_FLUSH_ASSIST)) {
		count_cache_flush_type = COUNT_CACHE_FLUSH_SW;
		pr_info("count-cache-flush: full software flush sequence enabled.\n");
		return;
	if (no_spectrev2 || cpu_mitigations_off()) {
		if (security_ftr_enabled(SEC_FTR_BCCTRL_SERIALISED) ||
		    security_ftr_enabled(SEC_FTR_COUNT_CACHE_DISABLED))
			pr_warn("Spectre v2 mitigations not under software control, can't disable\n");

		enable = false;
	}

	toggle_count_cache_flush(enable);
}

#ifdef CONFIG_DEBUG_FS
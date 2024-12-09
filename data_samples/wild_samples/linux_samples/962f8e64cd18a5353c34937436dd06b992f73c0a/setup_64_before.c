{
#ifdef CONFIG_HARDLOCKUP_DETECTOR_PERF
	hardlockup_detector_disable();
#else
	if (firmware_has_feature(FW_FEATURE_LPAR))
		hardlockup_detector_disable();
#endif

	return 0;
}
early_initcall(disable_hardlockup_detector);

#ifdef CONFIG_PPC_BOOK3S_64
static enum l1d_flush_type enabled_flush_types;
static void *l1d_flush_fallback_area;
static bool no_rfi_flush;
bool rfi_flush;

static int __init handle_no_rfi_flush(char *p)
{
	pr_info("rfi-flush: disabled on command line.");
	no_rfi_flush = true;
	return 0;
}
{
	pr_info("rfi-flush: disabled on command line.");
	no_rfi_flush = true;
	return 0;
}
early_param("no_rfi_flush", handle_no_rfi_flush);

/*
 * The RFI flush is not KPTI, but because users will see doco that says to use
 * nopti we hijack that option here to also disable the RFI flush.
 */
static int __init handle_no_pti(char *p)
{
	pr_info("rfi-flush: disabling due to 'nopti' on command line.\n");
	handle_no_rfi_flush(NULL);
	return 0;
}
	if (enable) {
		do_rfi_flush_fixups(enabled_flush_types);
		on_each_cpu(do_nothing, NULL, 1);
	} else
		do_rfi_flush_fixups(L1D_FLUSH_NONE);

	rfi_flush = enable;
}

static void __ref init_fallback_flush(void)
{
	u64 l1d_size, limit;
	int cpu;

	/* Only allocate the fallback flush area once (at boot time). */
	if (l1d_flush_fallback_area)
		return;

	l1d_size = ppc64_caches.l1d.size;

	/*
	 * If there is no d-cache-size property in the device tree, l1d_size
	 * could be zero. That leads to the loop in the asm wrapping around to
	 * 2^64-1, and then walking off the end of the fallback area and
	 * eventually causing a page fault which is fatal. Just default to
	 * something vaguely sane.
	 */
	if (!l1d_size)
		l1d_size = (64 * 1024);

	limit = min(ppc64_bolted_size(), ppc64_rma_size);

	/*
	 * Align to L1d size, and size it at 2x L1d size, to catch possible
	 * hardware prefetch runoff. We don't have a recipe for load patterns to
	 * reliably avoid the prefetcher.
	 */
	l1d_flush_fallback_area = memblock_alloc_try_nid(l1d_size * 2,
						l1d_size, MEMBLOCK_LOW_LIMIT,
						limit, NUMA_NO_NODE);
	if (!l1d_flush_fallback_area)
		panic("%s: Failed to allocate %llu bytes align=0x%llx max_addr=%pa\n",
		      __func__, l1d_size * 2, l1d_size, &limit);


	for_each_possible_cpu(cpu) {
		struct paca_struct *paca = paca_ptrs[cpu];
		paca->rfi_flush_fallback_area = l1d_flush_fallback_area;
		paca->l1d_flush_size = l1d_size;
	}
	if (types & L1D_FLUSH_FALLBACK) {
		pr_info("rfi-flush: fallback displacement flush available\n");
		init_fallback_flush();
	}

	if (types & L1D_FLUSH_ORI)
		pr_info("rfi-flush: ori type flush available\n");

	if (types & L1D_FLUSH_MTTRIG)
		pr_info("rfi-flush: mttrig type flush available\n");

	enabled_flush_types = types;

	if (!no_rfi_flush && !cpu_mitigations_off())
		rfi_flush_enable(enable);
}

#ifdef CONFIG_DEBUG_FS
static int rfi_flush_set(void *data, u64 val)
{
	bool enable;

	if (val == 1)
		enable = true;
	else if (val == 0)
		enable = false;
	else
		return -EINVAL;

	/* Only do anything if we're changing state */
	if (enable != rfi_flush)
		rfi_flush_enable(enable);

	return 0;
}

static int rfi_flush_get(void *data, u64 *val)
{
	*val = rfi_flush ? 1 : 0;
	return 0;
}

DEFINE_SIMPLE_ATTRIBUTE(fops_rfi_flush, rfi_flush_get, rfi_flush_set, "%llu\n");

static __init int rfi_flush_debugfs_init(void)
{
	debugfs_create_file("rfi_flush", 0600, powerpc_debugfs_root, NULL, &fops_rfi_flush);
	return 0;
}
device_initcall(rfi_flush_debugfs_init);
#endif
#endif /* CONFIG_PPC_BOOK3S_64 */
{
	*val = rfi_flush ? 1 : 0;
	return 0;
}

DEFINE_SIMPLE_ATTRIBUTE(fops_rfi_flush, rfi_flush_get, rfi_flush_set, "%llu\n");

static __init int rfi_flush_debugfs_init(void)
{
	debugfs_create_file("rfi_flush", 0600, powerpc_debugfs_root, NULL, &fops_rfi_flush);
	return 0;
}
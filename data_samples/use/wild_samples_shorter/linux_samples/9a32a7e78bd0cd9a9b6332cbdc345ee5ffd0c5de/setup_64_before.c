static void *l1d_flush_fallback_area;
static bool no_rfi_flush;
static bool no_entry_flush;
bool rfi_flush;
bool entry_flush;

static int __init handle_no_rfi_flush(char *p)
{
	pr_info("rfi-flush: disabled on command line.");
}
early_param("no_entry_flush", handle_no_entry_flush);

/*
 * The RFI flush is not KPTI, but because users will see doco that says to use
 * nopti we hijack that option here to also disable the RFI flush.
 */
	entry_flush = enable;
}

static void __ref init_fallback_flush(void)
{
	u64 l1d_size, limit;
	int cpu;
		entry_flush_enable(enable);
}

#ifdef CONFIG_DEBUG_FS
static int rfi_flush_set(void *data, u64 val)
{
	bool enable;

DEFINE_SIMPLE_ATTRIBUTE(fops_entry_flush, entry_flush_get, entry_flush_set, "%llu\n");

static __init int rfi_flush_debugfs_init(void)
{
	debugfs_create_file("rfi_flush", 0600, powerpc_debugfs_root, NULL, &fops_rfi_flush);
	debugfs_create_file("entry_flush", 0600, powerpc_debugfs_root, NULL, &fops_entry_flush);
	return 0;
}
device_initcall(rfi_flush_debugfs_init);
#endif
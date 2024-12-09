const char *const lockdown_reasons[LOCKDOWN_CONFIDENTIALITY_MAX+1] = {
	[LOCKDOWN_NONE] = "none",
	[LOCKDOWN_MODULE_SIGNATURE] = "unsigned module loading",
	[LOCKDOWN_DEV_MEM] = "/dev/mem,kmem,port",
	[LOCKDOWN_EFI_TEST] = "/dev/efi_test access",
	[LOCKDOWN_KEXEC] = "kexec of unsigned images",
	[LOCKDOWN_HIBERNATION] = "hibernation",
	[LOCKDOWN_PCI_ACCESS] = "direct PCI access",
	[LOCKDOWN_IOPORT] = "raw io port access",
	[LOCKDOWN_MSR] = "raw MSR access",
	[LOCKDOWN_ACPI_TABLES] = "modifying ACPI tables",
	[LOCKDOWN_PCMCIA_CIS] = "direct PCMCIA CIS storage",
	[LOCKDOWN_TIOCSSERIAL] = "reconfiguration of serial port IO",
	[LOCKDOWN_MODULE_PARAMETERS] = "unsafe module parameters",
	[LOCKDOWN_MMIOTRACE] = "unsafe mmio",
	[LOCKDOWN_DEBUGFS] = "debugfs access",
	[LOCKDOWN_XMON_WR] = "xmon write access",
	[LOCKDOWN_BPF_WRITE_USER] = "use of bpf to write user RAM",
	[LOCKDOWN_DBG_WRITE_KERNEL] = "use of kgdb/kdb to write kernel RAM",
	[LOCKDOWN_INTEGRITY_MAX] = "integrity",
	[LOCKDOWN_KCORE] = "/proc/kcore access",
	[LOCKDOWN_KPROBES] = "use of kprobes",
	[LOCKDOWN_BPF_READ_KERNEL] = "use of bpf to read kernel RAM",
	[LOCKDOWN_DBG_READ_KERNEL] = "use of kgdb/kdb to read kernel RAM",
	[LOCKDOWN_PERF] = "unsafe use of perf",
	[LOCKDOWN_TRACEFS] = "use of tracefs",
	[LOCKDOWN_XMON_RW] = "xmon read and write access",
	[LOCKDOWN_XFRM_SECRET] = "xfrm SA secret",
	[LOCKDOWN_CONFIDENTIALITY_MAX] = "confidentiality",
};

struct security_hook_heads security_hook_heads __lsm_ro_after_init;
static BLOCKING_NOTIFIER_HEAD(blocking_lsm_notifier_chain);

static struct kmem_cache *lsm_file_cache;
static struct kmem_cache *lsm_inode_cache;

char *lsm_names;
static struct lsm_blob_sizes blob_sizes __lsm_ro_after_init;

/* Boot-time LSM user choice */
static __initdata const char *chosen_lsm_order;
static __initdata const char *chosen_major_lsm;

static __initconst const char * const builtin_lsm_order = CONFIG_LSM;

/* Ordered list of LSMs to initialize. */
static __initdata struct lsm_info **ordered_lsms;
static __initdata struct lsm_info *exclusive;

static __initdata bool debug;
#define init_debug(...)						\
	do {							\
		if (debug)					\
			pr_info(__VA_ARGS__);			\
	} while (0)

static bool __init is_enabled(struct lsm_info *lsm)
{
	if (!lsm->enabled)
		return false;

	return *lsm->enabled;
}
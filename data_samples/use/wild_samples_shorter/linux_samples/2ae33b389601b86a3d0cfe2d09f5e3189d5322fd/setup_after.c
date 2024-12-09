
#ifdef CONFIG_X86_32
/* cpu data as detected by the assembly code in head.S */
struct cpuinfo_x86 new_cpu_data __cpuinitdata = {
	.wp_works_ok = -1,
	.fdiv_bug = -1,
};
/* common cpu data for all cpus */
struct cpuinfo_x86 boot_cpu_data __read_mostly = {
	.wp_works_ok = -1,
	.fdiv_bug = -1,
};
EXPORT_SYMBOL(boot_cpu_data);

unsigned int def_to_bigsmp;

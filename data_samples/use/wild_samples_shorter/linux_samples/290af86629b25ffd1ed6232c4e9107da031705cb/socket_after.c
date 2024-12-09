
core_initcall(sock_init);	/* early initcall */

static int __init jit_init(void)
{
#ifdef CONFIG_BPF_JIT_ALWAYS_ON
	bpf_jit_enable = 1;
#endif
	return 0;
}
pure_initcall(jit_init);

#ifdef CONFIG_PROC_FS
void socket_seq_show(struct seq_file *seq)
{
	int cpu;
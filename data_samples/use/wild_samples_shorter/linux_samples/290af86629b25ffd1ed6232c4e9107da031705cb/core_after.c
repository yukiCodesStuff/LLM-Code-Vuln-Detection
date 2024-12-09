}
EXPORT_SYMBOL_GPL(__bpf_call_base);

#ifndef CONFIG_BPF_JIT_ALWAYS_ON
/**
 *	__bpf_prog_run - run eBPF program on a given context
 *	@ctx: is the data we are operating on
 *	@insn: is the array of eBPF instructions
EVAL4(PROG_NAME_LIST, 416, 448, 480, 512)
};

#else
static unsigned int __bpf_prog_ret0(const void *ctx,
				    const struct bpf_insn *insn)
{
	return 0;
}
#endif

bool bpf_prog_array_compatible(struct bpf_array *array,
			       const struct bpf_prog *fp)
{
	if (!array->owner_prog_type) {
 */
struct bpf_prog *bpf_prog_select_runtime(struct bpf_prog *fp, int *err)
{
#ifndef CONFIG_BPF_JIT_ALWAYS_ON
	u32 stack_depth = max_t(u32, fp->aux->stack_depth, 1);

	fp->bpf_func = interpreters[(round_up(stack_depth, 32) / 32) - 1];
#else
	fp->bpf_func = __bpf_prog_ret0;
#endif

	/* eBPF JITs can rewrite the program in case constant
	 * blinding is active. However, in case of error during
	 * blinding, bpf_int_jit_compile() must always return a
	 */
	if (!bpf_prog_is_dev_bound(fp->aux)) {
		fp = bpf_int_jit_compile(fp);
#ifdef CONFIG_BPF_JIT_ALWAYS_ON
		if (!fp->jited) {
			*err = -ENOTSUPP;
			return fp;
		}
#endif
	} else {
		*err = bpf_prog_offload_compile(fp);
		if (*err)
			return fp;
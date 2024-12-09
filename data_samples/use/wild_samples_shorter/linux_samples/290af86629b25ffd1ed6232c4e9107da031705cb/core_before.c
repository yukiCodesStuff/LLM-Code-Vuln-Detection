}
EXPORT_SYMBOL_GPL(__bpf_call_base);

/**
 *	__bpf_prog_run - run eBPF program on a given context
 *	@ctx: is the data we are operating on
 *	@insn: is the array of eBPF instructions
EVAL4(PROG_NAME_LIST, 416, 448, 480, 512)
};

bool bpf_prog_array_compatible(struct bpf_array *array,
			       const struct bpf_prog *fp)
{
	if (!array->owner_prog_type) {
 */
struct bpf_prog *bpf_prog_select_runtime(struct bpf_prog *fp, int *err)
{
	u32 stack_depth = max_t(u32, fp->aux->stack_depth, 1);

	fp->bpf_func = interpreters[(round_up(stack_depth, 32) / 32) - 1];

	/* eBPF JITs can rewrite the program in case constant
	 * blinding is active. However, in case of error during
	 * blinding, bpf_int_jit_compile() must always return a
	 */
	if (!bpf_prog_is_dev_bound(fp->aux)) {
		fp = bpf_int_jit_compile(fp);
	} else {
		*err = bpf_prog_offload_compile(fp);
		if (*err)
			return fp;
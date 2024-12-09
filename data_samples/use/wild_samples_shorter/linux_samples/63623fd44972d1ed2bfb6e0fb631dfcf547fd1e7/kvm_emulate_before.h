#define X86EMUL_SMM_MASK             (1 << 6)
#define X86EMUL_SMM_INSIDE_NMI_MASK  (1 << 7)

struct x86_emulate_ctxt {
	const struct x86_emulate_ops *ops;

	/* Register state before/after emulation. */
	struct operand src;
	struct operand src2;
	struct operand dst;
	int (*execute)(struct x86_emulate_ctxt *ctxt);
	int (*check_perm)(struct x86_emulate_ctxt *ctxt);
	/*
	 * The following six fields are cleared together,
	 * the rest are initialized unconditionally in x86_decode_insn
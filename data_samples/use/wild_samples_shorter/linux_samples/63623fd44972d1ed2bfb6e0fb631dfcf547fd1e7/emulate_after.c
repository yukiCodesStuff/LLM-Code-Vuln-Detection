#define NR_FASTOP (ilog2(sizeof(ulong)) + 1)
#define FASTOP_SIZE 8

struct opcode {
	u64 flags : 56;
	u64 intercept : 8;
	union {
#define ON64(x)
#endif

/*
 * fastop functions have a special calling convention:
 *
 * dst:    rax        (in/out)
 * src:    rdx        (in/out)
 * src2:   rcx        (in)
 * flags:  rflags     (in/out)
 * ex:     rsi        (in:fastop pointer, out:zero if exception)
 *
 * Moreover, they are all exactly FASTOP_SIZE bytes long, so functions for
 * different operand sizes can be reached by calculation, rather than a jump
 * table (which would be bigger than the code).
 */
static int fastop(struct x86_emulate_ctxt *ctxt, fastop_t fop);

#define __FOP_FUNC(name) \
	".align " __stringify(FASTOP_SIZE) " \n\t" \

	if (ctxt->execute) {
		if (ctxt->d & Fastop)
			rc = fastop(ctxt, ctxt->fop);
		else
			rc = ctxt->execute(ctxt);
		if (rc != X86EMUL_CONTINUE)
			goto done;
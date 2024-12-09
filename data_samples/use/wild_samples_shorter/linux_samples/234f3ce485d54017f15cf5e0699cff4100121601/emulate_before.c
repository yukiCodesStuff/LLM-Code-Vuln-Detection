	return emulate_exception(ctxt, NM_VECTOR, 0, false);
}

static inline void assign_eip_near(struct x86_emulate_ctxt *ctxt, ulong dst)
{
	switch (ctxt->op_bytes) {
	case 2:
		ctxt->_eip = (u16)dst;
		ctxt->_eip = (u32)dst;
		break;
	case 8:
		ctxt->_eip = dst;
		break;
	default:
		WARN(1, "unsupported eip assignment size\n");
	}
}

static inline void jmp_rel(struct x86_emulate_ctxt *ctxt, int rel)
{
	assign_eip_near(ctxt, ctxt->_eip + rel);
}

static u16 get_segment_selector(struct x86_emulate_ctxt *ctxt, unsigned seg)
{
	case 2: /* call near abs */ {
		long int old_eip;
		old_eip = ctxt->_eip;
		ctxt->_eip = ctxt->src.val;
		ctxt->src.val = old_eip;
		rc = em_push(ctxt);
		break;
	}
	case 4: /* jmp abs */
		ctxt->_eip = ctxt->src.val;
		break;
	case 5: /* jmp far */
		rc = em_jmp_far(ctxt);
		break;

static int em_ret(struct x86_emulate_ctxt *ctxt)
{
	ctxt->dst.type = OP_REG;
	ctxt->dst.addr.reg = &ctxt->_eip;
	ctxt->dst.bytes = ctxt->op_bytes;
	return em_pop(ctxt);
}

static int em_ret_far(struct x86_emulate_ctxt *ctxt)
{
{
	const struct x86_emulate_ops *ops = ctxt->ops;
	struct desc_struct cs, ss;
	u64 msr_data;
	int usermode;
	u16 cs_sel = 0, ss_sel = 0;

	/* inject #GP if in real mode or Virtual 8086 mode */
	else
		usermode = X86EMUL_MODE_PROT32;

	cs.dpl = 3;
	ss.dpl = 3;
	ops->get_msr(ctxt, MSR_IA32_SYSENTER_CS, &msr_data);
	switch (usermode) {
		ss_sel = cs_sel + 8;
		cs.d = 0;
		cs.l = 1;
		break;
	}
	cs_sel |= SELECTOR_RPL_MASK;
	ss_sel |= SELECTOR_RPL_MASK;
	ops->set_segment(ctxt, cs_sel, &cs, 0, VCPU_SREG_CS);
	ops->set_segment(ctxt, ss_sel, &ss, 0, VCPU_SREG_SS);

	ctxt->_eip = reg_read(ctxt, VCPU_REGS_RDX);
	*reg_write(ctxt, VCPU_REGS_RSP) = reg_read(ctxt, VCPU_REGS_RCX);

	return X86EMUL_CONTINUE;
}


static int em_call(struct x86_emulate_ctxt *ctxt)
{
	long rel = ctxt->src.val;

	ctxt->src.val = (unsigned long)ctxt->_eip;
	jmp_rel(ctxt, rel);
	return em_push(ctxt);
}

static int em_call_far(struct x86_emulate_ctxt *ctxt)
static int em_ret_near_imm(struct x86_emulate_ctxt *ctxt)
{
	int rc;

	ctxt->dst.type = OP_REG;
	ctxt->dst.addr.reg = &ctxt->_eip;
	ctxt->dst.bytes = ctxt->op_bytes;
	rc = emulate_pop(ctxt, &ctxt->dst.val, ctxt->op_bytes);
	if (rc != X86EMUL_CONTINUE)
		return rc;
	rsp_increment(ctxt, ctxt->src.val);
	return X86EMUL_CONTINUE;

static int em_loop(struct x86_emulate_ctxt *ctxt)
{
	register_address_increment(ctxt, reg_rmw(ctxt, VCPU_REGS_RCX), -1);
	if ((address_mask(ctxt, reg_read(ctxt, VCPU_REGS_RCX)) != 0) &&
	    (ctxt->b == 0xe2 || test_cc(ctxt->b ^ 0x5, ctxt->eflags)))
		jmp_rel(ctxt, ctxt->src.val);

	return X86EMUL_CONTINUE;
}

static int em_jcxz(struct x86_emulate_ctxt *ctxt)
{
	if (address_mask(ctxt, reg_read(ctxt, VCPU_REGS_RCX)) == 0)
		jmp_rel(ctxt, ctxt->src.val);

	return X86EMUL_CONTINUE;
}

static int em_in(struct x86_emulate_ctxt *ctxt)
{
		break;
	case 0x70 ... 0x7f: /* jcc (short) */
		if (test_cc(ctxt->b, ctxt->eflags))
			jmp_rel(ctxt, ctxt->src.val);
		break;
	case 0x8d: /* lea r16/r32, m */
		ctxt->dst.val = ctxt->src.addr.mem.ea;
		break;
		break;
	case 0xe9: /* jmp rel */
	case 0xeb: /* jmp rel short */
		jmp_rel(ctxt, ctxt->src.val);
		ctxt->dst.type = OP_NONE; /* Disable writeback. */
		break;
	case 0xf4:              /* hlt */
		ctxt->ops->halt(ctxt);
		break;
	case 0x80 ... 0x8f: /* jnz rel, etc*/
		if (test_cc(ctxt->b, ctxt->eflags))
			jmp_rel(ctxt, ctxt->src.val);
		break;
	case 0x90 ... 0x9f:     /* setcc r/m8 */
		ctxt->dst.val = test_cc(ctxt->b, ctxt->eflags);
		break;
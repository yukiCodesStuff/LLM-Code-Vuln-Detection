
/* Does not support long mode */
static int __load_segment_descriptor(struct x86_emulate_ctxt *ctxt,
				     u16 selector, int seg, u8 cpl, bool in_task_switch)
{
	struct desc_struct seg_desc, old_desc;
	u8 dpl, rpl;
	unsigned err_vec = GP_VECTOR;
	}
load:
	ctxt->ops->set_segment(ctxt, selector, &seg_desc, base3, seg);
	return X86EMUL_CONTINUE;
exception:
	return emulate_exception(ctxt, err_vec, err_code, true);
}
				   u16 selector, int seg)
{
	u8 cpl = ctxt->ops->cpl(ctxt);
	return __load_segment_descriptor(ctxt, selector, seg, cpl, false);
}

static void write_register_operand(struct operand *op)
{
static int em_jmp_far(struct x86_emulate_ctxt *ctxt)
{
	int rc;
	unsigned short sel;

	memcpy(&sel, ctxt->src.valptr + ctxt->op_bytes, 2);

	rc = load_segment_descriptor(ctxt, sel, VCPU_SREG_CS);
	if (rc != X86EMUL_CONTINUE)
		return rc;

	ctxt->_eip = 0;
	memcpy(&ctxt->_eip, ctxt->src.valptr, ctxt->op_bytes);
	return X86EMUL_CONTINUE;
}

static int em_grp45(struct x86_emulate_ctxt *ctxt)
{
static int em_ret_far(struct x86_emulate_ctxt *ctxt)
{
	int rc;
	unsigned long cs;
	int cpl = ctxt->ops->cpl(ctxt);

	rc = emulate_pop(ctxt, &ctxt->_eip, ctxt->op_bytes);
	if (rc != X86EMUL_CONTINUE)
		return rc;
	if (ctxt->op_bytes == 4)
		ctxt->_eip = (u32)ctxt->_eip;
	rc = emulate_pop(ctxt, &cs, ctxt->op_bytes);
	if (rc != X86EMUL_CONTINUE)
		return rc;
	/* Outer-privilege level return is not implemented */
	if (ctxt->mode >= X86EMUL_MODE_PROT16 && (cs & 3) > cpl)
		return X86EMUL_UNHANDLEABLE;
	rc = load_segment_descriptor(ctxt, (u16)cs, VCPU_SREG_CS);
	return rc;
}

static int em_ret_far_imm(struct x86_emulate_ctxt *ctxt)
	 * Now load segment descriptors. If fault happens at this stage
	 * it is handled in a context of new task
	 */
	ret = __load_segment_descriptor(ctxt, tss->ldt, VCPU_SREG_LDTR, cpl, true);
	if (ret != X86EMUL_CONTINUE)
		return ret;
	ret = __load_segment_descriptor(ctxt, tss->es, VCPU_SREG_ES, cpl, true);
	if (ret != X86EMUL_CONTINUE)
		return ret;
	ret = __load_segment_descriptor(ctxt, tss->cs, VCPU_SREG_CS, cpl, true);
	if (ret != X86EMUL_CONTINUE)
		return ret;
	ret = __load_segment_descriptor(ctxt, tss->ss, VCPU_SREG_SS, cpl, true);
	if (ret != X86EMUL_CONTINUE)
		return ret;
	ret = __load_segment_descriptor(ctxt, tss->ds, VCPU_SREG_DS, cpl, true);
	if (ret != X86EMUL_CONTINUE)
		return ret;

	return X86EMUL_CONTINUE;
	 * Now load segment descriptors. If fault happenes at this stage
	 * it is handled in a context of new task
	 */
	ret = __load_segment_descriptor(ctxt, tss->ldt_selector, VCPU_SREG_LDTR, cpl, true);
	if (ret != X86EMUL_CONTINUE)
		return ret;
	ret = __load_segment_descriptor(ctxt, tss->es, VCPU_SREG_ES, cpl, true);
	if (ret != X86EMUL_CONTINUE)
		return ret;
	ret = __load_segment_descriptor(ctxt, tss->cs, VCPU_SREG_CS, cpl, true);
	if (ret != X86EMUL_CONTINUE)
		return ret;
	ret = __load_segment_descriptor(ctxt, tss->ss, VCPU_SREG_SS, cpl, true);
	if (ret != X86EMUL_CONTINUE)
		return ret;
	ret = __load_segment_descriptor(ctxt, tss->ds, VCPU_SREG_DS, cpl, true);
	if (ret != X86EMUL_CONTINUE)
		return ret;
	ret = __load_segment_descriptor(ctxt, tss->fs, VCPU_SREG_FS, cpl, true);
	if (ret != X86EMUL_CONTINUE)
		return ret;
	ret = __load_segment_descriptor(ctxt, tss->gs, VCPU_SREG_GS, cpl, true);
	if (ret != X86EMUL_CONTINUE)
		return ret;

	return X86EMUL_CONTINUE;
	u16 sel, old_cs;
	ulong old_eip;
	int rc;

	old_cs = get_segment_selector(ctxt, VCPU_SREG_CS);
	old_eip = ctxt->_eip;

	memcpy(&sel, ctxt->src.valptr + ctxt->op_bytes, 2);
	if (load_segment_descriptor(ctxt, sel, VCPU_SREG_CS))
		return X86EMUL_CONTINUE;

	ctxt->_eip = 0;
	memcpy(&ctxt->_eip, ctxt->src.valptr, ctxt->op_bytes);

	ctxt->src.val = old_cs;
	rc = em_push(ctxt);
	if (rc != X86EMUL_CONTINUE)
		return rc;

	ctxt->src.val = old_eip;
	return em_push(ctxt);
}

static int em_ret_near_imm(struct x86_emulate_ctxt *ctxt)
{
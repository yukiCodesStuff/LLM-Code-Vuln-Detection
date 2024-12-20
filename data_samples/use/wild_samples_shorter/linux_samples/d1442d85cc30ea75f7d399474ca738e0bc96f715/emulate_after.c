
/* Does not support long mode */
static int __load_segment_descriptor(struct x86_emulate_ctxt *ctxt,
				     u16 selector, int seg, u8 cpl,
				     bool in_task_switch,
				     struct desc_struct *desc)
{
	struct desc_struct seg_desc, old_desc;
	u8 dpl, rpl;
	unsigned err_vec = GP_VECTOR;
	}
load:
	ctxt->ops->set_segment(ctxt, selector, &seg_desc, base3, seg);
	if (desc)
		*desc = seg_desc;
	return X86EMUL_CONTINUE;
exception:
	return emulate_exception(ctxt, err_vec, err_code, true);
}
				   u16 selector, int seg)
{
	u8 cpl = ctxt->ops->cpl(ctxt);
	return __load_segment_descriptor(ctxt, selector, seg, cpl, false, NULL);
}

static void write_register_operand(struct operand *op)
{
static int em_jmp_far(struct x86_emulate_ctxt *ctxt)
{
	int rc;
	unsigned short sel, old_sel;
	struct desc_struct old_desc, new_desc;
	const struct x86_emulate_ops *ops = ctxt->ops;
	u8 cpl = ctxt->ops->cpl(ctxt);

	/* Assignment of RIP may only fail in 64-bit mode */
	if (ctxt->mode == X86EMUL_MODE_PROT64)
		ops->get_segment(ctxt, &old_sel, &old_desc, NULL,
				 VCPU_SREG_CS);

	memcpy(&sel, ctxt->src.valptr + ctxt->op_bytes, 2);

	rc = __load_segment_descriptor(ctxt, sel, VCPU_SREG_CS, cpl, false,
				       &new_desc);
	if (rc != X86EMUL_CONTINUE)
		return rc;

	rc = assign_eip_far(ctxt, ctxt->src.val, new_desc.l);
	if (rc != X86EMUL_CONTINUE) {
		WARN_ON(!ctxt->mode != X86EMUL_MODE_PROT64);
		/* assigning eip failed; restore the old cs */
		ops->set_segment(ctxt, old_sel, &old_desc, 0, VCPU_SREG_CS);
		return rc;
	}
	return rc;
}

static int em_grp45(struct x86_emulate_ctxt *ctxt)
{
static int em_ret_far(struct x86_emulate_ctxt *ctxt)
{
	int rc;
	unsigned long eip, cs;
	u16 old_cs;
	int cpl = ctxt->ops->cpl(ctxt);
	struct desc_struct old_desc, new_desc;
	const struct x86_emulate_ops *ops = ctxt->ops;

	if (ctxt->mode == X86EMUL_MODE_PROT64)
		ops->get_segment(ctxt, &old_cs, &old_desc, NULL,
				 VCPU_SREG_CS);

	rc = emulate_pop(ctxt, &eip, ctxt->op_bytes);
	if (rc != X86EMUL_CONTINUE)
		return rc;
	rc = emulate_pop(ctxt, &cs, ctxt->op_bytes);
	if (rc != X86EMUL_CONTINUE)
		return rc;
	/* Outer-privilege level return is not implemented */
	if (ctxt->mode >= X86EMUL_MODE_PROT16 && (cs & 3) > cpl)
		return X86EMUL_UNHANDLEABLE;
	rc = __load_segment_descriptor(ctxt, (u16)cs, VCPU_SREG_CS, 0, false,
				       &new_desc);
	if (rc != X86EMUL_CONTINUE)
		return rc;
	rc = assign_eip_far(ctxt, eip, new_desc.l);
	if (rc != X86EMUL_CONTINUE) {
		WARN_ON(!ctxt->mode != X86EMUL_MODE_PROT64);
		ops->set_segment(ctxt, old_cs, &old_desc, 0, VCPU_SREG_CS);
	}
	return rc;
}

static int em_ret_far_imm(struct x86_emulate_ctxt *ctxt)
	 * Now load segment descriptors. If fault happens at this stage
	 * it is handled in a context of new task
	 */
	ret = __load_segment_descriptor(ctxt, tss->ldt, VCPU_SREG_LDTR, cpl,
					true, NULL);
	if (ret != X86EMUL_CONTINUE)
		return ret;
	ret = __load_segment_descriptor(ctxt, tss->es, VCPU_SREG_ES, cpl,
					true, NULL);
	if (ret != X86EMUL_CONTINUE)
		return ret;
	ret = __load_segment_descriptor(ctxt, tss->cs, VCPU_SREG_CS, cpl,
					true, NULL);
	if (ret != X86EMUL_CONTINUE)
		return ret;
	ret = __load_segment_descriptor(ctxt, tss->ss, VCPU_SREG_SS, cpl,
					true, NULL);
	if (ret != X86EMUL_CONTINUE)
		return ret;
	ret = __load_segment_descriptor(ctxt, tss->ds, VCPU_SREG_DS, cpl,
					true, NULL);
	if (ret != X86EMUL_CONTINUE)
		return ret;

	return X86EMUL_CONTINUE;
	 * Now load segment descriptors. If fault happenes at this stage
	 * it is handled in a context of new task
	 */
	ret = __load_segment_descriptor(ctxt, tss->ldt_selector, VCPU_SREG_LDTR,
					cpl, true, NULL);
	if (ret != X86EMUL_CONTINUE)
		return ret;
	ret = __load_segment_descriptor(ctxt, tss->es, VCPU_SREG_ES, cpl,
					true, NULL);
	if (ret != X86EMUL_CONTINUE)
		return ret;
	ret = __load_segment_descriptor(ctxt, tss->cs, VCPU_SREG_CS, cpl,
					true, NULL);
	if (ret != X86EMUL_CONTINUE)
		return ret;
	ret = __load_segment_descriptor(ctxt, tss->ss, VCPU_SREG_SS, cpl,
					true, NULL);
	if (ret != X86EMUL_CONTINUE)
		return ret;
	ret = __load_segment_descriptor(ctxt, tss->ds, VCPU_SREG_DS, cpl,
					true, NULL);
	if (ret != X86EMUL_CONTINUE)
		return ret;
	ret = __load_segment_descriptor(ctxt, tss->fs, VCPU_SREG_FS, cpl,
					true, NULL);
	if (ret != X86EMUL_CONTINUE)
		return ret;
	ret = __load_segment_descriptor(ctxt, tss->gs, VCPU_SREG_GS, cpl,
					true, NULL);
	if (ret != X86EMUL_CONTINUE)
		return ret;

	return X86EMUL_CONTINUE;
	u16 sel, old_cs;
	ulong old_eip;
	int rc;
	struct desc_struct old_desc, new_desc;
	const struct x86_emulate_ops *ops = ctxt->ops;
	int cpl = ctxt->ops->cpl(ctxt);

	old_eip = ctxt->_eip;
	ops->get_segment(ctxt, &old_cs, &old_desc, NULL, VCPU_SREG_CS);

	memcpy(&sel, ctxt->src.valptr + ctxt->op_bytes, 2);
	rc = __load_segment_descriptor(ctxt, sel, VCPU_SREG_CS, cpl, false,
				       &new_desc);
	if (rc != X86EMUL_CONTINUE)
		return X86EMUL_CONTINUE;

	rc = assign_eip_far(ctxt, ctxt->src.val, new_desc.l);
	if (rc != X86EMUL_CONTINUE)
		goto fail;

	ctxt->src.val = old_cs;
	rc = em_push(ctxt);
	if (rc != X86EMUL_CONTINUE)
		goto fail;

	ctxt->src.val = old_eip;
	rc = em_push(ctxt);
	/* If we failed, we tainted the memory, but the very least we should
	   restore cs */
	if (rc != X86EMUL_CONTINUE)
		goto fail;
	return rc;
fail:
	ops->set_segment(ctxt, old_cs, &old_desc, 0, VCPU_SREG_CS);
	return rc;

}

static int em_ret_near_imm(struct x86_emulate_ctxt *ctxt)
{
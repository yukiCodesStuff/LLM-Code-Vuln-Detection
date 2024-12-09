				    &ctxt->exception);
}

static int __load_segment_descriptor(struct x86_emulate_ctxt *ctxt,
				     u16 selector, int seg, u8 cpl,
				     enum x86_transfer_type transfer,
				     struct desc_struct *desc)

	rpl = selector & 3;

	/* TR should be in GDT only */
	if (seg == VCPU_SREG_TR && (selector & (1 << 2)))
		goto exception;

	/* NULL selector is not valid for TR, CS and (except for long mode) SS */
	if (null_selector) {
		if (seg == VCPU_SREG_CS || seg == VCPU_SREG_TR)
			goto exception;

		if (seg == VCPU_SREG_SS) {
			if (ctxt->mode != X86EMUL_MODE_PROT64 || rpl != cpl)
				goto exception;

			/*
			 * ctxt->ops->set_segment expects the CPL to be in
			 * SS.DPL, so fake an expand-up 32-bit data segment.
			 */
			seg_desc.type = 3;
			seg_desc.p = 1;
			seg_desc.s = 1;
			seg_desc.dpl = cpl;
			seg_desc.d = 1;
			seg_desc.g = 1;
		}

		/* Skip all following checks */
		goto load;
	}

	ret = read_segment_descriptor(ctxt, selector, &seg_desc, &desc_addr);
	if (ret != X86EMUL_CONTINUE)
		return ret;
				   u16 selector, int seg)
{
	u8 cpl = ctxt->ops->cpl(ctxt);

	/*
	 * None of MOV, POP and LSS can load a NULL selector in CPL=3, but
	 * they can load it at CPL<3 (Intel's manual says only LSS can,
	 * but it's wrong).
	 *
	 * However, the Intel manual says that putting IST=1/DPL=3 in
	 * an interrupt gate will result in SS=3 (the AMD manual instead
	 * says it doesn't), so allow SS=3 in __load_segment_descriptor
	 * and only forbid it here.
	 */
	if (seg == VCPU_SREG_SS && selector == 3 &&
	    ctxt->mode == X86EMUL_MODE_PROT64)
		return emulate_exception(ctxt, GP_VECTOR, 0, true);

	return __load_segment_descriptor(ctxt, selector, seg, cpl,
					 X86_TRANSFER_NONE, NULL);
}

				    &ctxt->exception);
}

/* Does not support long mode */
static int __load_segment_descriptor(struct x86_emulate_ctxt *ctxt,
				     u16 selector, int seg, u8 cpl,
				     enum x86_transfer_type transfer,
				     struct desc_struct *desc)

	rpl = selector & 3;

	/* NULL selector is not valid for TR, CS and SS (except for long mode) */
	if ((seg == VCPU_SREG_CS
	     || (seg == VCPU_SREG_SS
		 && (ctxt->mode != X86EMUL_MODE_PROT64 || rpl != cpl))
	     || seg == VCPU_SREG_TR)
	    && null_selector)
		goto exception;

	/* TR should be in GDT only */
	if (seg == VCPU_SREG_TR && (selector & (1 << 2)))
		goto exception;

	if (null_selector) /* for NULL selector skip all following checks */
		goto load;

	ret = read_segment_descriptor(ctxt, selector, &seg_desc, &desc_addr);
	if (ret != X86EMUL_CONTINUE)
		return ret;
				   u16 selector, int seg)
{
	u8 cpl = ctxt->ops->cpl(ctxt);
	return __load_segment_descriptor(ctxt, selector, seg, cpl,
					 X86_TRANSFER_NONE, NULL);
}

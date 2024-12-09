	/* Decode and fetch the destination operand: register or memory. */
	rc = decode_operand(ctxt, &ctxt->dst, (ctxt->d >> DstShift) & OpMask);

	if (ctxt->rip_relative)
		ctxt->memopp->addr.mem.ea += ctxt->_eip;

done:
	return (rc != X86EMUL_CONTINUE) ? EMULATION_FAILED : EMULATION_OK;
}

bool x86_page_table_writing_insn(struct x86_emulate_ctxt *ctxt)
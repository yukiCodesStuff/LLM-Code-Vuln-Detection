	 * Not recognized on AMD in compat mode (but is recognized in legacy
	 * mode).
	 */
	if ((ctxt->mode != X86EMUL_MODE_PROT64) && (efer & EFER_LMA)
	    && !vendor_intel(ctxt))
		return emulate_ud(ctxt);

	/* sysenter/sysexit have not been tested in 64bit mode. */
	setup_syscalls_segments(ctxt, &cs, &ss);

	ops->get_msr(ctxt, MSR_IA32_SYSENTER_CS, &msr_data);
	if ((msr_data & 0xfffc) == 0x0)
		return emulate_gp(ctxt, 0);

	ctxt->eflags &= ~(EFLG_VM | EFLG_IF);
	cs_sel = (u16)msr_data & ~SELECTOR_RPL_MASK;
	ss_sel = cs_sel + 8;
	if (efer & EFER_LMA) {
		cs.d = 0;
		cs.l = 1;
	}

	ops->set_segment(ctxt, ss_sel, &ss, 0, VCPU_SREG_SS);

	ops->get_msr(ctxt, MSR_IA32_SYSENTER_EIP, &msr_data);
	ctxt->_eip = (efer & EFER_LMA) ? msr_data : (u32)msr_data;

	ops->get_msr(ctxt, MSR_IA32_SYSENTER_ESP, &msr_data);
	*reg_write(ctxt, VCPU_REGS_RSP) = (efer & EFER_LMA) ? msr_data :
							      (u32)msr_data;

	return X86EMUL_CONTINUE;
}

};

static const struct opcode group6[] = {
	DI(Prot | DstMem,	sldt),
	DI(Prot | DstMem,	str),
	II(Prot | Priv | SrcMem16, em_lldt, lldt),
	II(Prot | Priv | SrcMem16, em_ltr, ltr),
	N, N, N, N,
};
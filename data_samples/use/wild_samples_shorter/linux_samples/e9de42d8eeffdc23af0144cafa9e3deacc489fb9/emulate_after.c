			goto exception;
		break;
	case VCPU_SREG_CS:
		if (!(seg_desc.type & 8))
			goto exception;

		if (seg_desc.type & 4) {

	ctxt->execute = opcode.u.execute;

	if (unlikely(ctxt->ud) && likely(!(ctxt->d & EmulateOnUD)))
		return EMULATION_FAILED;

	if (unlikely(ctxt->d &
		     (NotImpl|Stack|Op3264|Sse|Mmx|Intercept|CheckPerm))) {
		/*
		 * These are copied unconditionally here, and checked unconditionally
		 * in x86_emulate_insn.
		 */
		if (ctxt->d & NotImpl)
			return EMULATION_FAILED;

		if (mode == X86EMUL_MODE_PROT64 && (ctxt->d & Stack))
			ctxt->op_bytes = 8;

		if (ctxt->d & Op3264) {
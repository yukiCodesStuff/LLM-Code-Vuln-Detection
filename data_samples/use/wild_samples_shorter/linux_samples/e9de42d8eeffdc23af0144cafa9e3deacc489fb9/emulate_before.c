			goto exception;
		break;
	case VCPU_SREG_CS:
		if (in_task_switch && rpl != dpl)
			goto exception;

		if (!(seg_desc.type & 8))
			goto exception;

		if (seg_desc.type & 4) {

	ctxt->execute = opcode.u.execute;

	if (unlikely(ctxt->d &
		     (NotImpl|EmulateOnUD|Stack|Op3264|Sse|Mmx|Intercept|CheckPerm))) {
		/*
		 * These are copied unconditionally here, and checked unconditionally
		 * in x86_emulate_insn.
		 */
		if (ctxt->d & NotImpl)
			return EMULATION_FAILED;

		if (!(ctxt->d & EmulateOnUD) && ctxt->ud)
			return EMULATION_FAILED;

		if (mode == X86EMUL_MODE_PROT64 && (ctxt->d & Stack))
			ctxt->op_bytes = 8;

		if (ctxt->d & Op3264) {
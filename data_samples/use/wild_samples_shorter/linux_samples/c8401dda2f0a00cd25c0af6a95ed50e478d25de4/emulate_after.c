		ctxt->eflags &= ~(X86_EFLAGS_VM | X86_EFLAGS_IF);
	}

	ctxt->tf = (ctxt->eflags & X86_EFLAGS_TF) != 0;
	return X86EMUL_CONTINUE;
}

static int em_sysenter(struct x86_emulate_ctxt *ctxt)
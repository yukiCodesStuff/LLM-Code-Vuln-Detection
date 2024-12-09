	return ctxt->ops->read_std(ctxt, linear, data, size, &ctxt->exception);
}

/*
 * Prefetch the remaining bytes of the instruction without crossing page
 * boundary if they are not in fetch_cache yet.
 */
	}
	/* Disable writeback. */
	ctxt->dst.type = OP_NONE;
	return segmented_write(ctxt, ctxt->dst.addr.mem,
			       &desc_ptr, 2 + ctxt->op_bytes);
}

static int em_sgdt(struct x86_emulate_ctxt *ctxt)
{
	else
		size = offsetof(struct fxregs_state, xmm_space[0]);

	return segmented_write(ctxt, ctxt->memop.addr.mem, &fx_state, size);
}

static int fxrstor_fixup(struct x86_emulate_ctxt *ctxt,
		struct fxregs_state *new)
	if (rc != X86EMUL_CONTINUE)
		return rc;

	rc = segmented_read(ctxt, ctxt->memop.addr.mem, &fx_state, 512);
	if (rc != X86EMUL_CONTINUE)
		return rc;

	if (fx_state.mxcsr >> 16)
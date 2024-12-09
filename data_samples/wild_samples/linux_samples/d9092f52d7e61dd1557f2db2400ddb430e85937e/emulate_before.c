		if (!has_seg_override) {
			has_seg_override = true;
			ctxt->seg_override = ctxt->modrm_seg;
		}
	} else if (ctxt->d & MemAbs)
		rc = decode_abs(ctxt, &ctxt->memop);
	if (rc != X86EMUL_CONTINUE)
		goto done;

	if (!has_seg_override)
		ctxt->seg_override = VCPU_SREG_DS;

	ctxt->memop.addr.mem.seg = ctxt->seg_override;

	/*
	 * Decode and fetch the source operand: register, memory
	 * or immediate.
	 */
	rc = decode_operand(ctxt, &ctxt->src, (ctxt->d >> SrcShift) & OpMask);
	if (rc != X86EMUL_CONTINUE)
		goto done;

	/*
	 * Decode and fetch the second source operand: register, memory
	 * or immediate.
	 */
	rc = decode_operand(ctxt, &ctxt->src2, (ctxt->d >> Src2Shift) & OpMask);
	if (rc != X86EMUL_CONTINUE)
		goto done;

	/* Decode and fetch the destination operand: register or memory. */
	rc = decode_operand(ctxt, &ctxt->dst, (ctxt->d >> DstShift) & OpMask);

	if (ctxt->rip_relative)
		ctxt->memopp->addr.mem.ea = address_mask(ctxt,
					ctxt->memopp->addr.mem.ea + ctxt->_eip);

done:
	return (rc != X86EMUL_CONTINUE) ? EMULATION_FAILED : EMULATION_OK;
}

bool x86_page_table_writing_insn(struct x86_emulate_ctxt *ctxt)
{
	return ctxt->d & PageTable;
}

static bool string_insn_completed(struct x86_emulate_ctxt *ctxt)
{
	/* The second termination condition only applies for REPE
	 * and REPNE. Test if the repeat string operation prefix is
	 * REPE/REPZ or REPNE/REPNZ and if it's the case it tests the
	 * corresponding termination condition according to:
	 * 	- if REPE/REPZ and ZF = 0 then done
	 * 	- if REPNE/REPNZ and ZF = 1 then done
	 */
	if (((ctxt->b == 0xa6) || (ctxt->b == 0xa7) ||
	     (ctxt->b == 0xae) || (ctxt->b == 0xaf))
	    && (((ctxt->rep_prefix == REPE_PREFIX) &&
		 ((ctxt->eflags & X86_EFLAGS_ZF) == 0))
		|| ((ctxt->rep_prefix == REPNE_PREFIX) &&
		    ((ctxt->eflags & X86_EFLAGS_ZF) == X86_EFLAGS_ZF))))
		return true;

	return false;
}

static int flush_pending_x87_faults(struct x86_emulate_ctxt *ctxt)
{
	bool fault = false;

	ctxt->ops->get_fpu(ctxt);
	asm volatile("1: fwait \n\t"
		     "2: \n\t"
		     ".pushsection .fixup,\"ax\" \n\t"
		     "3: \n\t"
		     "movb $1, %[fault] \n\t"
		     "jmp 2b \n\t"
		     ".popsection \n\t"
		     _ASM_EXTABLE(1b, 3b)
		     : [fault]"+qm"(fault));
	ctxt->ops->put_fpu(ctxt);

	if (unlikely(fault))
		return emulate_exception(ctxt, MF_VECTOR, 0, false);

	return X86EMUL_CONTINUE;
}

static void fetch_possible_mmx_operand(struct x86_emulate_ctxt *ctxt,
				       struct operand *op)
{
	if (op->type == OP_MM)
		read_mmx_reg(ctxt, &op->mm_val, op->addr.mm);
}

static int fastop(struct x86_emulate_ctxt *ctxt, void (*fop)(struct fastop *))
{
	register void *__sp asm(_ASM_SP);
	ulong flags = (ctxt->eflags & EFLAGS_MASK) | X86_EFLAGS_IF;

	if (!(ctxt->d & ByteOp))
		fop += __ffs(ctxt->dst.bytes) * FASTOP_SIZE;

	asm("push %[flags]; popf; call *%[fastop]; pushf; pop %[flags]\n"
	    : "+a"(ctxt->dst.val), "+d"(ctxt->src.val), [flags]"+D"(flags),
	      [fastop]"+S"(fop), "+r"(__sp)
	    : "c"(ctxt->src2.val));

	ctxt->eflags = (ctxt->eflags & ~EFLAGS_MASK) | (flags & EFLAGS_MASK);
	if (!fop) /* exception is returned in fop variable */
		return emulate_de(ctxt);
	return X86EMUL_CONTINUE;
}

void init_decode_cache(struct x86_emulate_ctxt *ctxt)
{
	memset(&ctxt->rip_relative, 0,
	       (void *)&ctxt->modrm - (void *)&ctxt->rip_relative);

	ctxt->io_read.pos = 0;
	ctxt->io_read.end = 0;
	ctxt->mem_read.end = 0;
}

int x86_emulate_insn(struct x86_emulate_ctxt *ctxt)
{
	const struct x86_emulate_ops *ops = ctxt->ops;
	int rc = X86EMUL_CONTINUE;
	int saved_dst_type = ctxt->dst.type;

	ctxt->mem_read.pos = 0;

	/* LOCK prefix is allowed only with some instructions */
	if (ctxt->lock_prefix && (!(ctxt->d & Lock) || ctxt->dst.type != OP_MEM)) {
		rc = emulate_ud(ctxt);
		goto done;
	}
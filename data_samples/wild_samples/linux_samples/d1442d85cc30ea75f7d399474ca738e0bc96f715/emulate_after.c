{
	struct desc_ptr dt;
	u16 index = selector >> 3;
	ulong addr;

	get_descriptor_table_ptr(ctxt, selector, &dt);

	if (dt.size < index * 8 + 7)
		return emulate_gp(ctxt, selector & 0xfffc);

	addr = dt.address + index * 8;
	return ctxt->ops->write_std(ctxt, addr, desc, sizeof *desc,
				    &ctxt->exception);
}

/* Does not support long mode */
static int __load_segment_descriptor(struct x86_emulate_ctxt *ctxt,
				     u16 selector, int seg, u8 cpl,
				     bool in_task_switch,
				     struct desc_struct *desc)
{
	struct desc_struct seg_desc, old_desc;
	u8 dpl, rpl;
	unsigned err_vec = GP_VECTOR;
	u32 err_code = 0;
	bool null_selector = !(selector & ~0x3); /* 0000-0003 are null */
	ulong desc_addr;
	int ret;
	u16 dummy;
	u32 base3 = 0;

	memset(&seg_desc, 0, sizeof seg_desc);

	if (ctxt->mode == X86EMUL_MODE_REAL) {
		/* set real mode segment descriptor (keep limit etc. for
		 * unreal mode) */
		ctxt->ops->get_segment(ctxt, &dummy, &seg_desc, NULL, seg);
		set_desc_base(&seg_desc, selector << 4);
		goto load;
	} else if (seg <= VCPU_SREG_GS && ctxt->mode == X86EMUL_MODE_VM86) {
		/* VM86 needs a clean new segment descriptor */
		set_desc_base(&seg_desc, selector << 4);
		set_desc_limit(&seg_desc, 0xffff);
		seg_desc.type = 3;
		seg_desc.p = 1;
		seg_desc.s = 1;
		seg_desc.dpl = 3;
		goto load;
	}

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

	err_code = selector & 0xfffc;
	err_vec = in_task_switch ? TS_VECTOR : GP_VECTOR;

	/* can't load system descriptor into segment selector */
	if (seg <= VCPU_SREG_GS && !seg_desc.s)
		goto exception;

	if (!seg_desc.p) {
		err_vec = (seg == VCPU_SREG_SS) ? SS_VECTOR : NP_VECTOR;
		goto exception;
	}

	dpl = seg_desc.dpl;

	switch (seg) {
	case VCPU_SREG_SS:
		/*
		 * segment is not a writable data segment or segment
		 * selector's RPL != CPL or segment selector's RPL != CPL
		 */
		if (rpl != cpl || (seg_desc.type & 0xa) != 0x2 || dpl != cpl)
			goto exception;
		break;
	case VCPU_SREG_CS:
		if (!(seg_desc.type & 8))
			goto exception;

		if (seg_desc.type & 4) {
			/* conforming */
			if (dpl > cpl)
				goto exception;
		} else {
			/* nonconforming */
			if (rpl > cpl || dpl != cpl)
				goto exception;
		}
		/* in long-mode d/b must be clear if l is set */
		if (seg_desc.d && seg_desc.l) {
			u64 efer = 0;

			ctxt->ops->get_msr(ctxt, MSR_EFER, &efer);
			if (efer & EFER_LMA)
				goto exception;
		}

		/* CS(RPL) <- CPL */
		selector = (selector & 0xfffc) | cpl;
		break;
	case VCPU_SREG_TR:
		if (seg_desc.s || (seg_desc.type != 1 && seg_desc.type != 9))
			goto exception;
		old_desc = seg_desc;
		seg_desc.type |= 2; /* busy */
		ret = ctxt->ops->cmpxchg_emulated(ctxt, desc_addr, &old_desc, &seg_desc,
						  sizeof(seg_desc), &ctxt->exception);
		if (ret != X86EMUL_CONTINUE)
			return ret;
		break;
	case VCPU_SREG_LDTR:
		if (seg_desc.s || seg_desc.type != 2)
			goto exception;
		break;
	default: /*  DS, ES, FS, or GS */
		/*
		 * segment is not a data or readable code segment or
		 * ((segment is a data or nonconforming code segment)
		 * and (both RPL and CPL > DPL))
		 */
		if ((seg_desc.type & 0xa) == 0x8 ||
		    (((seg_desc.type & 0xc) != 0xc) &&
		     (rpl > dpl && cpl > dpl)))
			goto exception;
		break;
	}

	if (seg_desc.s) {
		/* mark segment as accessed */
		seg_desc.type |= 1;
		ret = write_segment_descriptor(ctxt, selector, &seg_desc);
		if (ret != X86EMUL_CONTINUE)
			return ret;
	} else if (ctxt->mode == X86EMUL_MODE_PROT64) {
		ret = ctxt->ops->read_std(ctxt, desc_addr+8, &base3,
				sizeof(base3), &ctxt->exception);
		if (ret != X86EMUL_CONTINUE)
			return ret;
	}
load:
	ctxt->ops->set_segment(ctxt, selector, &seg_desc, base3, seg);
	if (desc)
		*desc = seg_desc;
	return X86EMUL_CONTINUE;
exception:
	return emulate_exception(ctxt, err_vec, err_code, true);
}

static int load_segment_descriptor(struct x86_emulate_ctxt *ctxt,
				   u16 selector, int seg)
{
	u8 cpl = ctxt->ops->cpl(ctxt);
	return __load_segment_descriptor(ctxt, selector, seg, cpl, false, NULL);
}

static void write_register_operand(struct operand *op)
{
	/* The 4-byte case *is* correct: in 64-bit mode we zero-extend. */
	switch (op->bytes) {
	case 1:
		*(u8 *)op->addr.reg = (u8)op->val;
		break;
	case 2:
		*(u16 *)op->addr.reg = (u16)op->val;
		break;
	case 4:
		*op->addr.reg = (u32)op->val;
		break;	/* 64b: zero-extend */
	case 8:
		*op->addr.reg = op->val;
		break;
	}
}

static int writeback(struct x86_emulate_ctxt *ctxt, struct operand *op)
{
	switch (op->type) {
	case OP_REG:
		write_register_operand(op);
		break;
	case OP_MEM:
		if (ctxt->lock_prefix)
			return segmented_cmpxchg(ctxt,
						 op->addr.mem,
						 &op->orig_val,
						 &op->val,
						 op->bytes);
		else
			return segmented_write(ctxt,
					       op->addr.mem,
					       &op->val,
					       op->bytes);
		break;
	case OP_MEM_STR:
		return segmented_write(ctxt,
				       op->addr.mem,
				       op->data,
				       op->bytes * op->count);
		break;
	case OP_XMM:
		write_sse_reg(ctxt, &op->vec_val, op->addr.xmm);
		break;
	case OP_MM:
		write_mmx_reg(ctxt, &op->mm_val, op->addr.mm);
		break;
	case OP_NONE:
		/* no writeback */
		break;
	default:
		break;
	}
	return X86EMUL_CONTINUE;
}

static int push(struct x86_emulate_ctxt *ctxt, void *data, int bytes)
{
	struct segmented_address addr;

	rsp_increment(ctxt, -bytes);
	addr.ea = reg_read(ctxt, VCPU_REGS_RSP) & stack_mask(ctxt);
	addr.seg = VCPU_SREG_SS;

	return segmented_write(ctxt, addr, data, bytes);
}

static int em_push(struct x86_emulate_ctxt *ctxt)
{
	/* Disable writeback. */
	ctxt->dst.type = OP_NONE;
	return push(ctxt, &ctxt->src.val, ctxt->op_bytes);
}

static int emulate_pop(struct x86_emulate_ctxt *ctxt,
		       void *dest, int len)
{
	int rc;
	struct segmented_address addr;

	addr.ea = reg_read(ctxt, VCPU_REGS_RSP) & stack_mask(ctxt);
	addr.seg = VCPU_SREG_SS;
	rc = segmented_read(ctxt, addr, dest, len);
	if (rc != X86EMUL_CONTINUE)
		return rc;

	rsp_increment(ctxt, len);
	return rc;
}

static int em_pop(struct x86_emulate_ctxt *ctxt)
{
	return emulate_pop(ctxt, &ctxt->dst.val, ctxt->op_bytes);
}

static int emulate_popf(struct x86_emulate_ctxt *ctxt,
			void *dest, int len)
{
	int rc;
	unsigned long val, change_mask;
	int iopl = (ctxt->eflags & X86_EFLAGS_IOPL) >> IOPL_SHIFT;
	int cpl = ctxt->ops->cpl(ctxt);

	rc = emulate_pop(ctxt, &val, len);
	if (rc != X86EMUL_CONTINUE)
		return rc;

	change_mask = EFLG_CF | EFLG_PF | EFLG_AF | EFLG_ZF | EFLG_SF | EFLG_OF
		| EFLG_TF | EFLG_DF | EFLG_NT | EFLG_AC | EFLG_ID;

	switch(ctxt->mode) {
	case X86EMUL_MODE_PROT64:
	case X86EMUL_MODE_PROT32:
	case X86EMUL_MODE_PROT16:
		if (cpl == 0)
			change_mask |= EFLG_IOPL;
		if (cpl <= iopl)
			change_mask |= EFLG_IF;
		break;
	case X86EMUL_MODE_VM86:
		if (iopl < 3)
			return emulate_gp(ctxt, 0);
		change_mask |= EFLG_IF;
		break;
	default: /* real mode */
		change_mask |= (EFLG_IOPL | EFLG_IF);
		break;
	}

	*(unsigned long *)dest =
		(ctxt->eflags & ~change_mask) | (val & change_mask);

	return rc;
}

static int em_popf(struct x86_emulate_ctxt *ctxt)
{
	ctxt->dst.type = OP_REG;
	ctxt->dst.addr.reg = &ctxt->eflags;
	ctxt->dst.bytes = ctxt->op_bytes;
	return emulate_popf(ctxt, &ctxt->dst.val, ctxt->op_bytes);
}

static int em_enter(struct x86_emulate_ctxt *ctxt)
{
	int rc;
	unsigned frame_size = ctxt->src.val;
	unsigned nesting_level = ctxt->src2.val & 31;
	ulong rbp;

	if (nesting_level)
		return X86EMUL_UNHANDLEABLE;

	rbp = reg_read(ctxt, VCPU_REGS_RBP);
	rc = push(ctxt, &rbp, stack_size(ctxt));
	if (rc != X86EMUL_CONTINUE)
		return rc;
	assign_masked(reg_rmw(ctxt, VCPU_REGS_RBP), reg_read(ctxt, VCPU_REGS_RSP),
		      stack_mask(ctxt));
	assign_masked(reg_rmw(ctxt, VCPU_REGS_RSP),
		      reg_read(ctxt, VCPU_REGS_RSP) - frame_size,
		      stack_mask(ctxt));
	return X86EMUL_CONTINUE;
}

static int em_leave(struct x86_emulate_ctxt *ctxt)
{
	assign_masked(reg_rmw(ctxt, VCPU_REGS_RSP), reg_read(ctxt, VCPU_REGS_RBP),
		      stack_mask(ctxt));
	return emulate_pop(ctxt, reg_rmw(ctxt, VCPU_REGS_RBP), ctxt->op_bytes);
}

static int em_push_sreg(struct x86_emulate_ctxt *ctxt)
{
	int seg = ctxt->src2.val;

	ctxt->src.val = get_segment_selector(ctxt, seg);

	return em_push(ctxt);
}

static int em_pop_sreg(struct x86_emulate_ctxt *ctxt)
{
	int seg = ctxt->src2.val;
	unsigned long selector;
	int rc;

	rc = emulate_pop(ctxt, &selector, ctxt->op_bytes);
	if (rc != X86EMUL_CONTINUE)
		return rc;

	if (ctxt->modrm_reg == VCPU_SREG_SS)
		ctxt->interruptibility = KVM_X86_SHADOW_INT_MOV_SS;

	rc = load_segment_descriptor(ctxt, (u16)selector, seg);
	return rc;
}

static int em_pusha(struct x86_emulate_ctxt *ctxt)
{
	unsigned long old_esp = reg_read(ctxt, VCPU_REGS_RSP);
	int rc = X86EMUL_CONTINUE;
	int reg = VCPU_REGS_RAX;

	while (reg <= VCPU_REGS_RDI) {
		(reg == VCPU_REGS_RSP) ?
		(ctxt->src.val = old_esp) : (ctxt->src.val = reg_read(ctxt, reg));

		rc = em_push(ctxt);
		if (rc != X86EMUL_CONTINUE)
			return rc;

		++reg;
	}

	return rc;
}

static int em_pushf(struct x86_emulate_ctxt *ctxt)
{
	ctxt->src.val =  (unsigned long)ctxt->eflags;
	return em_push(ctxt);
}

static int em_popa(struct x86_emulate_ctxt *ctxt)
{
	int rc = X86EMUL_CONTINUE;
	int reg = VCPU_REGS_RDI;

	while (reg >= VCPU_REGS_RAX) {
		if (reg == VCPU_REGS_RSP) {
			rsp_increment(ctxt, ctxt->op_bytes);
			--reg;
		}

		rc = emulate_pop(ctxt, reg_rmw(ctxt, reg), ctxt->op_bytes);
		if (rc != X86EMUL_CONTINUE)
			break;
		--reg;
	}
	return rc;
}

static int __emulate_int_real(struct x86_emulate_ctxt *ctxt, int irq)
{
	const struct x86_emulate_ops *ops = ctxt->ops;
	int rc;
	struct desc_ptr dt;
	gva_t cs_addr;
	gva_t eip_addr;
	u16 cs, eip;

	/* TODO: Add limit checks */
	ctxt->src.val = ctxt->eflags;
	rc = em_push(ctxt);
	if (rc != X86EMUL_CONTINUE)
		return rc;

	ctxt->eflags &= ~(EFLG_IF | EFLG_TF | EFLG_AC);

	ctxt->src.val = get_segment_selector(ctxt, VCPU_SREG_CS);
	rc = em_push(ctxt);
	if (rc != X86EMUL_CONTINUE)
		return rc;

	ctxt->src.val = ctxt->_eip;
	rc = em_push(ctxt);
	if (rc != X86EMUL_CONTINUE)
		return rc;

	ops->get_idt(ctxt, &dt);

	eip_addr = dt.address + (irq << 2);
	cs_addr = dt.address + (irq << 2) + 2;

	rc = ops->read_std(ctxt, cs_addr, &cs, 2, &ctxt->exception);
	if (rc != X86EMUL_CONTINUE)
		return rc;

	rc = ops->read_std(ctxt, eip_addr, &eip, 2, &ctxt->exception);
	if (rc != X86EMUL_CONTINUE)
		return rc;

	rc = load_segment_descriptor(ctxt, cs, VCPU_SREG_CS);
	if (rc != X86EMUL_CONTINUE)
		return rc;

	ctxt->_eip = eip;

	return rc;
}

int emulate_int_real(struct x86_emulate_ctxt *ctxt, int irq)
{
	int rc;

	invalidate_registers(ctxt);
	rc = __emulate_int_real(ctxt, irq);
	if (rc == X86EMUL_CONTINUE)
		writeback_registers(ctxt);
	return rc;
}

static int emulate_int(struct x86_emulate_ctxt *ctxt, int irq)
{
	switch(ctxt->mode) {
	case X86EMUL_MODE_REAL:
		return __emulate_int_real(ctxt, irq);
	case X86EMUL_MODE_VM86:
	case X86EMUL_MODE_PROT16:
	case X86EMUL_MODE_PROT32:
	case X86EMUL_MODE_PROT64:
	default:
		/* Protected mode interrupts unimplemented yet */
		return X86EMUL_UNHANDLEABLE;
	}
}

static int emulate_iret_real(struct x86_emulate_ctxt *ctxt)
{
	int rc = X86EMUL_CONTINUE;
	unsigned long temp_eip = 0;
	unsigned long temp_eflags = 0;
	unsigned long cs = 0;
	unsigned long mask = EFLG_CF | EFLG_PF | EFLG_AF | EFLG_ZF | EFLG_SF | EFLG_TF |
			     EFLG_IF | EFLG_DF | EFLG_OF | EFLG_IOPL | EFLG_NT | EFLG_RF |
			     EFLG_AC | EFLG_ID | (1 << 1); /* Last one is the reserved bit */
	unsigned long vm86_mask = EFLG_VM | EFLG_VIF | EFLG_VIP;

	/* TODO: Add stack limit check */

	rc = emulate_pop(ctxt, &temp_eip, ctxt->op_bytes);

	if (rc != X86EMUL_CONTINUE)
		return rc;

	if (temp_eip & ~0xffff)
		return emulate_gp(ctxt, 0);

	rc = emulate_pop(ctxt, &cs, ctxt->op_bytes);

	if (rc != X86EMUL_CONTINUE)
		return rc;

	rc = emulate_pop(ctxt, &temp_eflags, ctxt->op_bytes);

	if (rc != X86EMUL_CONTINUE)
		return rc;

	rc = load_segment_descriptor(ctxt, (u16)cs, VCPU_SREG_CS);

	if (rc != X86EMUL_CONTINUE)
		return rc;

	ctxt->_eip = temp_eip;


	if (ctxt->op_bytes == 4)
		ctxt->eflags = ((temp_eflags & mask) | (ctxt->eflags & vm86_mask));
	else if (ctxt->op_bytes == 2) {
		ctxt->eflags &= ~0xffff;
		ctxt->eflags |= temp_eflags;
	}

	ctxt->eflags &= ~EFLG_RESERVED_ZEROS_MASK; /* Clear reserved zeros */
	ctxt->eflags |= EFLG_RESERVED_ONE_MASK;

	return rc;
}

static int em_iret(struct x86_emulate_ctxt *ctxt)
{
	switch(ctxt->mode) {
	case X86EMUL_MODE_REAL:
		return emulate_iret_real(ctxt);
	case X86EMUL_MODE_VM86:
	case X86EMUL_MODE_PROT16:
	case X86EMUL_MODE_PROT32:
	case X86EMUL_MODE_PROT64:
	default:
		/* iret from protected mode unimplemented yet */
		return X86EMUL_UNHANDLEABLE;
	}
}

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
	int rc = X86EMUL_CONTINUE;

	switch (ctxt->modrm_reg) {
	case 2: /* call near abs */ {
		long int old_eip;
		old_eip = ctxt->_eip;
		rc = assign_eip_near(ctxt, ctxt->src.val);
		if (rc != X86EMUL_CONTINUE)
			break;
		ctxt->src.val = old_eip;
		rc = em_push(ctxt);
		break;
	}
	case 4: /* jmp abs */
		rc = assign_eip_near(ctxt, ctxt->src.val);
		break;
	case 5: /* jmp far */
		rc = em_jmp_far(ctxt);
		break;
	case 6:	/* push */
		rc = em_push(ctxt);
		break;
	}
	return rc;
}

static int em_cmpxchg8b(struct x86_emulate_ctxt *ctxt)
{
	u64 old = ctxt->dst.orig_val64;

	if (ctxt->dst.bytes == 16)
		return X86EMUL_UNHANDLEABLE;

	if (((u32) (old >> 0) != (u32) reg_read(ctxt, VCPU_REGS_RAX)) ||
	    ((u32) (old >> 32) != (u32) reg_read(ctxt, VCPU_REGS_RDX))) {
		*reg_write(ctxt, VCPU_REGS_RAX) = (u32) (old >> 0);
		*reg_write(ctxt, VCPU_REGS_RDX) = (u32) (old >> 32);
		ctxt->eflags &= ~EFLG_ZF;
	} else {
		ctxt->dst.val64 = ((u64)reg_read(ctxt, VCPU_REGS_RCX) << 32) |
			(u32) reg_read(ctxt, VCPU_REGS_RBX);

		ctxt->eflags |= EFLG_ZF;
	}
	return X86EMUL_CONTINUE;
}

static int em_ret(struct x86_emulate_ctxt *ctxt)
{
	int rc;
	unsigned long eip;

	rc = emulate_pop(ctxt, &eip, ctxt->op_bytes);
	if (rc != X86EMUL_CONTINUE)
		return rc;

	return assign_eip_near(ctxt, eip);
}

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
{
        int rc;

        rc = em_ret_far(ctxt);
        if (rc != X86EMUL_CONTINUE)
                return rc;
        rsp_increment(ctxt, ctxt->src.val);
        return X86EMUL_CONTINUE;
}

static int em_cmpxchg(struct x86_emulate_ctxt *ctxt)
{
	/* Save real source value, then compare EAX against destination. */
	ctxt->dst.orig_val = ctxt->dst.val;
	ctxt->dst.val = reg_read(ctxt, VCPU_REGS_RAX);
	ctxt->src.orig_val = ctxt->src.val;
	ctxt->src.val = ctxt->dst.orig_val;
	fastop(ctxt, em_cmp);

	if (ctxt->eflags & EFLG_ZF) {
		/* Success: write back to memory. */
		ctxt->dst.val = ctxt->src.orig_val;
	} else {
		/* Failure: write the value we saw to EAX. */
		ctxt->dst.type = OP_REG;
		ctxt->dst.addr.reg = reg_rmw(ctxt, VCPU_REGS_RAX);
		ctxt->dst.val = ctxt->dst.orig_val;
	}
	return X86EMUL_CONTINUE;
}

static int em_lseg(struct x86_emulate_ctxt *ctxt)
{
	int seg = ctxt->src2.val;
	unsigned short sel;
	int rc;

	memcpy(&sel, ctxt->src.valptr + ctxt->op_bytes, 2);

	rc = load_segment_descriptor(ctxt, sel, seg);
	if (rc != X86EMUL_CONTINUE)
		return rc;

	ctxt->dst.val = ctxt->src.val;
	return rc;
}

static void
setup_syscalls_segments(struct x86_emulate_ctxt *ctxt,
			struct desc_struct *cs, struct desc_struct *ss)
{
	cs->l = 0;		/* will be adjusted later */
	set_desc_base(cs, 0);	/* flat segment */
	cs->g = 1;		/* 4kb granularity */
	set_desc_limit(cs, 0xfffff);	/* 4GB limit */
	cs->type = 0x0b;	/* Read, Execute, Accessed */
	cs->s = 1;
	cs->dpl = 0;		/* will be adjusted later */
	cs->p = 1;
	cs->d = 1;
	cs->avl = 0;

	set_desc_base(ss, 0);	/* flat segment */
	set_desc_limit(ss, 0xfffff);	/* 4GB limit */
	ss->g = 1;		/* 4kb granularity */
	ss->s = 1;
	ss->type = 0x03;	/* Read/Write, Accessed */
	ss->d = 1;		/* 32bit stack segment */
	ss->dpl = 0;
	ss->p = 1;
	ss->l = 0;
	ss->avl = 0;
}

static bool vendor_intel(struct x86_emulate_ctxt *ctxt)
{
	u32 eax, ebx, ecx, edx;

	eax = ecx = 0;
	ctxt->ops->get_cpuid(ctxt, &eax, &ebx, &ecx, &edx);
	return ebx == X86EMUL_CPUID_VENDOR_GenuineIntel_ebx
		&& ecx == X86EMUL_CPUID_VENDOR_GenuineIntel_ecx
		&& edx == X86EMUL_CPUID_VENDOR_GenuineIntel_edx;
}

static bool em_syscall_is_enabled(struct x86_emulate_ctxt *ctxt)
{
	const struct x86_emulate_ops *ops = ctxt->ops;
	u32 eax, ebx, ecx, edx;

	/*
	 * syscall should always be enabled in longmode - so only become
	 * vendor specific (cpuid) if other modes are active...
	 */
	if (ctxt->mode == X86EMUL_MODE_PROT64)
		return true;

	eax = 0x00000000;
	ecx = 0x00000000;
	ops->get_cpuid(ctxt, &eax, &ebx, &ecx, &edx);
	/*
	 * Intel ("GenuineIntel")
	 * remark: Intel CPUs only support "syscall" in 64bit
	 * longmode. Also an 64bit guest with a
	 * 32bit compat-app running will #UD !! While this
	 * behaviour can be fixed (by emulating) into AMD
	 * response - CPUs of AMD can't behave like Intel.
	 */
	if (ebx == X86EMUL_CPUID_VENDOR_GenuineIntel_ebx &&
	    ecx == X86EMUL_CPUID_VENDOR_GenuineIntel_ecx &&
	    edx == X86EMUL_CPUID_VENDOR_GenuineIntel_edx)
		return false;

	/* AMD ("AuthenticAMD") */
	if (ebx == X86EMUL_CPUID_VENDOR_AuthenticAMD_ebx &&
	    ecx == X86EMUL_CPUID_VENDOR_AuthenticAMD_ecx &&
	    edx == X86EMUL_CPUID_VENDOR_AuthenticAMD_edx)
		return true;

	/* AMD ("AMDisbetter!") */
	if (ebx == X86EMUL_CPUID_VENDOR_AMDisbetterI_ebx &&
	    ecx == X86EMUL_CPUID_VENDOR_AMDisbetterI_ecx &&
	    edx == X86EMUL_CPUID_VENDOR_AMDisbetterI_edx)
		return true;

	/* default: (not Intel, not AMD), apply Intel's stricter rules... */
	return false;
}

static int em_syscall(struct x86_emulate_ctxt *ctxt)
{
	const struct x86_emulate_ops *ops = ctxt->ops;
	struct desc_struct cs, ss;
	u64 msr_data;
	u16 cs_sel, ss_sel;
	u64 efer = 0;

	/* syscall is not available in real mode */
	if (ctxt->mode == X86EMUL_MODE_REAL ||
	    ctxt->mode == X86EMUL_MODE_VM86)
		return emulate_ud(ctxt);

	if (!(em_syscall_is_enabled(ctxt)))
		return emulate_ud(ctxt);

	ops->get_msr(ctxt, MSR_EFER, &efer);
	setup_syscalls_segments(ctxt, &cs, &ss);

	if (!(efer & EFER_SCE))
		return emulate_ud(ctxt);

	ops->get_msr(ctxt, MSR_STAR, &msr_data);
	msr_data >>= 32;
	cs_sel = (u16)(msr_data & 0xfffc);
	ss_sel = (u16)(msr_data + 8);

	if (efer & EFER_LMA) {
		cs.d = 0;
		cs.l = 1;
	}
	ops->set_segment(ctxt, cs_sel, &cs, 0, VCPU_SREG_CS);
	ops->set_segment(ctxt, ss_sel, &ss, 0, VCPU_SREG_SS);

	*reg_write(ctxt, VCPU_REGS_RCX) = ctxt->_eip;
	if (efer & EFER_LMA) {
#ifdef CONFIG_X86_64
		*reg_write(ctxt, VCPU_REGS_R11) = ctxt->eflags;

		ops->get_msr(ctxt,
			     ctxt->mode == X86EMUL_MODE_PROT64 ?
			     MSR_LSTAR : MSR_CSTAR, &msr_data);
		ctxt->_eip = msr_data;

		ops->get_msr(ctxt, MSR_SYSCALL_MASK, &msr_data);
		ctxt->eflags &= ~msr_data;
#endif
	} else {
		/* legacy mode */
		ops->get_msr(ctxt, MSR_STAR, &msr_data);
		ctxt->_eip = (u32)msr_data;

		ctxt->eflags &= ~(EFLG_VM | EFLG_IF);
	}

	return X86EMUL_CONTINUE;
}

static int em_sysenter(struct x86_emulate_ctxt *ctxt)
{
	const struct x86_emulate_ops *ops = ctxt->ops;
	struct desc_struct cs, ss;
	u64 msr_data;
	u16 cs_sel, ss_sel;
	u64 efer = 0;

	ops->get_msr(ctxt, MSR_EFER, &efer);
	/* inject #GP if in real mode */
	if (ctxt->mode == X86EMUL_MODE_REAL)
		return emulate_gp(ctxt, 0);

	/*
	 * Not recognized on AMD in compat mode (but is recognized in legacy
	 * mode).
	 */
	if ((ctxt->mode == X86EMUL_MODE_PROT32) && (efer & EFER_LMA)
	    && !vendor_intel(ctxt))
		return emulate_ud(ctxt);

	/* XXX sysenter/sysexit have not been tested in 64bit mode.
	* Therefore, we inject an #UD.
	*/
	if (ctxt->mode == X86EMUL_MODE_PROT64)
		return emulate_ud(ctxt);

	setup_syscalls_segments(ctxt, &cs, &ss);

	ops->get_msr(ctxt, MSR_IA32_SYSENTER_CS, &msr_data);
	switch (ctxt->mode) {
	case X86EMUL_MODE_PROT32:
		if ((msr_data & 0xfffc) == 0x0)
			return emulate_gp(ctxt, 0);
		break;
	case X86EMUL_MODE_PROT64:
		if (msr_data == 0x0)
			return emulate_gp(ctxt, 0);
		break;
	default:
		break;
	}

	ctxt->eflags &= ~(EFLG_VM | EFLG_IF);
	cs_sel = (u16)msr_data;
	cs_sel &= ~SELECTOR_RPL_MASK;
	ss_sel = cs_sel + 8;
	ss_sel &= ~SELECTOR_RPL_MASK;
	if (ctxt->mode == X86EMUL_MODE_PROT64 || (efer & EFER_LMA)) {
		cs.d = 0;
		cs.l = 1;
	}

	ops->set_segment(ctxt, cs_sel, &cs, 0, VCPU_SREG_CS);
	ops->set_segment(ctxt, ss_sel, &ss, 0, VCPU_SREG_SS);

	ops->get_msr(ctxt, MSR_IA32_SYSENTER_EIP, &msr_data);
	ctxt->_eip = msr_data;

	ops->get_msr(ctxt, MSR_IA32_SYSENTER_ESP, &msr_data);
	*reg_write(ctxt, VCPU_REGS_RSP) = msr_data;

	return X86EMUL_CONTINUE;
}

static int em_sysexit(struct x86_emulate_ctxt *ctxt)
{
	const struct x86_emulate_ops *ops = ctxt->ops;
	struct desc_struct cs, ss;
	u64 msr_data, rcx, rdx;
	int usermode;
	u16 cs_sel = 0, ss_sel = 0;

	/* inject #GP if in real mode or Virtual 8086 mode */
	if (ctxt->mode == X86EMUL_MODE_REAL ||
	    ctxt->mode == X86EMUL_MODE_VM86)
		return emulate_gp(ctxt, 0);

	setup_syscalls_segments(ctxt, &cs, &ss);

	if ((ctxt->rex_prefix & 0x8) != 0x0)
		usermode = X86EMUL_MODE_PROT64;
	else
		usermode = X86EMUL_MODE_PROT32;

	rcx = reg_read(ctxt, VCPU_REGS_RCX);
	rdx = reg_read(ctxt, VCPU_REGS_RDX);

	cs.dpl = 3;
	ss.dpl = 3;
	ops->get_msr(ctxt, MSR_IA32_SYSENTER_CS, &msr_data);
	switch (usermode) {
	case X86EMUL_MODE_PROT32:
		cs_sel = (u16)(msr_data + 16);
		if ((msr_data & 0xfffc) == 0x0)
			return emulate_gp(ctxt, 0);
		ss_sel = (u16)(msr_data + 24);
		break;
	case X86EMUL_MODE_PROT64:
		cs_sel = (u16)(msr_data + 32);
		if (msr_data == 0x0)
			return emulate_gp(ctxt, 0);
		ss_sel = cs_sel + 8;
		cs.d = 0;
		cs.l = 1;
		if (is_noncanonical_address(rcx) ||
		    is_noncanonical_address(rdx))
			return emulate_gp(ctxt, 0);
		break;
	}
	cs_sel |= SELECTOR_RPL_MASK;
	ss_sel |= SELECTOR_RPL_MASK;

	ops->set_segment(ctxt, cs_sel, &cs, 0, VCPU_SREG_CS);
	ops->set_segment(ctxt, ss_sel, &ss, 0, VCPU_SREG_SS);

	ctxt->_eip = rdx;
	*reg_write(ctxt, VCPU_REGS_RSP) = rcx;

	return X86EMUL_CONTINUE;
}

static bool emulator_bad_iopl(struct x86_emulate_ctxt *ctxt)
{
	int iopl;
	if (ctxt->mode == X86EMUL_MODE_REAL)
		return false;
	if (ctxt->mode == X86EMUL_MODE_VM86)
		return true;
	iopl = (ctxt->eflags & X86_EFLAGS_IOPL) >> IOPL_SHIFT;
	return ctxt->ops->cpl(ctxt) > iopl;
}

static bool emulator_io_port_access_allowed(struct x86_emulate_ctxt *ctxt,
					    u16 port, u16 len)
{
	const struct x86_emulate_ops *ops = ctxt->ops;
	struct desc_struct tr_seg;
	u32 base3;
	int r;
	u16 tr, io_bitmap_ptr, perm, bit_idx = port & 0x7;
	unsigned mask = (1 << len) - 1;
	unsigned long base;

	ops->get_segment(ctxt, &tr, &tr_seg, &base3, VCPU_SREG_TR);
	if (!tr_seg.p)
		return false;
	if (desc_limit_scaled(&tr_seg) < 103)
		return false;
	base = get_desc_base(&tr_seg);
#ifdef CONFIG_X86_64
	base |= ((u64)base3) << 32;
#endif
	r = ops->read_std(ctxt, base + 102, &io_bitmap_ptr, 2, NULL);
	if (r != X86EMUL_CONTINUE)
		return false;
	if (io_bitmap_ptr + port/8 > desc_limit_scaled(&tr_seg))
		return false;
	r = ops->read_std(ctxt, base + io_bitmap_ptr + port/8, &perm, 2, NULL);
	if (r != X86EMUL_CONTINUE)
		return false;
	if ((perm >> bit_idx) & mask)
		return false;
	return true;
}

static bool emulator_io_permited(struct x86_emulate_ctxt *ctxt,
				 u16 port, u16 len)
{
	if (ctxt->perm_ok)
		return true;

	if (emulator_bad_iopl(ctxt))
		if (!emulator_io_port_access_allowed(ctxt, port, len))
			return false;

	ctxt->perm_ok = true;

	return true;
}

static void save_state_to_tss16(struct x86_emulate_ctxt *ctxt,
				struct tss_segment_16 *tss)
{
	tss->ip = ctxt->_eip;
	tss->flag = ctxt->eflags;
	tss->ax = reg_read(ctxt, VCPU_REGS_RAX);
	tss->cx = reg_read(ctxt, VCPU_REGS_RCX);
	tss->dx = reg_read(ctxt, VCPU_REGS_RDX);
	tss->bx = reg_read(ctxt, VCPU_REGS_RBX);
	tss->sp = reg_read(ctxt, VCPU_REGS_RSP);
	tss->bp = reg_read(ctxt, VCPU_REGS_RBP);
	tss->si = reg_read(ctxt, VCPU_REGS_RSI);
	tss->di = reg_read(ctxt, VCPU_REGS_RDI);

	tss->es = get_segment_selector(ctxt, VCPU_SREG_ES);
	tss->cs = get_segment_selector(ctxt, VCPU_SREG_CS);
	tss->ss = get_segment_selector(ctxt, VCPU_SREG_SS);
	tss->ds = get_segment_selector(ctxt, VCPU_SREG_DS);
	tss->ldt = get_segment_selector(ctxt, VCPU_SREG_LDTR);
}

static int load_state_from_tss16(struct x86_emulate_ctxt *ctxt,
				 struct tss_segment_16 *tss)
{
	int ret;
	u8 cpl;

	ctxt->_eip = tss->ip;
	ctxt->eflags = tss->flag | 2;
	*reg_write(ctxt, VCPU_REGS_RAX) = tss->ax;
	*reg_write(ctxt, VCPU_REGS_RCX) = tss->cx;
	*reg_write(ctxt, VCPU_REGS_RDX) = tss->dx;
	*reg_write(ctxt, VCPU_REGS_RBX) = tss->bx;
	*reg_write(ctxt, VCPU_REGS_RSP) = tss->sp;
	*reg_write(ctxt, VCPU_REGS_RBP) = tss->bp;
	*reg_write(ctxt, VCPU_REGS_RSI) = tss->si;
	*reg_write(ctxt, VCPU_REGS_RDI) = tss->di;

	/*
	 * SDM says that segment selectors are loaded before segment
	 * descriptors
	 */
	set_segment_selector(ctxt, tss->ldt, VCPU_SREG_LDTR);
	set_segment_selector(ctxt, tss->es, VCPU_SREG_ES);
	set_segment_selector(ctxt, tss->cs, VCPU_SREG_CS);
	set_segment_selector(ctxt, tss->ss, VCPU_SREG_SS);
	set_segment_selector(ctxt, tss->ds, VCPU_SREG_DS);

	cpl = tss->cs & 3;

	/*
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
}

static int task_switch_16(struct x86_emulate_ctxt *ctxt,
			  u16 tss_selector, u16 old_tss_sel,
			  ulong old_tss_base, struct desc_struct *new_desc)
{
	const struct x86_emulate_ops *ops = ctxt->ops;
	struct tss_segment_16 tss_seg;
	int ret;
	u32 new_tss_base = get_desc_base(new_desc);

	ret = ops->read_std(ctxt, old_tss_base, &tss_seg, sizeof tss_seg,
			    &ctxt->exception);
	if (ret != X86EMUL_CONTINUE)
		/* FIXME: need to provide precise fault address */
		return ret;

	save_state_to_tss16(ctxt, &tss_seg);

	ret = ops->write_std(ctxt, old_tss_base, &tss_seg, sizeof tss_seg,
			     &ctxt->exception);
	if (ret != X86EMUL_CONTINUE)
		/* FIXME: need to provide precise fault address */
		return ret;

	ret = ops->read_std(ctxt, new_tss_base, &tss_seg, sizeof tss_seg,
			    &ctxt->exception);
	if (ret != X86EMUL_CONTINUE)
		/* FIXME: need to provide precise fault address */
		return ret;

	if (old_tss_sel != 0xffff) {
		tss_seg.prev_task_link = old_tss_sel;

		ret = ops->write_std(ctxt, new_tss_base,
				     &tss_seg.prev_task_link,
				     sizeof tss_seg.prev_task_link,
				     &ctxt->exception);
		if (ret != X86EMUL_CONTINUE)
			/* FIXME: need to provide precise fault address */
			return ret;
	}

	return load_state_from_tss16(ctxt, &tss_seg);
}

static void save_state_to_tss32(struct x86_emulate_ctxt *ctxt,
				struct tss_segment_32 *tss)
{
	/* CR3 and ldt selector are not saved intentionally */
	tss->eip = ctxt->_eip;
	tss->eflags = ctxt->eflags;
	tss->eax = reg_read(ctxt, VCPU_REGS_RAX);
	tss->ecx = reg_read(ctxt, VCPU_REGS_RCX);
	tss->edx = reg_read(ctxt, VCPU_REGS_RDX);
	tss->ebx = reg_read(ctxt, VCPU_REGS_RBX);
	tss->esp = reg_read(ctxt, VCPU_REGS_RSP);
	tss->ebp = reg_read(ctxt, VCPU_REGS_RBP);
	tss->esi = reg_read(ctxt, VCPU_REGS_RSI);
	tss->edi = reg_read(ctxt, VCPU_REGS_RDI);

	tss->es = get_segment_selector(ctxt, VCPU_SREG_ES);
	tss->cs = get_segment_selector(ctxt, VCPU_SREG_CS);
	tss->ss = get_segment_selector(ctxt, VCPU_SREG_SS);
	tss->ds = get_segment_selector(ctxt, VCPU_SREG_DS);
	tss->fs = get_segment_selector(ctxt, VCPU_SREG_FS);
	tss->gs = get_segment_selector(ctxt, VCPU_SREG_GS);
}

static int load_state_from_tss32(struct x86_emulate_ctxt *ctxt,
				 struct tss_segment_32 *tss)
{
	int ret;
	u8 cpl;

	if (ctxt->ops->set_cr(ctxt, 3, tss->cr3))
		return emulate_gp(ctxt, 0);
	ctxt->_eip = tss->eip;
	ctxt->eflags = tss->eflags | 2;

	/* General purpose registers */
	*reg_write(ctxt, VCPU_REGS_RAX) = tss->eax;
	*reg_write(ctxt, VCPU_REGS_RCX) = tss->ecx;
	*reg_write(ctxt, VCPU_REGS_RDX) = tss->edx;
	*reg_write(ctxt, VCPU_REGS_RBX) = tss->ebx;
	*reg_write(ctxt, VCPU_REGS_RSP) = tss->esp;
	*reg_write(ctxt, VCPU_REGS_RBP) = tss->ebp;
	*reg_write(ctxt, VCPU_REGS_RSI) = tss->esi;
	*reg_write(ctxt, VCPU_REGS_RDI) = tss->edi;

	/*
	 * SDM says that segment selectors are loaded before segment
	 * descriptors.  This is important because CPL checks will
	 * use CS.RPL.
	 */
	set_segment_selector(ctxt, tss->ldt_selector, VCPU_SREG_LDTR);
	set_segment_selector(ctxt, tss->es, VCPU_SREG_ES);
	set_segment_selector(ctxt, tss->cs, VCPU_SREG_CS);
	set_segment_selector(ctxt, tss->ss, VCPU_SREG_SS);
	set_segment_selector(ctxt, tss->ds, VCPU_SREG_DS);
	set_segment_selector(ctxt, tss->fs, VCPU_SREG_FS);
	set_segment_selector(ctxt, tss->gs, VCPU_SREG_GS);

	/*
	 * If we're switching between Protected Mode and VM86, we need to make
	 * sure to update the mode before loading the segment descriptors so
	 * that the selectors are interpreted correctly.
	 */
	if (ctxt->eflags & X86_EFLAGS_VM) {
		ctxt->mode = X86EMUL_MODE_VM86;
		cpl = 3;
	} else {
		ctxt->mode = X86EMUL_MODE_PROT32;
		cpl = tss->cs & 3;
	}

	/*
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
}

static int task_switch_32(struct x86_emulate_ctxt *ctxt,
			  u16 tss_selector, u16 old_tss_sel,
			  ulong old_tss_base, struct desc_struct *new_desc)
{
	const struct x86_emulate_ops *ops = ctxt->ops;
	struct tss_segment_32 tss_seg;
	int ret;
	u32 new_tss_base = get_desc_base(new_desc);
	u32 eip_offset = offsetof(struct tss_segment_32, eip);
	u32 ldt_sel_offset = offsetof(struct tss_segment_32, ldt_selector);

	ret = ops->read_std(ctxt, old_tss_base, &tss_seg, sizeof tss_seg,
			    &ctxt->exception);
	if (ret != X86EMUL_CONTINUE)
		/* FIXME: need to provide precise fault address */
		return ret;

	save_state_to_tss32(ctxt, &tss_seg);

	/* Only GP registers and segment selectors are saved */
	ret = ops->write_std(ctxt, old_tss_base + eip_offset, &tss_seg.eip,
			     ldt_sel_offset - eip_offset, &ctxt->exception);
	if (ret != X86EMUL_CONTINUE)
		/* FIXME: need to provide precise fault address */
		return ret;

	ret = ops->read_std(ctxt, new_tss_base, &tss_seg, sizeof tss_seg,
			    &ctxt->exception);
	if (ret != X86EMUL_CONTINUE)
		/* FIXME: need to provide precise fault address */
		return ret;

	if (old_tss_sel != 0xffff) {
		tss_seg.prev_task_link = old_tss_sel;

		ret = ops->write_std(ctxt, new_tss_base,
				     &tss_seg.prev_task_link,
				     sizeof tss_seg.prev_task_link,
				     &ctxt->exception);
		if (ret != X86EMUL_CONTINUE)
			/* FIXME: need to provide precise fault address */
			return ret;
	}

	return load_state_from_tss32(ctxt, &tss_seg);
}

static int emulator_do_task_switch(struct x86_emulate_ctxt *ctxt,
				   u16 tss_selector, int idt_index, int reason,
				   bool has_error_code, u32 error_code)
{
	const struct x86_emulate_ops *ops = ctxt->ops;
	struct desc_struct curr_tss_desc, next_tss_desc;
	int ret;
	u16 old_tss_sel = get_segment_selector(ctxt, VCPU_SREG_TR);
	ulong old_tss_base =
		ops->get_cached_segment_base(ctxt, VCPU_SREG_TR);
	u32 desc_limit;
	ulong desc_addr;

	/* FIXME: old_tss_base == ~0 ? */

	ret = read_segment_descriptor(ctxt, tss_selector, &next_tss_desc, &desc_addr);
	if (ret != X86EMUL_CONTINUE)
		return ret;
	ret = read_segment_descriptor(ctxt, old_tss_sel, &curr_tss_desc, &desc_addr);
	if (ret != X86EMUL_CONTINUE)
		return ret;

	/* FIXME: check that next_tss_desc is tss */

	/*
	 * Check privileges. The three cases are task switch caused by...
	 *
	 * 1. jmp/call/int to task gate: Check against DPL of the task gate
	 * 2. Exception/IRQ/iret: No check is performed
	 * 3. jmp/call to TSS: Check against DPL of the TSS
	 */
	if (reason == TASK_SWITCH_GATE) {
		if (idt_index != -1) {
			/* Software interrupts */
			struct desc_struct task_gate_desc;
			int dpl;

			ret = read_interrupt_descriptor(ctxt, idt_index,
							&task_gate_desc);
			if (ret != X86EMUL_CONTINUE)
				return ret;

			dpl = task_gate_desc.dpl;
			if ((tss_selector & 3) > dpl || ops->cpl(ctxt) > dpl)
				return emulate_gp(ctxt, (idt_index << 3) | 0x2);
		}
	} else if (reason != TASK_SWITCH_IRET) {
		int dpl = next_tss_desc.dpl;
		if ((tss_selector & 3) > dpl || ops->cpl(ctxt) > dpl)
			return emulate_gp(ctxt, tss_selector);
	}


	desc_limit = desc_limit_scaled(&next_tss_desc);
	if (!next_tss_desc.p ||
	    ((desc_limit < 0x67 && (next_tss_desc.type & 8)) ||
	     desc_limit < 0x2b)) {
		return emulate_ts(ctxt, tss_selector & 0xfffc);
	}

	if (reason == TASK_SWITCH_IRET || reason == TASK_SWITCH_JMP) {
		curr_tss_desc.type &= ~(1 << 1); /* clear busy flag */
		write_segment_descriptor(ctxt, old_tss_sel, &curr_tss_desc);
	}

	if (reason == TASK_SWITCH_IRET)
		ctxt->eflags = ctxt->eflags & ~X86_EFLAGS_NT;

	/* set back link to prev task only if NT bit is set in eflags
	   note that old_tss_sel is not used after this point */
	if (reason != TASK_SWITCH_CALL && reason != TASK_SWITCH_GATE)
		old_tss_sel = 0xffff;

	if (next_tss_desc.type & 8)
		ret = task_switch_32(ctxt, tss_selector, old_tss_sel,
				     old_tss_base, &next_tss_desc);
	else
		ret = task_switch_16(ctxt, tss_selector, old_tss_sel,
				     old_tss_base, &next_tss_desc);
	if (ret != X86EMUL_CONTINUE)
		return ret;

	if (reason == TASK_SWITCH_CALL || reason == TASK_SWITCH_GATE)
		ctxt->eflags = ctxt->eflags | X86_EFLAGS_NT;

	if (reason != TASK_SWITCH_IRET) {
		next_tss_desc.type |= (1 << 1); /* set busy flag */
		write_segment_descriptor(ctxt, tss_selector, &next_tss_desc);
	}

	ops->set_cr(ctxt, 0,  ops->get_cr(ctxt, 0) | X86_CR0_TS);
	ops->set_segment(ctxt, tss_selector, &next_tss_desc, 0, VCPU_SREG_TR);

	if (has_error_code) {
		ctxt->op_bytes = ctxt->ad_bytes = (next_tss_desc.type & 8) ? 4 : 2;
		ctxt->lock_prefix = 0;
		ctxt->src.val = (unsigned long) error_code;
		ret = em_push(ctxt);
	}

	return ret;
}

int emulator_task_switch(struct x86_emulate_ctxt *ctxt,
			 u16 tss_selector, int idt_index, int reason,
			 bool has_error_code, u32 error_code)
{
	int rc;

	invalidate_registers(ctxt);
	ctxt->_eip = ctxt->eip;
	ctxt->dst.type = OP_NONE;

	rc = emulator_do_task_switch(ctxt, tss_selector, idt_index, reason,
				     has_error_code, error_code);

	if (rc == X86EMUL_CONTINUE) {
		ctxt->eip = ctxt->_eip;
		writeback_registers(ctxt);
	}

	return (rc == X86EMUL_UNHANDLEABLE) ? EMULATION_FAILED : EMULATION_OK;
}

static void string_addr_inc(struct x86_emulate_ctxt *ctxt, int reg,
		struct operand *op)
{
	int df = (ctxt->eflags & EFLG_DF) ? -op->count : op->count;

	register_address_increment(ctxt, reg_rmw(ctxt, reg), df * op->bytes);
	op->addr.mem.ea = register_address(ctxt, reg_read(ctxt, reg));
}

static int em_das(struct x86_emulate_ctxt *ctxt)
{
	u8 al, old_al;
	bool af, cf, old_cf;

	cf = ctxt->eflags & X86_EFLAGS_CF;
	al = ctxt->dst.val;

	old_al = al;
	old_cf = cf;
	cf = false;
	af = ctxt->eflags & X86_EFLAGS_AF;
	if ((al & 0x0f) > 9 || af) {
		al -= 6;
		cf = old_cf | (al >= 250);
		af = true;
	} else {
		af = false;
	}
	if (old_al > 0x99 || old_cf) {
		al -= 0x60;
		cf = true;
	}

	ctxt->dst.val = al;
	/* Set PF, ZF, SF */
	ctxt->src.type = OP_IMM;
	ctxt->src.val = 0;
	ctxt->src.bytes = 1;
	fastop(ctxt, em_or);
	ctxt->eflags &= ~(X86_EFLAGS_AF | X86_EFLAGS_CF);
	if (cf)
		ctxt->eflags |= X86_EFLAGS_CF;
	if (af)
		ctxt->eflags |= X86_EFLAGS_AF;
	return X86EMUL_CONTINUE;
}

static int em_aam(struct x86_emulate_ctxt *ctxt)
{
	u8 al, ah;

	if (ctxt->src.val == 0)
		return emulate_de(ctxt);

	al = ctxt->dst.val & 0xff;
	ah = al / ctxt->src.val;
	al %= ctxt->src.val;

	ctxt->dst.val = (ctxt->dst.val & 0xffff0000) | al | (ah << 8);

	/* Set PF, ZF, SF */
	ctxt->src.type = OP_IMM;
	ctxt->src.val = 0;
	ctxt->src.bytes = 1;
	fastop(ctxt, em_or);

	return X86EMUL_CONTINUE;
}

static int em_aad(struct x86_emulate_ctxt *ctxt)
{
	u8 al = ctxt->dst.val & 0xff;
	u8 ah = (ctxt->dst.val >> 8) & 0xff;

	al = (al + (ah * ctxt->src.val)) & 0xff;

	ctxt->dst.val = (ctxt->dst.val & 0xffff0000) | al;

	/* Set PF, ZF, SF */
	ctxt->src.type = OP_IMM;
	ctxt->src.val = 0;
	ctxt->src.bytes = 1;
	fastop(ctxt, em_or);

	return X86EMUL_CONTINUE;
}

static int em_call(struct x86_emulate_ctxt *ctxt)
{
	int rc;
	long rel = ctxt->src.val;

	ctxt->src.val = (unsigned long)ctxt->_eip;
	rc = jmp_rel(ctxt, rel);
	if (rc != X86EMUL_CONTINUE)
		return rc;
	return em_push(ctxt);
}

static int em_call_far(struct x86_emulate_ctxt *ctxt)
{
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
	int rc;
	unsigned long eip;

	rc = emulate_pop(ctxt, &eip, ctxt->op_bytes);
	if (rc != X86EMUL_CONTINUE)
		return rc;
	rc = assign_eip_near(ctxt, eip);
	if (rc != X86EMUL_CONTINUE)
		return rc;
	rsp_increment(ctxt, ctxt->src.val);
	return X86EMUL_CONTINUE;
}

static int em_xchg(struct x86_emulate_ctxt *ctxt)
{
	/* Write back the register source. */
	ctxt->src.val = ctxt->dst.val;
	write_register_operand(&ctxt->src);

	/* Write back the memory destination with implicit LOCK prefix. */
	ctxt->dst.val = ctxt->src.orig_val;
	ctxt->lock_prefix = 1;
	return X86EMUL_CONTINUE;
}

static int em_imul_3op(struct x86_emulate_ctxt *ctxt)
{
	ctxt->dst.val = ctxt->src2.val;
	return fastop(ctxt, em_imul);
}

static int em_cwd(struct x86_emulate_ctxt *ctxt)
{
	ctxt->dst.type = OP_REG;
	ctxt->dst.bytes = ctxt->src.bytes;
	ctxt->dst.addr.reg = reg_rmw(ctxt, VCPU_REGS_RDX);
	ctxt->dst.val = ~((ctxt->src.val >> (ctxt->src.bytes * 8 - 1)) - 1);

	return X86EMUL_CONTINUE;
}

static int em_rdtsc(struct x86_emulate_ctxt *ctxt)
{
	u64 tsc = 0;

	ctxt->ops->get_msr(ctxt, MSR_IA32_TSC, &tsc);
	*reg_write(ctxt, VCPU_REGS_RAX) = (u32)tsc;
	*reg_write(ctxt, VCPU_REGS_RDX) = tsc >> 32;
	return X86EMUL_CONTINUE;
}

static int em_rdpmc(struct x86_emulate_ctxt *ctxt)
{
	u64 pmc;

	if (ctxt->ops->read_pmc(ctxt, reg_read(ctxt, VCPU_REGS_RCX), &pmc))
		return emulate_gp(ctxt, 0);
	*reg_write(ctxt, VCPU_REGS_RAX) = (u32)pmc;
	*reg_write(ctxt, VCPU_REGS_RDX) = pmc >> 32;
	return X86EMUL_CONTINUE;
}

static int em_mov(struct x86_emulate_ctxt *ctxt)
{
	memcpy(ctxt->dst.valptr, ctxt->src.valptr, sizeof(ctxt->src.valptr));
	return X86EMUL_CONTINUE;
}

#define FFL(x) bit(X86_FEATURE_##x)

static int em_movbe(struct x86_emulate_ctxt *ctxt)
{
	u32 ebx, ecx, edx, eax = 1;
	u16 tmp;

	/*
	 * Check MOVBE is set in the guest-visible CPUID leaf.
	 */
	ctxt->ops->get_cpuid(ctxt, &eax, &ebx, &ecx, &edx);
	if (!(ecx & FFL(MOVBE)))
		return emulate_ud(ctxt);

	switch (ctxt->op_bytes) {
	case 2:
		/*
		 * From MOVBE definition: "...When the operand size is 16 bits,
		 * the upper word of the destination register remains unchanged
		 * ..."
		 *
		 * Both casting ->valptr and ->val to u16 breaks strict aliasing
		 * rules so we have to do the operation almost per hand.
		 */
		tmp = (u16)ctxt->src.val;
		ctxt->dst.val &= ~0xffffUL;
		ctxt->dst.val |= (unsigned long)swab16(tmp);
		break;
	case 4:
		ctxt->dst.val = swab32((u32)ctxt->src.val);
		break;
	case 8:
		ctxt->dst.val = swab64(ctxt->src.val);
		break;
	default:
		BUG();
	}
	return X86EMUL_CONTINUE;
}

static int em_cr_write(struct x86_emulate_ctxt *ctxt)
{
	if (ctxt->ops->set_cr(ctxt, ctxt->modrm_reg, ctxt->src.val))
		return emulate_gp(ctxt, 0);

	/* Disable writeback. */
	ctxt->dst.type = OP_NONE;
	return X86EMUL_CONTINUE;
}

static int em_dr_write(struct x86_emulate_ctxt *ctxt)
{
	unsigned long val;

	if (ctxt->mode == X86EMUL_MODE_PROT64)
		val = ctxt->src.val & ~0ULL;
	else
		val = ctxt->src.val & ~0U;

	/* #UD condition is already handled. */
	if (ctxt->ops->set_dr(ctxt, ctxt->modrm_reg, val) < 0)
		return emulate_gp(ctxt, 0);

	/* Disable writeback. */
	ctxt->dst.type = OP_NONE;
	return X86EMUL_CONTINUE;
}

static int em_wrmsr(struct x86_emulate_ctxt *ctxt)
{
	u64 msr_data;

	msr_data = (u32)reg_read(ctxt, VCPU_REGS_RAX)
		| ((u64)reg_read(ctxt, VCPU_REGS_RDX) << 32);
	if (ctxt->ops->set_msr(ctxt, reg_read(ctxt, VCPU_REGS_RCX), msr_data))
		return emulate_gp(ctxt, 0);

	return X86EMUL_CONTINUE;
}

static int em_rdmsr(struct x86_emulate_ctxt *ctxt)
{
	u64 msr_data;

	if (ctxt->ops->get_msr(ctxt, reg_read(ctxt, VCPU_REGS_RCX), &msr_data))
		return emulate_gp(ctxt, 0);

	*reg_write(ctxt, VCPU_REGS_RAX) = (u32)msr_data;
	*reg_write(ctxt, VCPU_REGS_RDX) = msr_data >> 32;
	return X86EMUL_CONTINUE;
}

static int em_mov_rm_sreg(struct x86_emulate_ctxt *ctxt)
{
	if (ctxt->modrm_reg > VCPU_SREG_GS)
		return emulate_ud(ctxt);

	ctxt->dst.val = get_segment_selector(ctxt, ctxt->modrm_reg);
	return X86EMUL_CONTINUE;
}

static int em_mov_sreg_rm(struct x86_emulate_ctxt *ctxt)
{
	u16 sel = ctxt->src.val;

	if (ctxt->modrm_reg == VCPU_SREG_CS || ctxt->modrm_reg > VCPU_SREG_GS)
		return emulate_ud(ctxt);

	if (ctxt->modrm_reg == VCPU_SREG_SS)
		ctxt->interruptibility = KVM_X86_SHADOW_INT_MOV_SS;

	/* Disable writeback. */
	ctxt->dst.type = OP_NONE;
	return load_segment_descriptor(ctxt, sel, ctxt->modrm_reg);
}

static int em_lldt(struct x86_emulate_ctxt *ctxt)
{
	u16 sel = ctxt->src.val;

	/* Disable writeback. */
	ctxt->dst.type = OP_NONE;
	return load_segment_descriptor(ctxt, sel, VCPU_SREG_LDTR);
}

static int em_ltr(struct x86_emulate_ctxt *ctxt)
{
	u16 sel = ctxt->src.val;

	/* Disable writeback. */
	ctxt->dst.type = OP_NONE;
	return load_segment_descriptor(ctxt, sel, VCPU_SREG_TR);
}

static int em_invlpg(struct x86_emulate_ctxt *ctxt)
{
	int rc;
	ulong linear;

	rc = linearize(ctxt, ctxt->src.addr.mem, 1, false, &linear);
	if (rc == X86EMUL_CONTINUE)
		ctxt->ops->invlpg(ctxt, linear);
	/* Disable writeback. */
	ctxt->dst.type = OP_NONE;
	return X86EMUL_CONTINUE;
}

static int em_clts(struct x86_emulate_ctxt *ctxt)
{
	ulong cr0;

	cr0 = ctxt->ops->get_cr(ctxt, 0);
	cr0 &= ~X86_CR0_TS;
	ctxt->ops->set_cr(ctxt, 0, cr0);
	return X86EMUL_CONTINUE;
}

static int em_vmcall(struct x86_emulate_ctxt *ctxt)
{
	int rc = ctxt->ops->fix_hypercall(ctxt);

	if (rc != X86EMUL_CONTINUE)
		return rc;

	/* Let the processor re-execute the fixed hypercall */
	ctxt->_eip = ctxt->eip;
	/* Disable writeback. */
	ctxt->dst.type = OP_NONE;
	return X86EMUL_CONTINUE;
}

static int emulate_store_desc_ptr(struct x86_emulate_ctxt *ctxt,
				  void (*get)(struct x86_emulate_ctxt *ctxt,
					      struct desc_ptr *ptr))
{
	struct desc_ptr desc_ptr;

	if (ctxt->mode == X86EMUL_MODE_PROT64)
		ctxt->op_bytes = 8;
	get(ctxt, &desc_ptr);
	if (ctxt->op_bytes == 2) {
		ctxt->op_bytes = 4;
		desc_ptr.address &= 0x00ffffff;
	}
	/* Disable writeback. */
	ctxt->dst.type = OP_NONE;
	return segmented_write(ctxt, ctxt->dst.addr.mem,
			       &desc_ptr, 2 + ctxt->op_bytes);
}

static int em_sgdt(struct x86_emulate_ctxt *ctxt)
{
	return emulate_store_desc_ptr(ctxt, ctxt->ops->get_gdt);
}

static int em_sidt(struct x86_emulate_ctxt *ctxt)
{
	return emulate_store_desc_ptr(ctxt, ctxt->ops->get_idt);
}

static int em_lgdt(struct x86_emulate_ctxt *ctxt)
{
	struct desc_ptr desc_ptr;
	int rc;

	if (ctxt->mode == X86EMUL_MODE_PROT64)
		ctxt->op_bytes = 8;
	rc = read_descriptor(ctxt, ctxt->src.addr.mem,
			     &desc_ptr.size, &desc_ptr.address,
			     ctxt->op_bytes);
	if (rc != X86EMUL_CONTINUE)
		return rc;
	ctxt->ops->set_gdt(ctxt, &desc_ptr);
	/* Disable writeback. */
	ctxt->dst.type = OP_NONE;
	return X86EMUL_CONTINUE;
}

static int em_vmmcall(struct x86_emulate_ctxt *ctxt)
{
	int rc;

	rc = ctxt->ops->fix_hypercall(ctxt);

	/* Disable writeback. */
	ctxt->dst.type = OP_NONE;
	return rc;
}

static int em_lidt(struct x86_emulate_ctxt *ctxt)
{
	struct desc_ptr desc_ptr;
	int rc;

	if (ctxt->mode == X86EMUL_MODE_PROT64)
		ctxt->op_bytes = 8;
	rc = read_descriptor(ctxt, ctxt->src.addr.mem,
			     &desc_ptr.size, &desc_ptr.address,
			     ctxt->op_bytes);
	if (rc != X86EMUL_CONTINUE)
		return rc;
	ctxt->ops->set_idt(ctxt, &desc_ptr);
	/* Disable writeback. */
	ctxt->dst.type = OP_NONE;
	return X86EMUL_CONTINUE;
}

static int em_smsw(struct x86_emulate_ctxt *ctxt)
{
	if (ctxt->dst.type == OP_MEM)
		ctxt->dst.bytes = 2;
	ctxt->dst.val = ctxt->ops->get_cr(ctxt, 0);
	return X86EMUL_CONTINUE;
}

static int em_lmsw(struct x86_emulate_ctxt *ctxt)
{
	ctxt->ops->set_cr(ctxt, 0, (ctxt->ops->get_cr(ctxt, 0) & ~0x0eul)
			  | (ctxt->src.val & 0x0f));
	ctxt->dst.type = OP_NONE;
	return X86EMUL_CONTINUE;
}

static int em_loop(struct x86_emulate_ctxt *ctxt)
{
	int rc = X86EMUL_CONTINUE;

	register_address_increment(ctxt, reg_rmw(ctxt, VCPU_REGS_RCX), -1);
	if ((address_mask(ctxt, reg_read(ctxt, VCPU_REGS_RCX)) != 0) &&
	    (ctxt->b == 0xe2 || test_cc(ctxt->b ^ 0x5, ctxt->eflags)))
		rc = jmp_rel(ctxt, ctxt->src.val);

	return rc;
}

static int em_jcxz(struct x86_emulate_ctxt *ctxt)
{
	int rc = X86EMUL_CONTINUE;

	if (address_mask(ctxt, reg_read(ctxt, VCPU_REGS_RCX)) == 0)
		rc = jmp_rel(ctxt, ctxt->src.val);

	return rc;
}

static int em_in(struct x86_emulate_ctxt *ctxt)
{
	if (!pio_in_emulated(ctxt, ctxt->dst.bytes, ctxt->src.val,
			     &ctxt->dst.val))
		return X86EMUL_IO_NEEDED;

	return X86EMUL_CONTINUE;
}

static int em_out(struct x86_emulate_ctxt *ctxt)
{
	ctxt->ops->pio_out_emulated(ctxt, ctxt->src.bytes, ctxt->dst.val,
				    &ctxt->src.val, 1);
	/* Disable writeback. */
	ctxt->dst.type = OP_NONE;
	return X86EMUL_CONTINUE;
}

static int em_cli(struct x86_emulate_ctxt *ctxt)
{
	if (emulator_bad_iopl(ctxt))
		return emulate_gp(ctxt, 0);

	ctxt->eflags &= ~X86_EFLAGS_IF;
	return X86EMUL_CONTINUE;
}

static int em_sti(struct x86_emulate_ctxt *ctxt)
{
	if (emulator_bad_iopl(ctxt))
		return emulate_gp(ctxt, 0);

	ctxt->interruptibility = KVM_X86_SHADOW_INT_STI;
	ctxt->eflags |= X86_EFLAGS_IF;
	return X86EMUL_CONTINUE;
}

static int em_cpuid(struct x86_emulate_ctxt *ctxt)
{
	u32 eax, ebx, ecx, edx;

	eax = reg_read(ctxt, VCPU_REGS_RAX);
	ecx = reg_read(ctxt, VCPU_REGS_RCX);
	ctxt->ops->get_cpuid(ctxt, &eax, &ebx, &ecx, &edx);
	*reg_write(ctxt, VCPU_REGS_RAX) = eax;
	*reg_write(ctxt, VCPU_REGS_RBX) = ebx;
	*reg_write(ctxt, VCPU_REGS_RCX) = ecx;
	*reg_write(ctxt, VCPU_REGS_RDX) = edx;
	return X86EMUL_CONTINUE;
}

static int em_sahf(struct x86_emulate_ctxt *ctxt)
{
	u32 flags;

	flags = EFLG_CF | EFLG_PF | EFLG_AF | EFLG_ZF | EFLG_SF;
	flags &= *reg_rmw(ctxt, VCPU_REGS_RAX) >> 8;

	ctxt->eflags &= ~0xffUL;
	ctxt->eflags |= flags | X86_EFLAGS_FIXED;
	return X86EMUL_CONTINUE;
}

static int em_lahf(struct x86_emulate_ctxt *ctxt)
{
	*reg_rmw(ctxt, VCPU_REGS_RAX) &= ~0xff00UL;
	*reg_rmw(ctxt, VCPU_REGS_RAX) |= (ctxt->eflags & 0xff) << 8;
	return X86EMUL_CONTINUE;
}

static int em_bswap(struct x86_emulate_ctxt *ctxt)
{
	switch (ctxt->op_bytes) {
#ifdef CONFIG_X86_64
	case 8:
		asm("bswap %0" : "+r"(ctxt->dst.val));
		break;
#endif
	default:
		asm("bswap %0" : "+r"(*(u32 *)&ctxt->dst.val));
		break;
	}
	return X86EMUL_CONTINUE;
}

static bool valid_cr(int nr)
{
	switch (nr) {
	case 0:
	case 2 ... 4:
	case 8:
		return true;
	default:
		return false;
	}
}

static int check_cr_read(struct x86_emulate_ctxt *ctxt)
{
	if (!valid_cr(ctxt->modrm_reg))
		return emulate_ud(ctxt);

	return X86EMUL_CONTINUE;
}

static int check_cr_write(struct x86_emulate_ctxt *ctxt)
{
	u64 new_val = ctxt->src.val64;
	int cr = ctxt->modrm_reg;
	u64 efer = 0;

	static u64 cr_reserved_bits[] = {
		0xffffffff00000000ULL,
		0, 0, 0, /* CR3 checked later */
		CR4_RESERVED_BITS,
		0, 0, 0,
		CR8_RESERVED_BITS,
	};

	if (!valid_cr(cr))
		return emulate_ud(ctxt);

	if (new_val & cr_reserved_bits[cr])
		return emulate_gp(ctxt, 0);

	switch (cr) {
	case 0: {
		u64 cr4;
		if (((new_val & X86_CR0_PG) && !(new_val & X86_CR0_PE)) ||
		    ((new_val & X86_CR0_NW) && !(new_val & X86_CR0_CD)))
			return emulate_gp(ctxt, 0);

		cr4 = ctxt->ops->get_cr(ctxt, 4);
		ctxt->ops->get_msr(ctxt, MSR_EFER, &efer);

		if ((new_val & X86_CR0_PG) && (efer & EFER_LME) &&
		    !(cr4 & X86_CR4_PAE))
			return emulate_gp(ctxt, 0);

		break;
		}
	case 3: {
		u64 rsvd = 0;

		ctxt->ops->get_msr(ctxt, MSR_EFER, &efer);
		if (efer & EFER_LMA)
			rsvd = CR3_L_MODE_RESERVED_BITS;

		if (new_val & rsvd)
			return emulate_gp(ctxt, 0);

		break;
		}
	case 4: {
		ctxt->ops->get_msr(ctxt, MSR_EFER, &efer);

		if ((efer & EFER_LMA) && !(new_val & X86_CR4_PAE))
			return emulate_gp(ctxt, 0);

		break;
		}
	}

	return X86EMUL_CONTINUE;
}

static int check_dr7_gd(struct x86_emulate_ctxt *ctxt)
{
	unsigned long dr7;

	ctxt->ops->get_dr(ctxt, 7, &dr7);

	/* Check if DR7.Global_Enable is set */
	return dr7 & (1 << 13);
}

static int check_dr_read(struct x86_emulate_ctxt *ctxt)
{
	int dr = ctxt->modrm_reg;
	u64 cr4;

	if (dr > 7)
		return emulate_ud(ctxt);

	cr4 = ctxt->ops->get_cr(ctxt, 4);
	if ((cr4 & X86_CR4_DE) && (dr == 4 || dr == 5))
		return emulate_ud(ctxt);

	if (check_dr7_gd(ctxt))
		return emulate_db(ctxt);

	return X86EMUL_CONTINUE;
}

static int check_dr_write(struct x86_emulate_ctxt *ctxt)
{
	u64 new_val = ctxt->src.val64;
	int dr = ctxt->modrm_reg;

	if ((dr == 6 || dr == 7) && (new_val & 0xffffffff00000000ULL))
		return emulate_gp(ctxt, 0);

	return check_dr_read(ctxt);
}

static int check_svme(struct x86_emulate_ctxt *ctxt)
{
	u64 efer;

	ctxt->ops->get_msr(ctxt, MSR_EFER, &efer);

	if (!(efer & EFER_SVME))
		return emulate_ud(ctxt);

	return X86EMUL_CONTINUE;
}

static int check_svme_pa(struct x86_emulate_ctxt *ctxt)
{
	u64 rax = reg_read(ctxt, VCPU_REGS_RAX);

	/* Valid physical address? */
	if (rax & 0xffff000000000000ULL)
		return emulate_gp(ctxt, 0);

	return check_svme(ctxt);
}

static int check_rdtsc(struct x86_emulate_ctxt *ctxt)
{
	u64 cr4 = ctxt->ops->get_cr(ctxt, 4);

	if (cr4 & X86_CR4_TSD && ctxt->ops->cpl(ctxt))
		return emulate_ud(ctxt);

	return X86EMUL_CONTINUE;
}

static int check_rdpmc(struct x86_emulate_ctxt *ctxt)
{
	u64 cr4 = ctxt->ops->get_cr(ctxt, 4);
	u64 rcx = reg_read(ctxt, VCPU_REGS_RCX);

	if ((!(cr4 & X86_CR4_PCE) && ctxt->ops->cpl(ctxt)) ||
	    ctxt->ops->check_pmc(ctxt, rcx))
		return emulate_gp(ctxt, 0);

	return X86EMUL_CONTINUE;
}

static int check_perm_in(struct x86_emulate_ctxt *ctxt)
{
	ctxt->dst.bytes = min(ctxt->dst.bytes, 4u);
	if (!emulator_io_permited(ctxt, ctxt->src.val, ctxt->dst.bytes))
		return emulate_gp(ctxt, 0);

	return X86EMUL_CONTINUE;
}

static int check_perm_out(struct x86_emulate_ctxt *ctxt)
{
	ctxt->src.bytes = min(ctxt->src.bytes, 4u);
	if (!emulator_io_permited(ctxt, ctxt->dst.val, ctxt->src.bytes))
		return emulate_gp(ctxt, 0);

	return X86EMUL_CONTINUE;
}

#define D(_y) { .flags = (_y) }
#define DI(_y, _i) { .flags = (_y)|Intercept, .intercept = x86_intercept_##_i }
#define DIP(_y, _i, _p) { .flags = (_y)|Intercept|CheckPerm, \
		      .intercept = x86_intercept_##_i, .check_perm = (_p) }
#define N    D(NotImpl)
#define EXT(_f, _e) { .flags = ((_f) | RMExt), .u.group = (_e) }
#define G(_f, _g) { .flags = ((_f) | Group | ModRM), .u.group = (_g) }
#define GD(_f, _g) { .flags = ((_f) | GroupDual | ModRM), .u.gdual = (_g) }
#define E(_f, _e) { .flags = ((_f) | Escape | ModRM), .u.esc = (_e) }
#define I(_f, _e) { .flags = (_f), .u.execute = (_e) }
#define F(_f, _e) { .flags = (_f) | Fastop, .u.fastop = (_e) }
#define II(_f, _e, _i) \
	{ .flags = (_f)|Intercept, .u.execute = (_e), .intercept = x86_intercept_##_i }
	} else if (ctxt->mode == X86EMUL_MODE_PROT64) {
		ret = ctxt->ops->read_std(ctxt, desc_addr+8, &base3,
				sizeof(base3), &ctxt->exception);
		if (ret != X86EMUL_CONTINUE)
			return ret;
	}
load:
	ctxt->ops->set_segment(ctxt, selector, &seg_desc, base3, seg);
	if (desc)
		*desc = seg_desc;
	return X86EMUL_CONTINUE;
exception:
	return emulate_exception(ctxt, err_vec, err_code, true);
}

static int load_segment_descriptor(struct x86_emulate_ctxt *ctxt,
				   u16 selector, int seg)
{
	u8 cpl = ctxt->ops->cpl(ctxt);
	return __load_segment_descriptor(ctxt, selector, seg, cpl, false, NULL);
}

static void write_register_operand(struct operand *op)
{
	/* The 4-byte case *is* correct: in 64-bit mode we zero-extend. */
	switch (op->bytes) {
	case 1:
		*(u8 *)op->addr.reg = (u8)op->val;
		break;
	case 2:
		*(u16 *)op->addr.reg = (u16)op->val;
		break;
	case 4:
		*op->addr.reg = (u32)op->val;
		break;	/* 64b: zero-extend */
	case 8:
		*op->addr.reg = op->val;
		break;
	}
{
	u8 cpl = ctxt->ops->cpl(ctxt);
	return __load_segment_descriptor(ctxt, selector, seg, cpl, false, NULL);
}
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
{
	int ret;
	u8 cpl;

	ctxt->_eip = tss->ip;
	ctxt->eflags = tss->flag | 2;
	*reg_write(ctxt, VCPU_REGS_RAX) = tss->ax;
	*reg_write(ctxt, VCPU_REGS_RCX) = tss->cx;
	*reg_write(ctxt, VCPU_REGS_RDX) = tss->dx;
	*reg_write(ctxt, VCPU_REGS_RBX) = tss->bx;
	*reg_write(ctxt, VCPU_REGS_RSP) = tss->sp;
	*reg_write(ctxt, VCPU_REGS_RBP) = tss->bp;
	*reg_write(ctxt, VCPU_REGS_RSI) = tss->si;
	*reg_write(ctxt, VCPU_REGS_RDI) = tss->di;

	/*
	 * SDM says that segment selectors are loaded before segment
	 * descriptors
	 */
	set_segment_selector(ctxt, tss->ldt, VCPU_SREG_LDTR);
	set_segment_selector(ctxt, tss->es, VCPU_SREG_ES);
	set_segment_selector(ctxt, tss->cs, VCPU_SREG_CS);
	set_segment_selector(ctxt, tss->ss, VCPU_SREG_SS);
	set_segment_selector(ctxt, tss->ds, VCPU_SREG_DS);

	cpl = tss->cs & 3;

	/*
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
}

static int task_switch_16(struct x86_emulate_ctxt *ctxt,
			  u16 tss_selector, u16 old_tss_sel,
			  ulong old_tss_base, struct desc_struct *new_desc)
{
	const struct x86_emulate_ops *ops = ctxt->ops;
	struct tss_segment_16 tss_seg;
	int ret;
	u32 new_tss_base = get_desc_base(new_desc);

	ret = ops->read_std(ctxt, old_tss_base, &tss_seg, sizeof tss_seg,
			    &ctxt->exception);
	if (ret != X86EMUL_CONTINUE)
		/* FIXME: need to provide precise fault address */
		return ret;

	save_state_to_tss16(ctxt, &tss_seg);

	ret = ops->write_std(ctxt, old_tss_base, &tss_seg, sizeof tss_seg,
			     &ctxt->exception);
	if (ret != X86EMUL_CONTINUE)
		/* FIXME: need to provide precise fault address */
		return ret;

	ret = ops->read_std(ctxt, new_tss_base, &tss_seg, sizeof tss_seg,
			    &ctxt->exception);
	if (ret != X86EMUL_CONTINUE)
		/* FIXME: need to provide precise fault address */
		return ret;

	if (old_tss_sel != 0xffff) {
		tss_seg.prev_task_link = old_tss_sel;

		ret = ops->write_std(ctxt, new_tss_base,
				     &tss_seg.prev_task_link,
				     sizeof tss_seg.prev_task_link,
				     &ctxt->exception);
		if (ret != X86EMUL_CONTINUE)
			/* FIXME: need to provide precise fault address */
			return ret;
	}
	} else {
		ctxt->mode = X86EMUL_MODE_PROT32;
		cpl = tss->cs & 3;
	}

	/*
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
}

static int task_switch_32(struct x86_emulate_ctxt *ctxt,
			  u16 tss_selector, u16 old_tss_sel,
			  ulong old_tss_base, struct desc_struct *new_desc)
{
	const struct x86_emulate_ops *ops = ctxt->ops;
	struct tss_segment_32 tss_seg;
	int ret;
	u32 new_tss_base = get_desc_base(new_desc);
	u32 eip_offset = offsetof(struct tss_segment_32, eip);
	u32 ldt_sel_offset = offsetof(struct tss_segment_32, ldt_selector);

	ret = ops->read_std(ctxt, old_tss_base, &tss_seg, sizeof tss_seg,
			    &ctxt->exception);
	if (ret != X86EMUL_CONTINUE)
		/* FIXME: need to provide precise fault address */
		return ret;

	save_state_to_tss32(ctxt, &tss_seg);

	/* Only GP registers and segment selectors are saved */
	ret = ops->write_std(ctxt, old_tss_base + eip_offset, &tss_seg.eip,
			     ldt_sel_offset - eip_offset, &ctxt->exception);
	if (ret != X86EMUL_CONTINUE)
		/* FIXME: need to provide precise fault address */
		return ret;

	ret = ops->read_std(ctxt, new_tss_base, &tss_seg, sizeof tss_seg,
			    &ctxt->exception);
	if (ret != X86EMUL_CONTINUE)
		/* FIXME: need to provide precise fault address */
		return ret;

	if (old_tss_sel != 0xffff) {
		tss_seg.prev_task_link = old_tss_sel;

		ret = ops->write_std(ctxt, new_tss_base,
				     &tss_seg.prev_task_link,
				     sizeof tss_seg.prev_task_link,
				     &ctxt->exception);
		if (ret != X86EMUL_CONTINUE)
			/* FIXME: need to provide precise fault address */
			return ret;
	}
{
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
	int rc;
	unsigned long eip;

	rc = emulate_pop(ctxt, &eip, ctxt->op_bytes);
	if (rc != X86EMUL_CONTINUE)
		return rc;
	rc = assign_eip_near(ctxt, eip);
	if (rc != X86EMUL_CONTINUE)
		return rc;
	rsp_increment(ctxt, ctxt->src.val);
	return X86EMUL_CONTINUE;
}

static int em_xchg(struct x86_emulate_ctxt *ctxt)
{
	/* Write back the register source. */
	ctxt->src.val = ctxt->dst.val;
	write_register_operand(&ctxt->src);

	/* Write back the memory destination with implicit LOCK prefix. */
	ctxt->dst.val = ctxt->src.orig_val;
	ctxt->lock_prefix = 1;
	return X86EMUL_CONTINUE;
}

static int em_imul_3op(struct x86_emulate_ctxt *ctxt)
{
	ctxt->dst.val = ctxt->src2.val;
	return fastop(ctxt, em_imul);
}

static int em_cwd(struct x86_emulate_ctxt *ctxt)
{
	ctxt->dst.type = OP_REG;
	ctxt->dst.bytes = ctxt->src.bytes;
	ctxt->dst.addr.reg = reg_rmw(ctxt, VCPU_REGS_RDX);
	ctxt->dst.val = ~((ctxt->src.val >> (ctxt->src.bytes * 8 - 1)) - 1);

	return X86EMUL_CONTINUE;
}

static int em_rdtsc(struct x86_emulate_ctxt *ctxt)
{
	u64 tsc = 0;

	ctxt->ops->get_msr(ctxt, MSR_IA32_TSC, &tsc);
	*reg_write(ctxt, VCPU_REGS_RAX) = (u32)tsc;
	*reg_write(ctxt, VCPU_REGS_RDX) = tsc >> 32;
	return X86EMUL_CONTINUE;
}

static int em_rdpmc(struct x86_emulate_ctxt *ctxt)
{
	u64 pmc;

	if (ctxt->ops->read_pmc(ctxt, reg_read(ctxt, VCPU_REGS_RCX), &pmc))
		return emulate_gp(ctxt, 0);
	*reg_write(ctxt, VCPU_REGS_RAX) = (u32)pmc;
	*reg_write(ctxt, VCPU_REGS_RDX) = pmc >> 32;
	return X86EMUL_CONTINUE;
}

static int em_mov(struct x86_emulate_ctxt *ctxt)
{
	memcpy(ctxt->dst.valptr, ctxt->src.valptr, sizeof(ctxt->src.valptr));
	return X86EMUL_CONTINUE;
}

#define FFL(x) bit(X86_FEATURE_##x)

static int em_movbe(struct x86_emulate_ctxt *ctxt)
{
	u32 ebx, ecx, edx, eax = 1;
	u16 tmp;

	/*
	 * Check MOVBE is set in the guest-visible CPUID leaf.
	 */
	ctxt->ops->get_cpuid(ctxt, &eax, &ebx, &ecx, &edx);
	if (!(ecx & FFL(MOVBE)))
		return emulate_ud(ctxt);

	switch (ctxt->op_bytes) {
	case 2:
		/*
		 * From MOVBE definition: "...When the operand size is 16 bits,
		 * the upper word of the destination register remains unchanged
		 * ..."
		 *
		 * Both casting ->valptr and ->val to u16 breaks strict aliasing
		 * rules so we have to do the operation almost per hand.
		 */
		tmp = (u16)ctxt->src.val;
		ctxt->dst.val &= ~0xffffUL;
		ctxt->dst.val |= (unsigned long)swab16(tmp);
		break;
	case 4:
		ctxt->dst.val = swab32((u32)ctxt->src.val);
		break;
	case 8:
		ctxt->dst.val = swab64(ctxt->src.val);
		break;
	default:
		BUG();
	}
{
	int rc;
	ulong linear;

	rc = linearize(ctxt, addr, size, false, &linear);
	if (rc != X86EMUL_CONTINUE)
		return rc;
	return ctxt->ops->read_std(ctxt, linear, data, size, &ctxt->exception);
}

/*
 * Prefetch the remaining bytes of the instruction without crossing page
 * boundary if they are not in fetch_cache yet.
 */
static int __do_insn_fetch_bytes(struct x86_emulate_ctxt *ctxt, int op_size)
{
	int rc;
	unsigned size, max_size;
	unsigned long linear;
	int cur_size = ctxt->fetch.end - ctxt->fetch.data;
	struct segmented_address addr = { .seg = VCPU_SREG_CS,
					   .ea = ctxt->eip + cur_size };

	/*
	 * We do not know exactly how many bytes will be needed, and
	 * __linearize is expensive, so fetch as much as possible.  We
	 * just have to avoid going beyond the 15 byte limit, the end
	 * of the segment, or the end of the page.
	 *
	 * __linearize is called with size 0 so that it does not do any
	 * boundary check itself.  Instead, we use max_size to check
	 * against op_size.
	 */
	rc = __linearize(ctxt, addr, &max_size, 0, false, true, ctxt->mode,
			 &linear);
	if (unlikely(rc != X86EMUL_CONTINUE))
		return rc;

	size = min_t(unsigned, 15UL ^ cur_size, max_size);
	size = min_t(unsigned, size, PAGE_SIZE - offset_in_page(linear));

	/*
	 * One instruction can only straddle two pages,
	 * and one has been loaded at the beginning of
	 * x86_decode_insn.  So, if not enough bytes
	 * still, we must have hit the 15-byte boundary.
	 */
	if (unlikely(size < op_size))
		return emulate_gp(ctxt, 0);

	rc = ctxt->ops->fetch(ctxt, linear, ctxt->fetch.end,
			      size, &ctxt->exception);
	if (unlikely(rc != X86EMUL_CONTINUE))
		return rc;
	ctxt->fetch.end += size;
	return X86EMUL_CONTINUE;
}

static __always_inline int do_insn_fetch_bytes(struct x86_emulate_ctxt *ctxt,
					       unsigned size)
{
	unsigned done_size = ctxt->fetch.end - ctxt->fetch.ptr;

	if (unlikely(done_size < size))
		return __do_insn_fetch_bytes(ctxt, size - done_size);
	else
		return X86EMUL_CONTINUE;
}

/* Fetch next part of the instruction being emulated. */
#define insn_fetch(_type, _ctxt)					\
({	_type _x;							\
									\
	rc = do_insn_fetch_bytes(_ctxt, sizeof(_type));			\
	if (rc != X86EMUL_CONTINUE)					\
		goto done;						\
	ctxt->_eip += sizeof(_type);					\
	_x = *(_type __aligned(1) *) ctxt->fetch.ptr;			\
	ctxt->fetch.ptr += sizeof(_type);				\
	_x;								\
})

#define insn_fetch_arr(_arr, _size, _ctxt)				\
({									\
	rc = do_insn_fetch_bytes(_ctxt, _size);				\
	if (rc != X86EMUL_CONTINUE)					\
		goto done;						\
	ctxt->_eip += (_size);						\
	memcpy(_arr, ctxt->fetch.ptr, _size);				\
	ctxt->fetch.ptr += (_size);					\
})

/*
 * Given the 'reg' portion of a ModRM byte, and a register block, return a
 * pointer into the block that addresses the relevant register.
 * @highbyte_regs specifies whether to decode AH,CH,DH,BH.
 */
static void *decode_register(struct x86_emulate_ctxt *ctxt, u8 modrm_reg,
			     int byteop)
{
	void *p;
	int highbyte_regs = (ctxt->rex_prefix == 0) && byteop;

	if (highbyte_regs && modrm_reg >= 4 && modrm_reg < 8)
		p = (unsigned char *)reg_rmw(ctxt, modrm_reg & 3) + 1;
	else
		p = reg_rmw(ctxt, modrm_reg);
	return p;
}

static int read_descriptor(struct x86_emulate_ctxt *ctxt,
			   struct segmented_address addr,
			   u16 *size, unsigned long *address, int op_bytes)
{
	int rc;

	if (op_bytes == 2)
		op_bytes = 3;
	*address = 0;
	rc = segmented_read_std(ctxt, addr, size, 2);
	if (rc != X86EMUL_CONTINUE)
		return rc;
	addr.ea += 2;
	rc = segmented_read_std(ctxt, addr, address, op_bytes);
	return rc;
}

FASTOP2(add);
FASTOP2(or);
FASTOP2(adc);
FASTOP2(sbb);
FASTOP2(and);
FASTOP2(sub);
FASTOP2(xor);
FASTOP2(cmp);
FASTOP2(test);

FASTOP1SRC2(mul, mul_ex);
FASTOP1SRC2(imul, imul_ex);
FASTOP1SRC2EX(div, div_ex);
FASTOP1SRC2EX(idiv, idiv_ex);

FASTOP3WCL(shld);
FASTOP3WCL(shrd);

FASTOP2W(imul);

FASTOP1(not);
FASTOP1(neg);
FASTOP1(inc);
FASTOP1(dec);

FASTOP2CL(rol);
FASTOP2CL(ror);
FASTOP2CL(rcl);
FASTOP2CL(rcr);
FASTOP2CL(shl);
FASTOP2CL(shr);
FASTOP2CL(sar);

FASTOP2W(bsf);
FASTOP2W(bsr);
FASTOP2W(bt);
FASTOP2W(bts);
FASTOP2W(btr);
FASTOP2W(btc);

FASTOP2(xadd);

FASTOP2R(cmp, cmp_r);

static int em_bsf_c(struct x86_emulate_ctxt *ctxt)
{
	/* If src is zero, do not writeback, but update flags */
	if (ctxt->src.val == 0)
		ctxt->dst.type = OP_NONE;
	return fastop(ctxt, em_bsf);
}

static int em_bsr_c(struct x86_emulate_ctxt *ctxt)
{
	/* If src is zero, do not writeback, but update flags */
	if (ctxt->src.val == 0)
		ctxt->dst.type = OP_NONE;
	return fastop(ctxt, em_bsr);
}

static __always_inline u8 test_cc(unsigned int condition, unsigned long flags)
{
	u8 rc;
	void (*fop)(void) = (void *)em_setcc + 4 * (condition & 0xf);

	flags = (flags & EFLAGS_MASK) | X86_EFLAGS_IF;
	asm("push %[flags]; popf; call *%[fastop]"
	    : "=a"(rc) : [fastop]"r"(fop), [flags]"r"(flags));
	return rc;
}

static void fetch_register_operand(struct operand *op)
{
	switch (op->bytes) {
	case 1:
		op->val = *(u8 *)op->addr.reg;
		break;
	case 2:
		op->val = *(u16 *)op->addr.reg;
		break;
	case 4:
		op->val = *(u32 *)op->addr.reg;
		break;
	case 8:
		op->val = *(u64 *)op->addr.reg;
		break;
	}
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

static int em_lgdt_lidt(struct x86_emulate_ctxt *ctxt, bool lgdt)
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
	if (ctxt->mode == X86EMUL_MODE_PROT64 &&
	    is_noncanonical_address(desc_ptr.address))
		return emulate_gp(ctxt, 0);
	if (lgdt)
		ctxt->ops->set_gdt(ctxt, &desc_ptr);
	else
		ctxt->ops->set_idt(ctxt, &desc_ptr);
	/* Disable writeback. */
	ctxt->dst.type = OP_NONE;
	return X86EMUL_CONTINUE;
}

static int em_lgdt(struct x86_emulate_ctxt *ctxt)
{
	return em_lgdt_lidt(ctxt, true);
}

static int em_lidt(struct x86_emulate_ctxt *ctxt)
{
	return em_lgdt_lidt(ctxt, false);
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

	register_address_increment(ctxt, VCPU_REGS_RCX, -1);
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

	flags = X86_EFLAGS_CF | X86_EFLAGS_PF | X86_EFLAGS_AF | X86_EFLAGS_ZF |
		X86_EFLAGS_SF;
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
{
	struct fxregs_state fx_state;
	size_t size;
	int rc;

	rc = check_fxsr(ctxt);
	if (rc != X86EMUL_CONTINUE)
		return rc;

	ctxt->ops->get_fpu(ctxt);

	rc = asm_safe("fxsave %[fx]", , [fx] "+m"(fx_state));

	ctxt->ops->put_fpu(ctxt);

	if (rc != X86EMUL_CONTINUE)
		return rc;

	if (ctxt->ops->get_cr(ctxt, 4) & X86_CR4_OSFXSR)
		size = offsetof(struct fxregs_state, xmm_space[8 * 16/4]);
	else
		size = offsetof(struct fxregs_state, xmm_space[0]);

	return segmented_write(ctxt, ctxt->memop.addr.mem, &fx_state, size);
}

static int fxrstor_fixup(struct x86_emulate_ctxt *ctxt,
		struct fxregs_state *new)
{
	int rc = X86EMUL_CONTINUE;
	struct fxregs_state old;

	rc = asm_safe("fxsave %[fx]", , [fx] "+m"(old));
	if (rc != X86EMUL_CONTINUE)
		return rc;

	/*
	 * 64 bit host will restore XMM 8-15, which is not correct on non-64
	 * bit guests.  Load the current values in order to preserve 64 bit
	 * XMMs after fxrstor.
	 */
#ifdef CONFIG_X86_64
	/* XXX: accessing XMM 8-15 very awkwardly */
	memcpy(&new->xmm_space[8 * 16/4], &old.xmm_space[8 * 16/4], 8 * 16);
#endif

	/*
	 * Hardware doesn't save and restore XMM 0-7 without CR4.OSFXSR, but
	 * does save and restore MXCSR.
	 */
	if (!(ctxt->ops->get_cr(ctxt, 4) & X86_CR4_OSFXSR))
		memcpy(new->xmm_space, old.xmm_space, 8 * 16);

	return rc;
}

static int em_fxrstor(struct x86_emulate_ctxt *ctxt)
{
	struct fxregs_state fx_state;
	int rc;

	rc = check_fxsr(ctxt);
	if (rc != X86EMUL_CONTINUE)
		return rc;

	rc = segmented_read(ctxt, ctxt->memop.addr.mem, &fx_state, 512);
	if (rc != X86EMUL_CONTINUE)
		return rc;

	if (fx_state.mxcsr >> 16)
		return emulate_gp(ctxt, 0);

	ctxt->ops->get_fpu(ctxt);

	if (ctxt->mode < X86EMUL_MODE_PROT64)
		rc = fxrstor_fixup(ctxt, &fx_state);

	if (rc == X86EMUL_CONTINUE)
		rc = asm_safe("fxrstor %[fx]", : [fx] "m"(fx_state));

	ctxt->ops->put_fpu(ctxt);

	return rc;
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
{
	struct fxregs_state fx_state;
	int rc;

	rc = check_fxsr(ctxt);
	if (rc != X86EMUL_CONTINUE)
		return rc;

	rc = segmented_read(ctxt, ctxt->memop.addr.mem, &fx_state, 512);
	if (rc != X86EMUL_CONTINUE)
		return rc;

	if (fx_state.mxcsr >> 16)
		return emulate_gp(ctxt, 0);

	ctxt->ops->get_fpu(ctxt);

	if (ctxt->mode < X86EMUL_MODE_PROT64)
		rc = fxrstor_fixup(ctxt, &fx_state);

	if (rc == X86EMUL_CONTINUE)
		rc = asm_safe("fxrstor %[fx]", : [fx] "m"(fx_state));

	ctxt->ops->put_fpu(ctxt);

	return rc;
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
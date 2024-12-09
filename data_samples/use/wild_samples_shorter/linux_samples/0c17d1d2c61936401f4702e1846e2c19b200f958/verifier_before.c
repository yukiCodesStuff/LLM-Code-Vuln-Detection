					   strict);
}

/* check whether memory at (regno + off) is accessible for t = (read | write)
 * if t==write, value_regno is a register which value is stored into memory
 * if t==read, value_regno is a register which will receive the value from memory
 * if t==write && value_regno==-1, some unknown value is stored into memory
	if (!err && size < BPF_REG_SIZE && value_regno >= 0 && t == BPF_READ &&
	    regs[value_regno].type == SCALAR_VALUE) {
		/* b/h/w load zero-extends, mark upper bits as known 0 */
		regs[value_regno].var_off =
			tnum_cast(regs[value_regno].var_off, size);
		__update_reg_bounds(&regs[value_regno]);
	}
	return err;
}

	return 0;
}

static void coerce_reg_to_32(struct bpf_reg_state *reg)
{
	/* clear high 32 bits */
	reg->var_off = tnum_cast(reg->var_off, 4);
	/* Update bounds */
	__update_reg_bounds(reg);
}

static bool signed_add_overflows(s64 a, s64 b)
{
	/* Do the add in u64, where overflow is well-defined */
	s64 res = (s64)((u64)a + (u64)b);

	if (BPF_CLASS(insn->code) != BPF_ALU64) {
		/* 32-bit ALU ops are (32,32)->64 */
		coerce_reg_to_32(dst_reg);
		coerce_reg_to_32(&src_reg);
	}
	smin_val = src_reg.smin_value;
	smax_val = src_reg.smax_value;
	umin_val = src_reg.umin_value;
					return -EACCES;
				}
				mark_reg_unknown(env, regs, insn->dst_reg);
				/* high 32 bits are known zero. */
				regs[insn->dst_reg].var_off = tnum_cast(
						regs[insn->dst_reg].var_off, 4);
				__update_reg_bounds(&regs[insn->dst_reg]);
			}
		} else {
			/* case: R = imm
			 * remember the value we stored into this reg
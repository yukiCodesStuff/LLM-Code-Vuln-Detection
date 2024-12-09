			 * remember the value we stored into this reg
			 */
			regs[insn->dst_reg].type = SCALAR_VALUE;
			__mark_reg_known(regs + insn->dst_reg, insn->imm);
		}

	} else if (opcode > BPF_END) {
		verbose(env, "invalid BPF_ALU opcode %x\n", opcode);
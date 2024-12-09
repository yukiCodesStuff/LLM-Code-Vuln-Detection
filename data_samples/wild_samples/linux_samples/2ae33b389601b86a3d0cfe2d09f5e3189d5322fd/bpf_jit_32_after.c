			} else {
				emit(ARM_CMP_R(r_skb_hl, r_off), ctx);
				condt = ARM_COND_HI;
			}

			_emit(condt, ARM_ADD_R(r_scratch, r_off, r_skb_data),
			      ctx);

			if (load_order == 0)
				_emit(condt, ARM_LDRB_I(r_A, r_scratch, 0),
				      ctx);
			else if (load_order == 1)
				emit_load_be16(condt, r_A, r_scratch, ctx);
			else if (load_order == 2)
				emit_load_be32(condt, r_A, r_scratch, ctx);

			_emit(condt, ARM_B(b_imm(i + 1, ctx)), ctx);

			/* the slowpath */
			emit_mov_i(ARM_R3, (u32)load_func[load_order], ctx);
			emit(ARM_MOV_R(ARM_R0, r_skb), ctx);
			/* the offset is already in R1 */
			emit_blx_r(ARM_R3, ctx);
			/* check the result of skb_copy_bits */
			emit(ARM_CMP_I(ARM_R1, 0), ctx);
			emit_err_ret(ARM_COND_NE, ctx);
			emit(ARM_MOV_R(r_A, ARM_R0), ctx);
			break;
		case BPF_S_LD_W_IND:
			load_order = 2;
			goto load_ind;
		case BPF_S_LD_H_IND:
			load_order = 1;
			goto load_ind;
		case BPF_S_LD_B_IND:
			load_order = 0;
load_ind:
			OP_IMM3(ARM_ADD, r_off, r_X, k, ctx);
			goto load_common;
		case BPF_S_LDX_IMM:
			ctx->seen |= SEEN_X;
			emit_mov_i(r_X, k, ctx);
			break;
		case BPF_S_LDX_W_LEN:
			ctx->seen |= SEEN_X | SEEN_SKB;
			emit(ARM_LDR_I(r_X, r_skb,
				       offsetof(struct sk_buff, len)), ctx);
			break;
		case BPF_S_LDX_MEM:
			ctx->seen |= SEEN_X | SEEN_MEM_WORD(k);
			emit(ARM_LDR_I(r_X, ARM_SP, SCRATCH_OFF(k)), ctx);
			break;
		case BPF_S_LDX_B_MSH:
			/* x = ((*(frame + k)) & 0xf) << 2; */
			ctx->seen |= SEEN_X | SEEN_DATA | SEEN_CALL;
			/* the interpreter should deal with the negative K */
			if ((int)k < 0)
				return -1;
			/* offset in r1: we might have to take the slow path */
			emit_mov_i(r_off, k, ctx);
			emit(ARM_CMP_R(r_skb_hl, r_off), ctx);

			/* load in r0: common with the slowpath */
			_emit(ARM_COND_HI, ARM_LDRB_R(ARM_R0, r_skb_data,
						      ARM_R1), ctx);
			/*
			 * emit_mov_i() might generate one or two instructions,
			 * the same holds for emit_blx_r()
			 */
			_emit(ARM_COND_HI, ARM_B(b_imm(i + 1, ctx) - 2), ctx);

			emit(ARM_MOV_R(ARM_R0, r_skb), ctx);
			/* r_off is r1 */
			emit_mov_i(ARM_R3, (u32)jit_get_skb_b, ctx);
			emit_blx_r(ARM_R3, ctx);
			/* check the return value of skb_copy_bits */
			emit(ARM_CMP_I(ARM_R1, 0), ctx);
			emit_err_ret(ARM_COND_NE, ctx);

			emit(ARM_AND_I(r_X, ARM_R0, 0x00f), ctx);
			emit(ARM_LSL_I(r_X, r_X, 2), ctx);
			break;
		case BPF_S_ST:
			ctx->seen |= SEEN_MEM_WORD(k);
			emit(ARM_STR_I(r_A, ARM_SP, SCRATCH_OFF(k)), ctx);
			break;
		case BPF_S_STX:
			update_on_xread(ctx);
			ctx->seen |= SEEN_MEM_WORD(k);
			emit(ARM_STR_I(r_X, ARM_SP, SCRATCH_OFF(k)), ctx);
			break;
		case BPF_S_ALU_ADD_K:
			/* A += K */
			OP_IMM3(ARM_ADD, r_A, r_A, k, ctx);
			break;
		case BPF_S_ALU_ADD_X:
			update_on_xread(ctx);
			emit(ARM_ADD_R(r_A, r_A, r_X), ctx);
			break;
		case BPF_S_ALU_SUB_K:
			/* A -= K */
			OP_IMM3(ARM_SUB, r_A, r_A, k, ctx);
			break;
		case BPF_S_ALU_SUB_X:
			update_on_xread(ctx);
			emit(ARM_SUB_R(r_A, r_A, r_X), ctx);
			break;
		case BPF_S_ALU_MUL_K:
			/* A *= K */
			emit_mov_i(r_scratch, k, ctx);
			emit(ARM_MUL(r_A, r_A, r_scratch), ctx);
			break;
		case BPF_S_ALU_MUL_X:
			update_on_xread(ctx);
			emit(ARM_MUL(r_A, r_A, r_X), ctx);
			break;
		case BPF_S_ALU_DIV_K:
			/* current k == reciprocal_value(userspace k) */
			emit_mov_i(r_scratch, k, ctx);
			/* A = top 32 bits of the product */
			emit(ARM_UMULL(r_scratch, r_A, r_A, r_scratch), ctx);
			break;
		case BPF_S_ALU_DIV_X:
			update_on_xread(ctx);
			emit(ARM_CMP_I(r_X, 0), ctx);
			emit_err_ret(ARM_COND_EQ, ctx);
			emit_udiv(r_A, r_A, r_X, ctx);
			break;
		case BPF_S_ALU_OR_K:
			/* A |= K */
			OP_IMM3(ARM_ORR, r_A, r_A, k, ctx);
			break;
		case BPF_S_ALU_OR_X:
			update_on_xread(ctx);
			emit(ARM_ORR_R(r_A, r_A, r_X), ctx);
			break;
		case BPF_S_ALU_XOR_K:
			/* A ^= K; */
			OP_IMM3(ARM_EOR, r_A, r_A, k, ctx);
			break;
		case BPF_S_ANC_ALU_XOR_X:
		case BPF_S_ALU_XOR_X:
			/* A ^= X */
			update_on_xread(ctx);
			emit(ARM_EOR_R(r_A, r_A, r_X), ctx);
			break;
		case BPF_S_ALU_AND_K:
			/* A &= K */
			OP_IMM3(ARM_AND, r_A, r_A, k, ctx);
			break;
		case BPF_S_ALU_AND_X:
			update_on_xread(ctx);
			emit(ARM_AND_R(r_A, r_A, r_X), ctx);
			break;
		case BPF_S_ALU_LSH_K:
			if (unlikely(k > 31))
				return -1;
			emit(ARM_LSL_I(r_A, r_A, k), ctx);
			break;
		case BPF_S_ALU_LSH_X:
			update_on_xread(ctx);
			emit(ARM_LSL_R(r_A, r_A, r_X), ctx);
			break;
		case BPF_S_ALU_RSH_K:
			if (unlikely(k > 31))
				return -1;
			emit(ARM_LSR_I(r_A, r_A, k), ctx);
			break;
		case BPF_S_ALU_RSH_X:
			update_on_xread(ctx);
			emit(ARM_LSR_R(r_A, r_A, r_X), ctx);
			break;
		case BPF_S_ALU_NEG:
			/* A = -A */
			emit(ARM_RSB_I(r_A, r_A, 0), ctx);
			break;
		case BPF_S_JMP_JA:
			/* pc += K */
			emit(ARM_B(b_imm(i + k + 1, ctx)), ctx);
			break;
		case BPF_S_JMP_JEQ_K:
			/* pc += (A == K) ? pc->jt : pc->jf */
			condt  = ARM_COND_EQ;
			goto cmp_imm;
		case BPF_S_JMP_JGT_K:
			/* pc += (A > K) ? pc->jt : pc->jf */
			condt  = ARM_COND_HI;
			goto cmp_imm;
		case BPF_S_JMP_JGE_K:
			/* pc += (A >= K) ? pc->jt : pc->jf */
			condt  = ARM_COND_HS;
cmp_imm:
			imm12 = imm8m(k);
			if (imm12 < 0) {
				emit_mov_i_no8m(r_scratch, k, ctx);
				emit(ARM_CMP_R(r_A, r_scratch), ctx);
			} else {
				emit(ARM_CMP_I(r_A, imm12), ctx);
			}
cond_jump:
			if (inst->jt)
				_emit(condt, ARM_B(b_imm(i + inst->jt + 1,
						   ctx)), ctx);
			if (inst->jf)
				_emit(condt ^ 1, ARM_B(b_imm(i + inst->jf + 1,
							     ctx)), ctx);
			break;
		case BPF_S_JMP_JEQ_X:
			/* pc += (A == X) ? pc->jt : pc->jf */
			condt   = ARM_COND_EQ;
			goto cmp_x;
		case BPF_S_JMP_JGT_X:
			/* pc += (A > X) ? pc->jt : pc->jf */
			condt   = ARM_COND_HI;
			goto cmp_x;
		case BPF_S_JMP_JGE_X:
			/* pc += (A >= X) ? pc->jt : pc->jf */
			condt   = ARM_COND_CS;
cmp_x:
			update_on_xread(ctx);
			emit(ARM_CMP_R(r_A, r_X), ctx);
			goto cond_jump;
		case BPF_S_JMP_JSET_K:
			/* pc += (A & K) ? pc->jt : pc->jf */
			condt  = ARM_COND_NE;
			/* not set iff all zeroes iff Z==1 iff EQ */

			imm12 = imm8m(k);
			if (imm12 < 0) {
				emit_mov_i_no8m(r_scratch, k, ctx);
				emit(ARM_TST_R(r_A, r_scratch), ctx);
			} else {
				emit(ARM_TST_I(r_A, imm12), ctx);
			}
			goto cond_jump;
		case BPF_S_JMP_JSET_X:
			/* pc += (A & X) ? pc->jt : pc->jf */
			update_on_xread(ctx);
			condt  = ARM_COND_NE;
			emit(ARM_TST_R(r_A, r_X), ctx);
			goto cond_jump;
		case BPF_S_RET_A:
			emit(ARM_MOV_R(ARM_R0, r_A), ctx);
			goto b_epilogue;
		case BPF_S_RET_K:
			if ((k == 0) && (ctx->ret0_fp_idx < 0))
				ctx->ret0_fp_idx = i;
			emit_mov_i(ARM_R0, k, ctx);
b_epilogue:
			if (i != ctx->skf->len - 1)
				emit(ARM_B(b_imm(prog->len, ctx)), ctx);
			break;
		case BPF_S_MISC_TAX:
			/* X = A */
			ctx->seen |= SEEN_X;
			emit(ARM_MOV_R(r_X, r_A), ctx);
			break;
		case BPF_S_MISC_TXA:
			/* A = X */
			update_on_xread(ctx);
			emit(ARM_MOV_R(r_A, r_X), ctx);
			break;
		case BPF_S_ANC_PROTOCOL:
			/* A = ntohs(skb->protocol) */
			ctx->seen |= SEEN_SKB;
			BUILD_BUG_ON(FIELD_SIZEOF(struct sk_buff,
						  protocol) != 2);
			off = offsetof(struct sk_buff, protocol);
			emit(ARM_LDRH_I(r_scratch, r_skb, off), ctx);
			emit_swap16(r_A, r_scratch, ctx);
			break;
		case BPF_S_ANC_CPU:
			/* r_scratch = current_thread_info() */
			OP_IMM3(ARM_BIC, r_scratch, ARM_SP, THREAD_SIZE - 1, ctx);
			/* A = current_thread_info()->cpu */
			BUILD_BUG_ON(FIELD_SIZEOF(struct thread_info, cpu) != 4);
			off = offsetof(struct thread_info, cpu);
			emit(ARM_LDR_I(r_A, r_scratch, off), ctx);
			break;
		case BPF_S_ANC_IFINDEX:
			/* A = skb->dev->ifindex */
			ctx->seen |= SEEN_SKB;
			off = offsetof(struct sk_buff, dev);
			emit(ARM_LDR_I(r_scratch, r_skb, off), ctx);

			emit(ARM_CMP_I(r_scratch, 0), ctx);
			emit_err_ret(ARM_COND_EQ, ctx);

			BUILD_BUG_ON(FIELD_SIZEOF(struct net_device,
						  ifindex) != 4);
			off = offsetof(struct net_device, ifindex);
			emit(ARM_LDR_I(r_A, r_scratch, off), ctx);
			break;
		case BPF_S_ANC_MARK:
			ctx->seen |= SEEN_SKB;
			BUILD_BUG_ON(FIELD_SIZEOF(struct sk_buff, mark) != 4);
			off = offsetof(struct sk_buff, mark);
			emit(ARM_LDR_I(r_A, r_skb, off), ctx);
			break;
		case BPF_S_ANC_RXHASH:
			ctx->seen |= SEEN_SKB;
			BUILD_BUG_ON(FIELD_SIZEOF(struct sk_buff, rxhash) != 4);
			off = offsetof(struct sk_buff, rxhash);
			emit(ARM_LDR_I(r_A, r_skb, off), ctx);
			break;
		case BPF_S_ANC_VLAN_TAG:
		case BPF_S_ANC_VLAN_TAG_PRESENT:
			ctx->seen |= SEEN_SKB;
			BUILD_BUG_ON(FIELD_SIZEOF(struct sk_buff, vlan_tci) != 2);
			off = offsetof(struct sk_buff, vlan_tci);
			emit(ARM_LDRH_I(r_A, r_skb, off), ctx);
			if (inst->code == BPF_S_ANC_VLAN_TAG)
				OP_IMM3(ARM_AND, r_A, r_A, VLAN_VID_MASK, ctx);
			else
				OP_IMM3(ARM_AND, r_A, r_A, VLAN_TAG_PRESENT, ctx);
			break;
		case BPF_S_ANC_QUEUE:
			ctx->seen |= SEEN_SKB;
			BUILD_BUG_ON(FIELD_SIZEOF(struct sk_buff,
						  queue_mapping) != 2);
			BUILD_BUG_ON(offsetof(struct sk_buff,
					      queue_mapping) > 0xff);
			off = offsetof(struct sk_buff, queue_mapping);
			emit(ARM_LDRH_I(r_A, r_skb, off), ctx);
			break;
		default:
			return -1;
		}
	}
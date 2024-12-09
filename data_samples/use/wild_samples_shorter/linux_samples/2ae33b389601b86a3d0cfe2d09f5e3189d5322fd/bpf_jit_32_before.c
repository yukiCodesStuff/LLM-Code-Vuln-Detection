			/* x = ((*(frame + k)) & 0xf) << 2; */
			ctx->seen |= SEEN_X | SEEN_DATA | SEEN_CALL;
			/* the interpreter should deal with the negative K */
			if (k < 0)
				return -1;
			/* offset in r1: we might have to take the slow path */
			emit_mov_i(r_off, k, ctx);
			emit(ARM_CMP_R(r_skb_hl, r_off), ctx);
	};
	u64 map_key_state; /* constant (32 bit) key tracking for maps */
	int ctx_field_size; /* the ctx field size for load insn, maybe 0 */
	int sanitize_stack_off; /* stack slot to be cleared */
	u32 seen; /* this insn was processed by the verifier at env->pass_cnt */
	bool zext_dst; /* this insn zero extends dst reg */
	u8 alu_state; /* used in combination with alu_limit */

	/* below fields are initialized once */
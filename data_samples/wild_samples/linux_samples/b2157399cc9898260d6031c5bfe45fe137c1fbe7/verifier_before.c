	if (err) {
		verbose(env, "kernel subsystem misconfigured func %s#%d\n",
			func_id_name(func_id), func_id);
		return err;
	}

	/* check args */
	err = check_func_arg(env, BPF_REG_1, fn->arg1_type, &meta);
	if (err)
		return err;
	err = check_func_arg(env, BPF_REG_2, fn->arg2_type, &meta);
	if (err)
		return err;
	err = check_func_arg(env, BPF_REG_3, fn->arg3_type, &meta);
	if (err)
		return err;
	err = check_func_arg(env, BPF_REG_4, fn->arg4_type, &meta);
	if (err)
		return err;
	err = check_func_arg(env, BPF_REG_5, fn->arg5_type, &meta);
	if (err)
		return err;

	/* Mark slots with STACK_MISC in case of raw mode, stack offset
	 * is inferred from register state.
	 */
	for (i = 0; i < meta.access_size; i++) {
		err = check_mem_access(env, insn_idx, meta.regno, i, BPF_B, BPF_WRITE, -1);
		if (err)
			return err;
	}
		if (insn->imm == BPF_FUNC_tail_call) {
			/* If we tail call into other programs, we
			 * cannot make any assumptions since they can
			 * be replaced dynamically during runtime in
			 * the program array.
			 */
			prog->cb_access = 1;
			env->prog->aux->stack_depth = MAX_BPF_STACK;

			/* mark bpf_tail_call as different opcode to avoid
			 * conditional branch in the interpeter for every normal
			 * call and to prevent accidental JITing by JIT compiler
			 * that doesn't support bpf_tail_call yet
			 */
			insn->imm = 0;
			insn->code = BPF_JMP | BPF_TAIL_CALL;
			continue;
		}

		/* BPF_EMIT_CALL() assumptions in some of the map_gen_lookup
		 * handlers are currently limited to 64 bit only.
		 */
		if (ebpf_jit_enabled() && BITS_PER_LONG == 64 &&
		    insn->imm == BPF_FUNC_map_lookup_elem) {
			map_ptr = env->insn_aux_data[i + delta].map_ptr;
			if (map_ptr == BPF_MAP_PTR_POISON ||
			    !map_ptr->ops->map_gen_lookup)
				goto patch_call_imm;

			cnt = map_ptr->ops->map_gen_lookup(map_ptr, insn_buf);
			if (cnt == 0 || cnt >= ARRAY_SIZE(insn_buf)) {
				verbose(env, "bpf verifier is misconfigured\n");
				return -EINVAL;
			}
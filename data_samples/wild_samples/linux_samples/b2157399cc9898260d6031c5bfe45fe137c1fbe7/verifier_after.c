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
	if (func_id == BPF_FUNC_tail_call) {
		if (meta.map_ptr == NULL) {
			verbose(env, "verifier bug\n");
			return -EINVAL;
		}
		env->insn_aux_data[insn_idx].map_ptr = meta.map_ptr;
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

			/* instead of changing every JIT dealing with tail_call
			 * emit two extra insns:
			 * if (index >= max_entries) goto out;
			 * index &= array->index_mask;
			 * to avoid out-of-bounds cpu speculation
			 */
			map_ptr = env->insn_aux_data[i + delta].map_ptr;
			if (map_ptr == BPF_MAP_PTR_POISON) {
				verbose(env, "tail_call obusing map_ptr\n");
				return -EINVAL;
			}
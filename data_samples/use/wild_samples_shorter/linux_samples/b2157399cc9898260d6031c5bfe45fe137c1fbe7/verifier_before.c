	err = check_func_arg(env, BPF_REG_2, fn->arg2_type, &meta);
	if (err)
		return err;
	err = check_func_arg(env, BPF_REG_3, fn->arg3_type, &meta);
	if (err)
		return err;
	err = check_func_arg(env, BPF_REG_4, fn->arg4_type, &meta);
			 */
			insn->imm = 0;
			insn->code = BPF_JMP | BPF_TAIL_CALL;
			continue;
		}

		/* BPF_EMIT_CALL() assumptions in some of the map_gen_lookup
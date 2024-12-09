	zend_hash_add(ctx->constants, Z_STR_P(name), &val);
}

zend_uchar zend_compound_assign_to_binary_op(zend_uchar opcode)
{
	switch (opcode) {
		case ZEND_ASSIGN_ADD: return ZEND_ADD;
		case ZEND_ASSIGN_SUB: return ZEND_SUB;
		case ZEND_ASSIGN_MUL: return ZEND_MUL;
		case ZEND_ASSIGN_DIV: return ZEND_DIV;
		case ZEND_ASSIGN_MOD: return ZEND_MOD;
		case ZEND_ASSIGN_SL: return ZEND_SL;
		case ZEND_ASSIGN_SR: return ZEND_SR;
		case ZEND_ASSIGN_CONCAT: return ZEND_CONCAT;
		case ZEND_ASSIGN_BW_OR: return ZEND_BW_OR;
		case ZEND_ASSIGN_BW_AND: return ZEND_BW_AND;
		case ZEND_ASSIGN_BW_XOR: return ZEND_BW_XOR;
		case ZEND_ASSIGN_POW: return ZEND_POW;
		EMPTY_SWITCH_DEFAULT_CASE()
	}
}

int zend_optimizer_eval_binary_op(zval *result, zend_uchar opcode, zval *op1, zval *op2) /* {{{ */
{
	binary_op_type binary_op = get_binary_op(opcode);
	int er, ret;
                                    zend_op       *opline,
                                    zval          *val)
{
	zend_op *target_opline;

	switch (opline->opcode) {
		case ZEND_JMPZ:
			if (zend_is_true(val)) {
				MAKE_NOP(opline);
			} else {
				opline->opcode = ZEND_JMP;
				COPY_NODE(opline->op1, opline->op2);
				opline->op2_type = IS_UNUSED;
			}
			zval_ptr_dtor_nogc(val);
			return 1;
		case ZEND_JMPNZ:
			if (zend_is_true(val)) {
				opline->opcode = ZEND_JMP;
				COPY_NODE(opline->op1, opline->op2);
				opline->op2_type = IS_UNUSED;
			} else {
				MAKE_NOP(opline);
			}
			zval_ptr_dtor_nogc(val);
			return 1;
		case ZEND_JMPZNZ:
			if (zend_is_true(val)) {
				target_opline = ZEND_OFFSET_TO_OPLINE(opline, opline->extended_value);
			} else {
				target_opline = ZEND_OP2_JMP_ADDR(opline);
			}
			ZEND_SET_OP_JMP_ADDR(opline, opline->op1, target_opline);
			opline->op1_type = IS_UNUSED;
			opline->extended_value = 0;
			opline->opcode = ZEND_JMP;
			zval_ptr_dtor_nogc(val);
			return 1;
		case ZEND_FREE:
			MAKE_NOP(opline);
			zval_ptr_dtor_nogc(val);
			return 1;
		case ZEND_SEND_VAR_EX:
		case ZEND_FETCH_DIM_W:
		case ZEND_FETCH_DIM_RW:
		case ZEND_FETCH_DIM_FUNC_ARG:
		case ZEND_FETCH_DIM_UNSET:
		case ZEND_ASSIGN_DIM:
		case ZEND_RETURN_BY_REF:
			return 0;
		case ZEND_INIT_STATIC_METHOD_CALL:
		case ZEND_CATCH:
		case ZEND_FETCH_CONSTANT:
		case ZEND_FETCH_CLASS_CONSTANT:
                                    zend_op       *opline,
                                    zval          *val)
{
	zval tmp;

	switch (opline->opcode) {
		case ZEND_ASSIGN_REF:
		case ZEND_FAST_CALL:
			return 0;
			break;
		case ZEND_INIT_FCALL:
			REQUIRES_STRING(val);
			if (Z_REFCOUNT_P(val) == 1) {
				zend_str_tolower(Z_STRVAL_P(val), Z_STRLEN_P(val));
			} else {
				ZVAL_STR(&tmp, zend_string_tolower(Z_STR_P(val)));
				zval_ptr_dtor_nogc(val);
				val = &tmp;
			}
			opline->op2.constant = zend_optimizer_add_literal(op_array, val);
			alloc_cache_slots_op2(op_array, opline, 1);
			break;
		case ZEND_INIT_DYNAMIC_CALL:
	}
}

void zend_optimizer_remove_live_range_ex(zend_op_array *op_array, uint32_t var, uint32_t start)
{
	uint32_t i = 0;

	while (i < op_array->last_live_range) {
		if (op_array->live_range[i].var == var
				&& op_array->live_range[i].start == start) {
			op_array->last_live_range--;
			if (i < op_array->last_live_range) {
				memmove(&op_array->live_range[i], &op_array->live_range[i+1], (op_array->last_live_range - i) * sizeof(zend_live_range));
			}
			break;
		}
		i++;
	}
}

int zend_optimizer_replace_by_const(zend_op_array *op_array,
                                    zend_op       *opline,
                                    zend_uchar     type,
                                    uint32_t       var,
	/* pass 11:
	 * - Compact literals table
	 */
	if ((ZEND_OPTIMIZER_PASS_11 & ctx->optimization_level) &&
	    (!(ZEND_OPTIMIZER_PASS_6 & ctx->optimization_level) ||
	     !(ZEND_OPTIMIZER_PASS_7 & ctx->optimization_level))) {
		zend_optimizer_compact_literals(op_array, ctx);
		if (ctx->debug_level & ZEND_DUMP_AFTER_PASS_11) {
			zend_dump_op_array(op_array, 0, "after pass 11", NULL);
		}
	}

	if ((ZEND_OPTIMIZER_PASS_13 & ctx->optimization_level) &&
	    (!(ZEND_OPTIMIZER_PASS_6 & ctx->optimization_level) ||
	     !(ZEND_OPTIMIZER_PASS_7 & ctx->optimization_level))) {
		zend_optimizer_compact_vars(op_array);
		if (ctx->debug_level & ZEND_DUMP_AFTER_PASS_13) {
			zend_dump_op_array(op_array, 0, "after pass 13", NULL);
		}
	}

	if (ZEND_OPTIMIZER_PASS_7 & ctx->optimization_level) {
		return;
	}

		for (i = 0; i < call_graph.op_arrays_count; i++) {
			func_info = ZEND_FUNC_INFO(call_graph.op_arrays[i]);
			if (func_info) {
				zend_dfa_optimize_op_array(call_graph.op_arrays[i], &ctx, &func_info->ssa, func_info->call_map);
			}
		}

		if (debug_level & ZEND_DUMP_AFTER_PASS_7) {
			}
		}

		if (ZEND_OPTIMIZER_PASS_11 & optimization_level) {
			for (i = 0; i < call_graph.op_arrays_count; i++) {
				zend_optimizer_compact_literals(call_graph.op_arrays[i], &ctx);
				if (debug_level & ZEND_DUMP_AFTER_PASS_11) {
					zend_dump_op_array(call_graph.op_arrays[i], 0, "after pass 11", NULL);
				}
			}
		}

		if (ZEND_OPTIMIZER_PASS_13 & optimization_level) {
			for (i = 0; i < call_graph.op_arrays_count; i++) {
				zend_optimizer_compact_vars(call_graph.op_arrays[i]);
				if (debug_level & ZEND_DUMP_AFTER_PASS_13) {
					zend_dump_op_array(call_graph.op_arrays[i], 0, "after pass 13", NULL);
				}
			}
		}

		if (ZEND_OPTIMIZER_PASS_12 & optimization_level) {
			for (i = 0; i < call_graph.op_arrays_count; i++) {
				zend_adjust_fcall_stack_size_graph(call_graph.op_arrays[i]);
			}
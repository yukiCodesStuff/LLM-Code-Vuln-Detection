	zend_hash_add(ctx->constants, Z_STR_P(name), &val);
}

int zend_optimizer_eval_binary_op(zval *result, zend_uchar opcode, zval *op1, zval *op2) /* {{{ */
{
	binary_op_type binary_op = get_binary_op(opcode);
	int er, ret;
                                    zend_op       *opline,
                                    zval          *val)
{
	switch (opline->opcode) {
		case ZEND_FREE:
			MAKE_NOP(opline);
			zval_ptr_dtor_nogc(val);
			return 1;
		case ZEND_INIT_STATIC_METHOD_CALL:
		case ZEND_CATCH:
		case ZEND_FETCH_CONSTANT:
		case ZEND_FETCH_CLASS_CONSTANT:
                                    zend_op       *opline,
                                    zval          *val)
{
	switch (opline->opcode) {
		case ZEND_ASSIGN_REF:
		case ZEND_FAST_CALL:
			return 0;
			break;
		case ZEND_INIT_FCALL:
			REQUIRES_STRING(val);
			zend_str_tolower(Z_STRVAL_P(val), Z_STRLEN_P(val));
			opline->op2.constant = zend_optimizer_add_literal(op_array, val);
			alloc_cache_slots_op2(op_array, opline, 1);
			break;
		case ZEND_INIT_DYNAMIC_CALL:
	}
}

int zend_optimizer_replace_by_const(zend_op_array *op_array,
                                    zend_op       *opline,
                                    zend_uchar     type,
                                    uint32_t       var,
	/* pass 11:
	 * - Compact literals table
	 */
	if (ZEND_OPTIMIZER_PASS_11 & ctx->optimization_level) {
		zend_optimizer_compact_literals(op_array, ctx);
		if (ctx->debug_level & ZEND_DUMP_AFTER_PASS_11) {
			zend_dump_op_array(op_array, 0, "after pass 11", NULL);
		}
	}

	if (ZEND_OPTIMIZER_PASS_7 & ctx->optimization_level) {
		return;
	}

		for (i = 0; i < call_graph.op_arrays_count; i++) {
			func_info = ZEND_FUNC_INFO(call_graph.op_arrays[i]);
			if (func_info) {
				zend_dfa_optimize_op_array(call_graph.op_arrays[i], &ctx, &func_info->ssa);
			}
		}

		if (debug_level & ZEND_DUMP_AFTER_PASS_7) {
			}
		}

		if (ZEND_OPTIMIZER_PASS_12 & optimization_level) {
			for (i = 0; i < call_graph.op_arrays_count; i++) {
				zend_adjust_fcall_stack_size_graph(call_graph.op_arrays[i]);
			}
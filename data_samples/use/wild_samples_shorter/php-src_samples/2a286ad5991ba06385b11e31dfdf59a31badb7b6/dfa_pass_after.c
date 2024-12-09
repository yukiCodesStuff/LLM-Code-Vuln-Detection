#include "zend_inference.h"
#include "zend_dump.h"

#ifndef ZEND_DEBUG_DFA
# define ZEND_DEBUG_DFA ZEND_DEBUG
#endif

#if ZEND_DEBUG_DFA
# include "ssa_integrity.c"
#endif

int zend_dfa_analyze_op_array(zend_op_array *op_array, zend_optimizer_ctx *ctx, zend_ssa *ssa, uint32_t *flags)
{
	uint32_t build_flags;

	}

	if (ctx->debug_level & ZEND_DUMP_DFA_SSA) {
		zend_dump_op_array(op_array, ZEND_DUMP_SSA, "dfa ssa", ssa);
	}


	if (zend_ssa_compute_use_def_chains(&ctx->arena, op_array, ssa) != SUCCESS){
					if (i != target) {
						op_array->opcodes[target] = op_array->opcodes[i];
						ssa->ops[target] = ssa->ops[i];
						ssa->cfg.map[target] = b - blocks;
					}
					target++;
				}
				i++;
				new_opline = op_array->opcodes + target - 1;
				zend_optimizer_migrate_jump(op_array, new_opline, opline);
			}
		} else {
			b->start = target;
			b->len = 0;
		}
	}

	if (target != op_array->last) {
	return 1;
}

int zend_dfa_optimize_calls(zend_op_array *op_array, zend_ssa *ssa)
{
	zend_func_info *func_info = ZEND_FUNC_INFO(op_array);
	int removed_ops = 0;

	if (func_info->callee_info) {
		zend_call_info *call_info = func_info->callee_info;

		do {
			if (call_info->caller_call_opline->opcode == ZEND_DO_ICALL
			 && call_info->callee_func
			 && ZSTR_LEN(call_info->callee_func->common.function_name) == sizeof("in_array")-1
			 && memcmp(ZSTR_VAL(call_info->callee_func->common.function_name), "in_array", sizeof("in_array")-1) == 0
			 && (call_info->caller_init_opline->extended_value == 2
			  || (call_info->caller_init_opline->extended_value == 3
			   && (call_info->caller_call_opline - 1)->opcode == ZEND_SEND_VAL
			   && (call_info->caller_call_opline - 1)->op1_type == IS_CONST))) {

				zend_op *send_array;
				zend_op *send_needly;
				zend_bool strict = 0;

				if (call_info->caller_init_opline->extended_value == 2) {
					send_array = call_info->caller_call_opline - 1;
					send_needly = call_info->caller_call_opline - 2;
				} else {
					if (zend_is_true(CT_CONSTANT_EX(op_array, (call_info->caller_call_opline - 1)->op1.constant))) {
						strict = 1;
					}
					send_array = call_info->caller_call_opline - 2;
					send_needly = call_info->caller_call_opline - 3;
				}

				if (send_array->opcode == ZEND_SEND_VAL
				 && send_array->op1_type == IS_CONST
				 && Z_TYPE_P(CT_CONSTANT_EX(op_array, send_array->op1.constant)) == IS_ARRAY
				 && (send_needly->opcode == ZEND_SEND_VAL
				  || send_needly->opcode == ZEND_SEND_VAR)
			    ) {
					int ok = 1;

					HashTable *src = Z_ARRVAL_P(CT_CONSTANT_EX(op_array, send_array->op1.constant));
					HashTable *dst;
					zval *val, tmp;
					zend_ulong idx;

					ZVAL_TRUE(&tmp);
					dst = emalloc(sizeof(HashTable));
					zend_hash_init(dst, zend_hash_num_elements(src), NULL, ZVAL_PTR_DTOR, 0);
					if (strict) {
						ZEND_HASH_FOREACH_VAL(src, val) {
							if (Z_TYPE_P(val) == IS_STRING) {
								zend_hash_add(dst, Z_STR_P(val), &tmp);
							} else if (Z_TYPE_P(val) == IS_LONG) {
								zend_hash_index_add(dst, Z_LVAL_P(val), &tmp);
							} else {
								zend_array_destroy(dst);
								ok = 0;
								break;
							}
						} ZEND_HASH_FOREACH_END();
					} else {
						ZEND_HASH_FOREACH_VAL(src, val) {
							if (Z_TYPE_P(val) != IS_STRING || ZEND_HANDLE_NUMERIC(Z_STR_P(val), idx)) {
								zend_array_destroy(dst);
								ok = 0;
								break;
							}
							zend_hash_add(dst, Z_STR_P(val), &tmp);
						} ZEND_HASH_FOREACH_END();
					}

					if (ok) {
						uint32_t op_num = send_needly - op_array->opcodes;
						zend_ssa_op *ssa_op = ssa->ops + op_num;

						if (ssa_op->op1_use >= 0) {
							/* Reconstruct SSA */
							int var_num = ssa_op->op1_use;
							zend_ssa_var *var = ssa->vars + var_num;

							ZEND_ASSERT(ssa_op->op1_def < 0);
							zend_ssa_unlink_use_chain(ssa, op_num, ssa_op->op1_use);
							ssa_op->op1_use = -1;
							ssa_op->op1_use_chain = -1;
							op_num = call_info->caller_call_opline - op_array->opcodes;
							ssa_op = ssa->ops + op_num;
							ssa_op->op1_use = var_num;
							ssa_op->op1_use_chain = var->use_chain;
							var->use_chain = op_num;
						}

						ZVAL_ARR(&tmp, dst);

						/* Update opcode */
						call_info->caller_call_opline->opcode = ZEND_IN_ARRAY;
						call_info->caller_call_opline->extended_value = strict;
						call_info->caller_call_opline->op1_type = send_needly->op1_type;
						call_info->caller_call_opline->op1.num = send_needly->op1.num;
						call_info->caller_call_opline->op2_type = IS_CONST;
						call_info->caller_call_opline->op2.constant = zend_optimizer_add_literal(op_array, &tmp);
						if (call_info->caller_init_opline->extended_value == 3) {
							MAKE_NOP(call_info->caller_call_opline - 1);
						}
						MAKE_NOP(call_info->caller_init_opline);
						MAKE_NOP(send_needly);
						MAKE_NOP(send_array);
						removed_ops++;

					}
				}
			}
			call_info = call_info->next_callee;
		} while (call_info);
	}

	return removed_ops;
}

void zend_dfa_optimize_op_array(zend_op_array *op_array, zend_optimizer_ctx *ctx, zend_ssa *ssa, zend_call_info **call_map)
{
	if (ctx->debug_level & ZEND_DUMP_BEFORE_DFA_PASS) {
		zend_dump_op_array(op_array, ZEND_DUMP_SSA, "before dfa pass", ssa);
	}
		zend_op *opline;
		zval tmp;

		if (ZEND_OPTIMIZER_PASS_8 & ctx->optimization_level) {
			if (sccp_optimize_op_array(ctx, op_array, ssa, call_map)) {
				remove_nops = 1;
			}
#if ZEND_DEBUG_DFA
			ssa_verify_integrity(op_array, ssa, "after sccp");
#endif
			if (ZEND_FUNC_INFO(op_array)) {
				if (zend_dfa_optimize_calls(op_array, ssa)) {
					remove_nops = 1;
				}
			}
			if (ctx->debug_level & ZEND_DUMP_AFTER_PASS_8) {
				zend_dump_op_array(op_array, ZEND_DUMP_SSA, "after sccp pass", ssa);
			}
#if ZEND_DEBUG_DFA
			ssa_verify_integrity(op_array, ssa, "after calls");
#endif
		}

		if (ZEND_OPTIMIZER_PASS_14 & ctx->optimization_level) {
			if (dce_optimize_op_array(op_array, ssa, 0)) {
				remove_nops = 1;
			}
			if (ctx->debug_level & ZEND_DUMP_AFTER_PASS_14) {
				zend_dump_op_array(op_array, ZEND_DUMP_SSA, "after dce pass", ssa);
			}
#if ZEND_DEBUG_DFA
			ssa_verify_integrity(op_array, ssa, "after dce");
#endif
		}

		for (v = op_array->last_var; v < ssa->vars_count; v++) {

			op_1 = ssa->vars[v].definition;


// op_1: ASSIGN #orig_var.CV [undef,scalar] -> #v.CV, CONST|TMPVAR => QM_ASSIGN v.CV, CONST|TMPVAR

						if (ssa->ops[op_1].op1_use != ssa->ops[op_1].op2_use) {
							zend_ssa_unlink_use_chain(ssa, op_1, orig_var);
						} else {
							ssa->ops[op_1].op2_use_chain = ssa->ops[op_1].op1_use_chain;
						}

						/* Reconstruct SSA */
						ssa->ops[op_1].result_def = v;
						ssa->ops[op_1].op1_def = -1;
						ssa->ops[op_1].op1_use = ssa->ops[op_1].op2_use;
						ssa->ops[op_1].op1_use_chain = ssa->ops[op_1].op2_use_chain;
						ssa->ops[op_1].op2_use = -1;
						ssa->ops[op_1].op2_use_chain = -1;

						/* Update opcode */
						opline->result_type = opline->op1_type;
						opline->result.var = opline->op1.var;
						opline->op1_type = opline->op2_type;
						opline->op1.var = opline->op2.var;
						opline->op2_type = IS_UNUSED;
						opline->op2.var = 0;
						opline->opcode = ZEND_QM_ASSIGN;
					}
				}

			} else if (opline->opcode == ZEND_ASSIGN_ADD
			}
		}

#if ZEND_DEBUG_DFA
		ssa_verify_integrity(op_array, ssa, "after dfa");
#endif

		if (remove_nops) {
			zend_ssa_remove_nops(op_array, ssa);
#if ZEND_DEBUG_DFA
			ssa_verify_integrity(op_array, ssa, "after nop");
#endif
		}
	}

	if (ctx->debug_level & ZEND_DUMP_AFTER_DFA_PASS) {
		return;
	}

	zend_dfa_optimize_op_array(op_array, ctx, &ssa, NULL);

	/* Destroy SSA */
	zend_arena_release(&ctx->arena, checkpoint);
}
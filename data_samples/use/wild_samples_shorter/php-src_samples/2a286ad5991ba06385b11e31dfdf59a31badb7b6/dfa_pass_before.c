#include "zend_inference.h"
#include "zend_dump.h"

int zend_dfa_analyze_op_array(zend_op_array *op_array, zend_optimizer_ctx *ctx, zend_ssa *ssa, uint32_t *flags)
{
	uint32_t build_flags;

	}

	if (ctx->debug_level & ZEND_DUMP_DFA_SSA) {
		zend_dump_op_array(op_array, ZEND_DUMP_SSA, "before dfa pass", ssa);
	}


	if (zend_ssa_compute_use_def_chains(&ctx->arena, op_array, ssa) != SUCCESS){
					if (i != target) {
						op_array->opcodes[target] = op_array->opcodes[i];
						ssa->ops[target] = ssa->ops[i];
					}
					target++;
				}
				i++;
				new_opline = op_array->opcodes + target - 1;
				zend_optimizer_migrate_jump(op_array, new_opline, opline);
			}
		}
	}

	if (target != op_array->last) {
	return 1;
}

void zend_dfa_optimize_op_array(zend_op_array *op_array, zend_optimizer_ctx *ctx, zend_ssa *ssa)
{
	if (ctx->debug_level & ZEND_DUMP_BEFORE_DFA_PASS) {
		zend_dump_op_array(op_array, ZEND_DUMP_SSA, "before dfa pass", ssa);
	}
		zend_op *opline;
		zval tmp;

		for (v = op_array->last_var; v < ssa->vars_count; v++) {

			op_1 = ssa->vars[v].definition;


// op_1: ASSIGN #orig_var.CV [undef,scalar] -> #v.CV, CONST|TMPVAR => QM_ASSIGN v.CV, CONST|TMPVAR

						if (zend_ssa_unlink_use_chain(ssa, op_1, orig_var)) {
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
				}

			} else if (opline->opcode == ZEND_ASSIGN_ADD
			}
		}

		if (remove_nops) {
			zend_ssa_remove_nops(op_array, ssa);
		}
	}

	if (ctx->debug_level & ZEND_DUMP_AFTER_DFA_PASS) {
		return;
	}

	zend_dfa_optimize_op_array(op_array, ctx, &ssa);

	/* Destroy SSA */
	zend_arena_release(&ctx->arena, checkpoint);
}
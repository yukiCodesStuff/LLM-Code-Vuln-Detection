   | license@php.net so we can mail you a copy immediately.               |
   +----------------------------------------------------------------------+
   | Authors: Dmitry Stogov <dmitry@zend.com>                             |
   |          Nikita Popov <nikic@php.net>                                |
   +----------------------------------------------------------------------+
*/

#include "php.h"
#include "zend_ssa.h"
#include "zend_dump.h"
#include "zend_inference.h"
#include "Optimizer/zend_optimizer_internal.h"

static zend_bool dominates(const zend_basic_block *blocks, int a, int b) {
	while (blocks[b].level > blocks[a].level) {
		b = blocks[b].idom;
			ssa_vars[phi->ssa_var].var = phi->var;
			ssa_vars[phi->ssa_var].definition_phi = phi;
			if (phi->pi >= 0) {
				zend_ssa_phi *p;

				ZEND_ASSERT(phi->sources[0] >= 0);
				p = ssa_vars[phi->sources[0]].phi_use_chain;
				while (p && p != phi) {
					p = zend_ssa_next_use_phi(ssa, phi->sources[0], p);
				}
				if (!p) {
					phi->use_chains[0] = ssa_vars[phi->sources[0]].phi_use_chain;
					ssa_vars[phi->sources[0]].phi_use_chain = phi;
				}
				if (phi->has_range_constraint) {
					/* min and max variables can't be used together */
					zend_ssa_range_constraint *constraint = &phi->constraint.range;
				int j;

				for (j = 0; j < ssa->cfg.blocks[i].predecessors_count; j++) {
					zend_ssa_phi *p;

					ZEND_ASSERT(phi->sources[j] >= 0);
					p = ssa_vars[phi->sources[j]].phi_use_chain;
					while (p && p != phi) {
						p = zend_ssa_next_use_phi(ssa, phi->sources[j], p);
					}
					if (!p) {
						phi->use_chains[j] = ssa_vars[phi->sources[j]].phi_use_chain;
						ssa_vars[phi->sources[j]].phi_use_chain = phi;
					}
				}
			}
			phi = phi->next;
}
/* }}} */

void zend_ssa_remove_instr(zend_ssa *ssa, zend_op *opline, zend_ssa_op *ssa_op) /* {{{ */
{
	if (ssa_op->result_use >= 0) {
		zend_ssa_unlink_use_chain(ssa, ssa_op - ssa->ops, ssa_op->result_use);
		ssa_op->result_use = -1;
		ssa_op->res_use_chain = -1;
	}
	if (ssa_op->op1_use >= 0) {
		if (ssa_op->op1_use != ssa_op->op2_use) {
			zend_ssa_unlink_use_chain(ssa, ssa_op - ssa->ops, ssa_op->op1_use);
		} else {
			ssa_op->op2_use_chain = ssa_op->op1_use_chain;
		}
		ssa_op->op1_use = -1;
		ssa_op->op1_use_chain = -1;
	}
	if (ssa_op->op2_use >= 0) {
		zend_ssa_unlink_use_chain(ssa, ssa_op - ssa->ops, ssa_op->op2_use);
		ssa_op->op2_use = -1;
		ssa_op->op2_use_chain = -1;
	}

	/* We let the caller make sure that all defs are gone */
	ZEND_ASSERT(ssa_op->result_def == -1);
	ZEND_ASSERT(ssa_op->op1_def == -1);
	ZEND_ASSERT(ssa_op->op2_def == -1);

	MAKE_NOP(opline);
}
/* }}} */

static inline zend_ssa_phi **zend_ssa_next_use_phi_ptr(zend_ssa *ssa, int var, zend_ssa_phi *p) /* {{{ */
{
	if (p->pi >= 0) {
		return &p->use_chains[0];
	} else {
		int j;
		for (j = 0; j < ssa->cfg.blocks[p->block].predecessors_count; j++) {
			if (p->sources[j] == var) {
				return &p->use_chains[j];
			}
		}
	}
	ZEND_ASSERT(0);
	return NULL;
}
/* }}} */

/* May be called even if source is not used in the phi (useful when removing uses in a phi
 * with multiple identical operands) */
static inline void zend_ssa_remove_use_of_phi_source(zend_ssa *ssa, zend_ssa_phi *phi, int source) /* {{{ */
{
	zend_ssa_phi **cur = &ssa->vars[source].phi_use_chain;
	while (*cur && *cur != phi) {
		cur = zend_ssa_next_use_phi_ptr(ssa, source, *cur);
	}
	if (*cur) {
		*cur = zend_ssa_next_use_phi(ssa, source, *cur);
	}
}
/* }}} */

static void zend_ssa_remove_uses_of_phi_sources(zend_ssa *ssa, zend_ssa_phi *phi) /* {{{ */
{
	int source;
	FOREACH_PHI_SOURCE(phi, source) {
		zend_ssa_remove_use_of_phi_source(ssa, phi, source);
	} FOREACH_PHI_SOURCE_END();
}
/* }}} */

static void zend_ssa_remove_phi_from_block(zend_ssa *ssa, zend_ssa_phi *phi) /* {{{ */
{
	zend_ssa_block *block = &ssa->blocks[phi->block];
	zend_ssa_phi **cur = &block->phis;
	while (*cur != phi) {
		ZEND_ASSERT(*cur != NULL);
		cur = &(*cur)->next;
	}
	*cur = (*cur)->next;
}
/* }}} */

static inline void zend_ssa_remove_defs_of_instr(zend_ssa *ssa, zend_ssa_op *ssa_op) /* {{{ */
{
	if (ssa_op->op1_def >= 0) {
		zend_ssa_remove_uses_of_var(ssa, ssa_op->op1_def);
		zend_ssa_remove_op1_def(ssa, ssa_op);
	}
	if (ssa_op->op2_def >= 0) {
		zend_ssa_remove_uses_of_var(ssa, ssa_op->op2_def);
		zend_ssa_remove_op2_def(ssa, ssa_op);
	}
	if (ssa_op->result_def >= 0) {
		zend_ssa_remove_uses_of_var(ssa, ssa_op->result_def);
		zend_ssa_remove_result_def(ssa, ssa_op);
	}
}
/* }}} */

static inline void zend_ssa_remove_phi_source(zend_ssa *ssa, zend_ssa_phi *phi, int pred_offset, int predecessors_count) /* {{{ */
{
	int j, var_num = phi->sources[pred_offset];

	predecessors_count--;
	if (pred_offset < predecessors_count) {
		memmove(phi->sources + pred_offset, phi->sources + pred_offset + 1, (predecessors_count - pred_offset) * sizeof(uint32_t));
	}

	/* Check if they same var is used in a different phi operand as well, in this case we don't
	 * need to adjust the use chain (but may have to move the next pointer). */
	for (j = 0; j < predecessors_count; j++) {
		if (phi->sources[j] == var_num) {
			if (j < pred_offset) {
				ZEND_ASSERT(phi->use_chains[pred_offset] == NULL);
				return;
			}
			if (j >= pred_offset) {
				phi->use_chains[j] = phi->use_chains[pred_offset];
				phi->use_chains[pred_offset] = NULL;
				return;
			}
		}
	}

	/* Variable only used in one operand, remove the phi from the use chain. */
	zend_ssa_remove_use_of_phi_source(ssa, phi, var_num);
	phi->use_chains[pred_offset] = NULL;
}
/* }}} */

void zend_ssa_remove_phi(zend_ssa *ssa, zend_ssa_phi *phi) /* {{{ */
{
	ZEND_ASSERT(phi->ssa_var >= 0);
	ZEND_ASSERT(ssa->vars[phi->ssa_var].use_chain < 0
		&& ssa->vars[phi->ssa_var].phi_use_chain == NULL);
	zend_ssa_remove_uses_of_phi_sources(ssa, phi);
	zend_ssa_remove_phi_from_block(ssa, phi);
	ssa->vars[phi->ssa_var].definition_phi = NULL;
	phi->ssa_var = -1;
}
/* }}} */

void zend_ssa_remove_uses_of_var(zend_ssa *ssa, int var_num) /* {{{ */
{
	zend_ssa_var *var = &ssa->vars[var_num];
	zend_ssa_phi *phi;
	int use;
	FOREACH_PHI_USE(var, phi) {
		int i, end = NUM_PHI_SOURCES(phi);
		for (i = 0; i < end; i++) {
			if (phi->sources[i] == var_num) {
				phi->use_chains[i] = NULL;
			}
		}
	} FOREACH_PHI_USE_END();
	var->phi_use_chain = NULL;
	FOREACH_USE(var, use) {
		zend_ssa_op *ssa_op = &ssa->ops[use];
		if (ssa_op->op1_use == var_num) {
			ssa_op->op1_use = -1;
			ssa_op->op1_use_chain = -1;
		}
		if (ssa_op->op2_use == var_num) {
			ssa_op->op2_use = -1;
			ssa_op->op2_use_chain = -1;
		}
		if (ssa_op->result_use == var_num) {
			ssa_op->result_use = -1;
			ssa_op->res_use_chain = -1;
		}
	} FOREACH_USE_END();
	var->use_chain = -1;
}
/* }}} */

void zend_ssa_remove_block(zend_op_array *op_array, zend_ssa *ssa, int i) /* {{{ */
{
	zend_basic_block *block = &ssa->cfg.blocks[i];
	zend_ssa_block *ssa_block = &ssa->blocks[i];
	int *predecessors;
	zend_ssa_phi *phi;
	int j, s;

	block->flags &= ~ZEND_BB_REACHABLE;

	/* Removes phis in this block */
	for (phi = ssa_block->phis; phi; phi = phi->next) {
		zend_ssa_remove_uses_of_var(ssa, phi->ssa_var);
		zend_ssa_remove_phi(ssa, phi);
	}

	/* Remove instructions in this block */
	for (j = block->start; j < block->start + block->len; j++) {
		if (op_array->opcodes[j].opcode == ZEND_NOP) {
			continue;
		}

		if (op_array->opcodes[j].result_type & (IS_TMP_VAR|IS_VAR)) {
			zend_optimizer_remove_live_range_ex(op_array, op_array->opcodes[j].result.var, j + 1);
		}
		zend_ssa_remove_defs_of_instr(ssa, &ssa->ops[j]);
		zend_ssa_remove_instr(ssa, &op_array->opcodes[j], &ssa->ops[j]);
	}

	for (s = 0; s < block->successors_count; s++) {
		zend_basic_block *next_block = &ssa->cfg.blocks[block->successors[s]];
		zend_ssa_block *next_ssa_block = &ssa->blocks[block->successors[s]];
		zend_ssa_phi *phi;

		/* Find at which predecessor offset this block is referenced */
		int pred_offset = -1;
		predecessors = &ssa->cfg.predecessors[next_block->predecessor_offset];
		for (j = 0; j < next_block->predecessors_count; j++) {
			if (predecessors[j] == i) {
				pred_offset = j;
				break;
			}
		}
		ZEND_ASSERT(pred_offset != -1);

		/* For phis in successor blocks, remove the operands associated with this block */
		for (phi = next_ssa_block->phis; phi; phi = phi->next) {
			if (phi->pi >= 0) {
				if (phi->pi == i) {
					zend_ssa_remove_uses_of_var(ssa, phi->ssa_var);
					zend_ssa_remove_phi(ssa, phi);
				}
			} else {
				ZEND_ASSERT(phi->sources[pred_offset] >= 0);
				zend_ssa_remove_phi_source(ssa, phi, pred_offset, next_block->predecessors_count);
			}
		}

		/* Remove this predecessor */
		next_block->predecessors_count--;
		if (pred_offset < next_block->predecessors_count) {
			predecessors = &ssa->cfg.predecessors[next_block->predecessor_offset + pred_offset];
			memmove(predecessors, predecessors + 1, (next_block->predecessors_count - pred_offset) * sizeof(uint32_t));
		}
	}

	/* Remove successors of predecessors */
	predecessors = &ssa->cfg.predecessors[block->predecessor_offset];
	for (j = 0; j < block->predecessors_count; j++) {
		if (predecessors[j] >= 0) {
			zend_basic_block *prev_block = &ssa->cfg.blocks[predecessors[j]];

			for (s = 0; s < prev_block->successors_count; s++) {
				if (prev_block->successors[s] == i) {
					memmove(prev_block->successors + s,
							prev_block->successors + s + 1,
							sizeof(int) * (prev_block->successors_count - s - 1));
					prev_block->successors_count--;
					s--;
				}
			}
		}
	}

	block->successors_count = 0;
	block->predecessors_count = 0;

	/* Remove from dominators tree */
	if (block->idom >= 0) {
		j = ssa->cfg.blocks[block->idom].children;
		if (j == i) {
			ssa->cfg.blocks[block->idom].children = block->next_child;
		} else if (j >= 0) {
			while (ssa->cfg.blocks[j].next_child >= 0) {
				if (ssa->cfg.blocks[j].next_child == i) {
					ssa->cfg.blocks[j].next_child = block->next_child;
					break;
				}
				j = ssa->cfg.blocks[j].next_child;
			}
		}
	}
	block->idom = -1;
	block->level = -1;
	block->children = -1;
	block->next_child = -1;
}
/* }}} */

static void propagate_phi_type_widening(zend_ssa *ssa, int var) /* {{{ */
{
	zend_ssa_phi *phi;
	FOREACH_PHI_USE(&ssa->vars[var], phi) {
		if (ssa->var_info[var].type & ~ssa->var_info[phi->ssa_var].type) {
			ssa->var_info[phi->ssa_var].type |= ssa->var_info[var].type;
			propagate_phi_type_widening(ssa, phi->ssa_var);
		}
	} FOREACH_PHI_USE_END();
}
/* }}} */

void zend_ssa_rename_var_uses(zend_ssa *ssa, int old, int new, zend_bool update_types) /* {{{ */
{
	zend_ssa_var *old_var = &ssa->vars[old];
	zend_ssa_var *new_var = &ssa->vars[new];
	int use;
	zend_ssa_phi *phi;

	ZEND_ASSERT(old >= 0 && new >= 0);
	ZEND_ASSERT(old != new);

	/* Only a no_val is both variables are */
	new_var->no_val &= old_var->no_val;

	/* Update ssa_op use chains */
	FOREACH_USE(old_var, use) {
		zend_ssa_op *ssa_op = &ssa->ops[use];

		/* If the op already uses the new var, don't add the op to the use
		 * list again. Instead move the use_chain to the correct operand. */
		zend_bool add_to_use_chain = 1;
		if (ssa_op->result_use == new) {
			add_to_use_chain = 0;
		} else if (ssa_op->op1_use == new) {
			if (ssa_op->result_use == old) {
				ssa_op->res_use_chain = ssa_op->op1_use_chain;
				ssa_op->op1_use_chain = -1;
			}
			add_to_use_chain = 0;
		} else if (ssa_op->op2_use == new) {
			if (ssa_op->result_use == old) {
				ssa_op->res_use_chain = ssa_op->op2_use_chain;
				ssa_op->op2_use_chain = -1;
			} else if (ssa_op->op1_use == old) {
				ssa_op->op1_use_chain = ssa_op->op2_use_chain;
				ssa_op->op2_use_chain = -1;
			}
			add_to_use_chain = 0;
		}

		/* Perform the actual renaming */
		if (ssa_op->result_use == old) {
			ssa_op->result_use = new;
		}
		if (ssa_op->op1_use == old) {
			ssa_op->op1_use = new;
		}
		if (ssa_op->op2_use == old) {
			ssa_op->op2_use = new;
		}

		/* Add op to use chain of new var (if it isn't already). We use the
		 * first use chain of (result, op1, op2) that has the new variable. */
		if (add_to_use_chain) {
			if (ssa_op->result_use == new) {
				ssa_op->res_use_chain = new_var->use_chain;
				new_var->use_chain = use;
			} else if (ssa_op->op1_use == new) {
				ssa_op->op1_use_chain = new_var->use_chain;
				new_var->use_chain = use;
			} else {
				ZEND_ASSERT(ssa_op->op2_use == new);
				ssa_op->op2_use_chain = new_var->use_chain;
				new_var->use_chain = use;
			}
		}
	} FOREACH_USE_END();
	old_var->use_chain = -1;

	/* Update phi use chains */
	FOREACH_PHI_USE(old_var, phi) {
		int j;
		zend_bool after_first_new_source = 0;

		/* If the phi already uses the new var, find its use chain, as we may
		 * need to move it to a different source operand. */
		zend_ssa_phi **existing_use_chain_ptr = NULL;
		for (j = 0; j < ssa->cfg.blocks[phi->block].predecessors_count; j++) {
			if (phi->sources[j] == new) {
				existing_use_chain_ptr = &phi->use_chains[j];
				break;
			}
		}

		for (j = 0; j < ssa->cfg.blocks[phi->block].predecessors_count; j++) {
			if (phi->sources[j] == new) {
				after_first_new_source = 1;
			} else if (phi->sources[j] == old) {
				phi->sources[j] = new;

				/* Either move existing use chain to this source, or add the phi
				 * to the phi use chain of the new variables. Do this only once. */
				if (!after_first_new_source) {
					if (existing_use_chain_ptr) {
						phi->use_chains[j] = *existing_use_chain_ptr;
						*existing_use_chain_ptr = NULL;
					} else {
						phi->use_chains[j] = new_var->phi_use_chain;
						new_var->phi_use_chain = phi;
					}
					after_first_new_source = 1;
				}
			}
		}

		/* Make sure phi result types are not incorrectly narrow after renaming.
		 * This should not normally happen, but can occur if we DCE an assignment
		 * or unset and there is an improper phi-indirected use lateron. */
		// TODO Alternatively we could rerun type-inference after DCE
		if (update_types && (ssa->var_info[new].type & ~ssa->var_info[phi->ssa_var].type)) {
			ssa->var_info[phi->ssa_var].type |= ssa->var_info[new].type;
			propagate_phi_type_widening(ssa, phi->ssa_var);
		}
	} FOREACH_PHI_USE_END();
	old_var->phi_use_chain = NULL;
}
/* }}} */

/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
int zend_ssa_compute_use_def_chains(zend_arena **arena, const zend_op_array *op_array, zend_ssa *ssa);
int zend_ssa_unlink_use_chain(zend_ssa *ssa, int op, int var);

void zend_ssa_remove_instr(zend_ssa *ssa, zend_op *opline, zend_ssa_op *ssa_op);
void zend_ssa_remove_phi(zend_ssa *ssa, zend_ssa_phi *phi);
void zend_ssa_remove_uses_of_var(zend_ssa *ssa, int var_num);
void zend_ssa_remove_block(zend_op_array *op_array, zend_ssa *ssa, int b);
void zend_ssa_rename_var_uses(zend_ssa *ssa, int old_var, int new_var, zend_bool update_types);

static zend_always_inline void _zend_ssa_remove_def(zend_ssa_var *var)
{
	ZEND_ASSERT(var->definition >= 0);
	ZEND_ASSERT(var->use_chain < 0);
	ZEND_ASSERT(!var->phi_use_chain);
	var->definition = -1;
}

static zend_always_inline void zend_ssa_remove_result_def(zend_ssa *ssa, zend_ssa_op *ssa_op)
{
	zend_ssa_var *var = &ssa->vars[ssa_op->result_def];
	_zend_ssa_remove_def(var);
	ssa_op->result_def = -1;
}

static zend_always_inline void zend_ssa_remove_op1_def(zend_ssa *ssa, zend_ssa_op *ssa_op)
{
	zend_ssa_var *var = &ssa->vars[ssa_op->op1_def];
	_zend_ssa_remove_def(var);
	ssa_op->op1_def = -1;
}

static zend_always_inline void zend_ssa_remove_op2_def(zend_ssa *ssa, zend_ssa_op *ssa_op)
{
	zend_ssa_var *var = &ssa->vars[ssa_op->op2_def];
	_zend_ssa_remove_def(var);
	ssa_op->op2_def = -1;
}

END_EXTERN_C()

static zend_always_inline int zend_ssa_next_use(const zend_ssa_op *ssa_op, int var, int use)
{
	return 0;
}

static zend_always_inline void zend_ssa_rename_defs_of_instr(zend_ssa *ssa, zend_ssa_op *ssa_op) {
	/* Rename def to use if possible. Mark variable as not defined otherwise. */
	if (ssa_op->op1_def >= 0) {
		if (ssa_op->op1_use >= 0) {
			zend_ssa_rename_var_uses(ssa, ssa_op->op1_def, ssa_op->op1_use, 1);
		}
		ssa->vars[ssa_op->op1_def].definition = -1;
		ssa_op->op1_def = -1;
	}
	if (ssa_op->op2_def >= 0) {
		if (ssa_op->op2_use >= 0) {
			zend_ssa_rename_var_uses(ssa, ssa_op->op2_def, ssa_op->op2_use, 1);
		}
		ssa->vars[ssa_op->op2_def].definition = -1;
		ssa_op->op2_def = -1;
	}
	if (ssa_op->result_def >= 0) {
		if (ssa_op->result_use >= 0) {
			zend_ssa_rename_var_uses(ssa, ssa_op->result_def, ssa_op->result_use, 1);
		}
		ssa->vars[ssa_op->result_def].definition = -1;
		ssa_op->result_def = -1;
	}
}

#define NUM_PHI_SOURCES(phi) \
	((phi)->pi >= 0 ? 1 : (ssa->cfg.blocks[(phi)->block].predecessors_count))

/* FOREACH_USE and FOREACH_PHI_USE explicitly support "continue"
 * and changing the use chain of the current element */
#define FOREACH_USE(var, use) do { \
	int _var_num = (var) - ssa->vars, next; \
	for (use = (var)->use_chain; use >= 0; use = next) { \
		next = zend_ssa_next_use(ssa->ops, _var_num, use);
#define FOREACH_USE_END() \
	} \
} while (0)

#define FOREACH_PHI_USE(var, phi) do { \
	int _var_num = (var) - ssa->vars; \
	zend_ssa_phi *next_phi; \
	for (phi = (var)->phi_use_chain; phi; phi = next_phi) { \
		next_phi = zend_ssa_next_use_phi(ssa, _var_num, phi);
#define FOREACH_PHI_USE_END() \
	} \
} while (0)

#define FOREACH_PHI_SOURCE(phi, source) do { \
	zend_ssa_phi *_phi = (phi); \
	int _i, _end = NUM_PHI_SOURCES(phi); \
	for (_i = 0; _i < _end; _i++) { \
		ZEND_ASSERT(_phi->sources[_i] >= 0); \
		source = _phi->sources[_i];
#define FOREACH_PHI_SOURCE_END() \
	} \
} while (0)

#define FOREACH_PHI(phi) do { \
	int _i; \
	for (_i = 0; _i < ssa->cfg.blocks_count; _i++) { \
		phi = ssa->blocks[_i].phis; \
		for (; phi; phi = phi->next) {
#define FOREACH_PHI_END() \
		} \
	} \
} while (0)

#define FOREACH_BLOCK(block) do { \
	int _i; \
	for (_i = 0; _i < ssa->cfg.blocks_count; _i++) { \
		(block) = &ssa->cfg.blocks[_i]; \
		if (!((block)->flags & ZEND_BB_REACHABLE)) { \
			continue; \
		}
#define FOREACH_BLOCK_END() \
	} \
} while (0)

/* Does not support "break" */
#define FOREACH_INSTR_NUM(i) do { \
	zend_basic_block *_block; \
	FOREACH_BLOCK(_block) { \
		uint32_t _end = _block->start + _block->len; \
		for ((i) = _block->start; (i) < _end; (i)++) {
#define FOREACH_INSTR_NUM_END() \
		} \
	} FOREACH_BLOCK_END(); \
} while (0)

#endif /* ZEND_SSA_H */

/*
 * Local variables:
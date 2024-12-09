typedef struct _zend_ssa {
	zend_cfg               cfg;            /* control flow graph             */
	int                    rt_constants;   /* run-time or compile-time       */
	int                    vars_count;     /* number of SSA variables        */
	zend_ssa_block        *blocks;         /* array of SSA blocks            */
	zend_ssa_op           *ops;            /* array of SSA instructions      */
	zend_ssa_var          *vars;           /* use/def chain of SSA variables */
	int                    sccs;           /* number of SCCs                 */
	zend_ssa_var_info     *var_info;
} zend_ssa;

BEGIN_EXTERN_C()

int zend_build_ssa(zend_arena **arena, const zend_script *script, const zend_op_array *op_array, uint32_t build_flags, zend_ssa *ssa, uint32_t *func_flags);
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
	if (ssa_op->result_use == var && opline->opcode != ZEND_ADD_ARRAY_ELEMENT) {
		return 1;
	}
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
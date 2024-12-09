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

END_EXTERN_C()

static zend_always_inline int zend_ssa_next_use(const zend_ssa_op *ssa_op, int var, int use)
{
	ssa_op += use;
	if (ssa_op->result_use == var) {
		return ssa_op->res_use_chain;
	}
	return (ssa_op->op1_use == var) ? ssa_op->op1_use_chain : ssa_op->op2_use_chain;
}
	if (ssa_op->result_use == var && opline->opcode != ZEND_ADD_ARRAY_ELEMENT) {
		return 1;
	}
	return 0;
}

#endif /* ZEND_SSA_H */

/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * indent-tabs-mode: t
 * End:
 */
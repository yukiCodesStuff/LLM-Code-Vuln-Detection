int zend_ssa_compute_use_def_chains(zend_arena **arena, const zend_op_array *op_array, zend_ssa *ssa);
int zend_ssa_unlink_use_chain(zend_ssa *ssa, int op, int var);

END_EXTERN_C()

static zend_always_inline int zend_ssa_next_use(const zend_ssa_op *ssa_op, int var, int use)
{
	return 0;
}

#endif /* ZEND_SSA_H */

/*
 * Local variables:
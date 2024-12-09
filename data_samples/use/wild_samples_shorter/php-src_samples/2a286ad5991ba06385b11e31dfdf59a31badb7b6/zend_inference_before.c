				}
			} else {
				for (j = 0; j < ssa->cfg.blocks[p->block].predecessors_count; j++) {
					if (p->sources[j] >= 0 && ssa->vars[p->sources[j]].no_val) {
						ssa_vars[p->sources[j]].no_val = 0; /* used indirectly */
						zend_bitset_incl(worklist, p->sources[j]);
					}
				}
			}
		} else {
			for (i = 0; i < ssa->cfg.blocks[p->block].predecessors_count; i++) {
				if (p->sources[i] >= 0 && ssa->var_info[p->sources[i]].has_range) {
					/* union */
					tmp->underflow |= ssa->var_info[p->sources[i]].range.underflow;
					tmp->min = MIN(tmp->min, ssa->var_info[p->sources[i]].range.min);
					tmp->max = MAX(tmp->max, ssa->var_info[p->sources[i]].range.max);
				}
				UPDATE_SSA_TYPE(tmp, j);
				for (i = 0; i < blocks[p->block].predecessors_count; i++) {
					if (p->sources[i] >= 0) {
						zend_ssa_var_info *info = &ssa_var_info[p->sources[i]];
						if (info->type & MAY_BE_OBJECT) {
							if (first) {
								ce = info->ce;
								is_instanceof = info->is_instanceof;
								first = 0;
							} else {
								is_instanceof |= info->is_instanceof;
								ce = join_class_entries(ce, info->ce, &is_instanceof);
							}
						}
					}
				}
				UPDATE_SSA_OBJ_TYPE(ce, ce ? is_instanceof : 0, j);
	free_alloca(worklist, use_heap);
}

/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
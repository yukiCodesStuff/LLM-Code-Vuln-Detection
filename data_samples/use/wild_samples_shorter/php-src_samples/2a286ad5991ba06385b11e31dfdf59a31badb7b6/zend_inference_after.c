				}
			} else {
				for (j = 0; j < ssa->cfg.blocks[p->block].predecessors_count; j++) {
					ZEND_ASSERT(p->sources[j] >= 0);
					if (ssa->vars[p->sources[j]].no_val) {
						ssa_vars[p->sources[j]].no_val = 0; /* used indirectly */
						zend_bitset_incl(worklist, p->sources[j]);
					}
				}
			}
		} else {
			for (i = 0; i < ssa->cfg.blocks[p->block].predecessors_count; i++) {
				ZEND_ASSERT(p->sources[i] >= 0);
				if (ssa->var_info[p->sources[i]].has_range) {
					/* union */
					tmp->underflow |= ssa->var_info[p->sources[i]].range.underflow;
					tmp->min = MIN(tmp->min, ssa->var_info[p->sources[i]].range.min);
					tmp->max = MAX(tmp->max, ssa->var_info[p->sources[i]].range.max);
				}
				UPDATE_SSA_TYPE(tmp, j);
				for (i = 0; i < blocks[p->block].predecessors_count; i++) {
					zend_ssa_var_info *info;

					ZEND_ASSERT(p->sources[i] >= 0);
					info = &ssa_var_info[p->sources[i]];
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
				UPDATE_SSA_OBJ_TYPE(ce, ce ? is_instanceof : 0, j);
	free_alloca(worklist, use_heap);
}

int zend_may_throw(const zend_op *opline, zend_op_array *op_array, zend_ssa *ssa)
{
	uint32_t t1 = OP1_INFO();
	uint32_t t2 = OP2_INFO();

	if (opline->op1_type == IS_CV) {
		if (t1 & MAY_BE_UNDEF) {
			switch (opline->opcode) {
				case ZEND_UNSET_VAR:
				case ZEND_ISSET_ISEMPTY_VAR:
					if (opline->extended_value & ZEND_QUICK_SET) {
						break;
					}
					return 1;
				case ZEND_ISSET_ISEMPTY_DIM_OBJ:
				case ZEND_ISSET_ISEMPTY_PROP_OBJ:
				case ZEND_ASSIGN:
				case ZEND_ASSIGN_DIM:
				case ZEND_ASSIGN_REF:
				case ZEND_BIND_GLOBAL:
				case ZEND_FETCH_DIM_IS:
				case ZEND_FETCH_OBJ_IS:
				case ZEND_SEND_REF:
					break;
				default:
					/* undefined variable warning */
					return 1;
			}
		}
    } else if (opline->op1_type & (IS_TMP_VAR|IS_VAR)) {
		if (t1 & (MAY_BE_OBJECT|MAY_BE_RESOURCE|MAY_BE_ARRAY_OF_OBJECT|MAY_BE_ARRAY_OF_RESOURCE|MAY_BE_ARRAY_OF_ARRAY)) {
			switch (opline->opcode) {
				case ZEND_CASE:
				case ZEND_FE_FETCH_R:
				case ZEND_FE_FETCH_RW:
				case ZEND_FETCH_LIST:
				case ZEND_QM_ASSIGN:
				case ZEND_SEND_VAL:
				case ZEND_SEND_VAL_EX:
				case ZEND_SEND_VAR:
				case ZEND_SEND_VAR_EX:
				case ZEND_SEND_VAR_NO_REF:
				case ZEND_SEND_VAR_NO_REF_EX:
				case ZEND_SEND_REF:
				case ZEND_SEPARATE:
				case ZEND_END_SILENCE:
					break;
				default:
					/* destructor may be called */
					return 1;
			}
		}
    }

    if (opline->op2_type == IS_CV) {
		if (t2 & MAY_BE_UNDEF) {
			switch (opline->opcode) {
				case ZEND_ASSIGN_REF:
					break;
				default:
					/* undefined variable warning */
					return 1;
			}
		}
	} else if (opline->op2_type & (IS_TMP_VAR|IS_VAR)) {
		if (t2 & (MAY_BE_OBJECT|MAY_BE_RESOURCE|MAY_BE_ARRAY_OF_OBJECT|MAY_BE_ARRAY_OF_RESOURCE|MAY_BE_ARRAY_OF_ARRAY)) {
			switch (opline->opcode) {
				case ZEND_ASSIGN:
					break;
				default:
					/* destructor may be called */
					return 1;
			}
		}
    }

	switch (opline->opcode) {
		case ZEND_NOP:
		case ZEND_IS_IDENTICAL:
		case ZEND_IS_NOT_IDENTICAL:
		case ZEND_QM_ASSIGN:
		case ZEND_JMP:
		case ZEND_CHECK_VAR:
		case ZEND_MAKE_REF:
		case ZEND_SEND_VAR:
		case ZEND_BEGIN_SILENCE:
		case ZEND_END_SILENCE:
		case ZEND_SEND_VAL:
		case ZEND_SEND_REF:
		case ZEND_SEND_VAR_EX:
		case ZEND_FREE:
		case ZEND_SEPARATE:
		case ZEND_TYPE_CHECK:
		case ZEND_DEFINED:
		case ZEND_ISSET_ISEMPTY_THIS:
		case ZEND_COALESCE:
		case ZEND_SWITCH_LONG:
		case ZEND_SWITCH_STRING:
		case ZEND_ISSET_ISEMPTY_VAR:
			return 0;
		case ZEND_INIT_FCALL:
			/* can't throw, because call is resolved at compile time */
			return 0;
		case ZEND_BIND_GLOBAL:
			if ((opline+1)->opcode == ZEND_BIND_GLOBAL) {
				return zend_may_throw(opline + 1, op_array, ssa);
			}
			return 0;
		case ZEND_ADD:
			if ((t1 & MAY_BE_ANY) == MAY_BE_ARRAY
			 && (t2 & MAY_BE_ANY) == MAY_BE_ARRAY) {
				return 0;
			}
			return (t1 & (MAY_BE_STRING|MAY_BE_ARRAY|MAY_BE_OBJECT)) ||
				(t2 & (MAY_BE_STRING|MAY_BE_ARRAY|MAY_BE_OBJECT));
		case ZEND_DIV:
		case ZEND_MOD:
			if (!OP2_HAS_RANGE() ||
				(OP2_MIN_RANGE() <= 0 && OP2_MAX_RANGE() >= 0)) {
				/* Division by zero */
				return 1;
			}
			/* break missing intentionally */
		case ZEND_SUB:
		case ZEND_MUL:
		case ZEND_POW:
			return (t1 & (MAY_BE_STRING|MAY_BE_ARRAY|MAY_BE_OBJECT)) ||
				(t2 & (MAY_BE_STRING|MAY_BE_ARRAY|MAY_BE_OBJECT));
		case ZEND_SL:
		case ZEND_SR:
			return (t1 & (MAY_BE_STRING|MAY_BE_ARRAY|MAY_BE_OBJECT)) ||
				(t2 & (MAY_BE_STRING|MAY_BE_ARRAY|MAY_BE_OBJECT)) ||
				!OP2_HAS_RANGE() ||
				OP2_MIN_RANGE() < 0;
		case ZEND_CONCAT:
		case ZEND_FAST_CONCAT:
			return (t1 & (MAY_BE_ARRAY|MAY_BE_OBJECT)) ||
				(t2 & (MAY_BE_ARRAY|MAY_BE_OBJECT));
		case ZEND_BW_OR:
		case ZEND_BW_AND:
		case ZEND_BW_XOR:
			if ((t1 & MAY_BE_ANY) == MAY_BE_STRING
			 && (t2 & MAY_BE_ANY) == MAY_BE_STRING) {
				return 0;
			}
			return (t1 & (MAY_BE_STRING|MAY_BE_ARRAY|MAY_BE_OBJECT)) ||
				(t2 & (MAY_BE_STRING|MAY_BE_ARRAY|MAY_BE_OBJECT));
		case ZEND_BW_NOT:
			return (t1 & (MAY_BE_NULL|MAY_BE_FALSE|MAY_BE_TRUE|MAY_BE_ARRAY|MAY_BE_OBJECT|MAY_BE_RESOURCE));
		case ZEND_BOOL_NOT:
		case ZEND_PRE_INC:
		case ZEND_POST_INC:
		case ZEND_PRE_DEC:
		case ZEND_POST_DEC:
		case ZEND_JMPZ:
		case ZEND_JMPNZ:
		case ZEND_JMPZNZ:
		case ZEND_JMPZ_EX:
		case ZEND_JMPNZ_EX:
		case ZEND_BOOL:
		case ZEND_JMP_SET:
			return (t1 & MAY_BE_OBJECT);
		case ZEND_BOOL_XOR:
			return (t1 & MAY_BE_OBJECT) || (t2 & MAY_BE_OBJECT);
		case ZEND_IS_EQUAL:
		case ZEND_IS_NOT_EQUAL:
		case ZEND_IS_SMALLER:
		case ZEND_IS_SMALLER_OR_EQUAL:
		case ZEND_CASE:
		case ZEND_SPACESHIP:
			if ((t1 & MAY_BE_ANY) == MAY_BE_NULL
			 || (t2 & MAY_BE_ANY) == MAY_BE_NULL) {
				return 0;
			}
			return (t1 & (MAY_BE_OBJECT|MAY_BE_ARRAY_OF_ARRAY|MAY_BE_ARRAY_OF_OBJECT)) || (t2 & (MAY_BE_OBJECT|MAY_BE_ARRAY_OF_ARRAY|MAY_BE_ARRAY_OF_OBJECT));
		case ZEND_ASSIGN_ADD:
			if (opline->extended_value != 0) {
				return 1;
			}
			if ((t1 & MAY_BE_ANY) == MAY_BE_ARRAY
			 && (t2 & MAY_BE_ANY) == MAY_BE_ARRAY) {
				return 0;
			}
			return (t1 & (MAY_BE_STRING|MAY_BE_ARRAY|MAY_BE_OBJECT)) ||
				(t2 & (MAY_BE_STRING|MAY_BE_ARRAY|MAY_BE_OBJECT));
		case ZEND_ASSIGN_DIV:
		case ZEND_ASSIGN_MOD:
			if (opline->extended_value != 0) {
				return 1;
			}
			if (!OP2_HAS_RANGE() ||
				(OP2_MIN_RANGE() <= 0 && OP2_MAX_RANGE() >= 0)) {
				/* Division by zero */
				return 1;
			}
			/* break missing intentionally */
		case ZEND_ASSIGN_SUB:
		case ZEND_ASSIGN_MUL:
		case ZEND_ASSIGN_POW:
			if (opline->extended_value != 0) {
				return 1;
			}
			return (t1 & (MAY_BE_STRING|MAY_BE_ARRAY|MAY_BE_OBJECT)) ||
				(t2 & (MAY_BE_STRING|MAY_BE_ARRAY|MAY_BE_OBJECT));
		case ZEND_ASSIGN_SL:
		case ZEND_ASSIGN_SR:
			if (opline->extended_value != 0) {
				return 1;
			}
			return (t1 & (MAY_BE_STRING|MAY_BE_ARRAY|MAY_BE_OBJECT)) ||
				(t2 & (MAY_BE_STRING|MAY_BE_ARRAY|MAY_BE_OBJECT)) ||
				!OP2_HAS_RANGE() ||
				OP2_MIN_RANGE() < 0;
		case ZEND_ASSIGN_CONCAT:
			if (opline->extended_value != 0) {
				return 1;
			}
			return (t1 & (MAY_BE_ARRAY|MAY_BE_OBJECT)) ||
				(t2 & (MAY_BE_ARRAY|MAY_BE_OBJECT));
		case ZEND_ASSIGN_BW_OR:
		case ZEND_ASSIGN_BW_AND:
		case ZEND_ASSIGN_BW_XOR:
			if (opline->extended_value != 0) {
				return 1;
			}
			if ((t1 & MAY_BE_ANY) == MAY_BE_STRING
			 && (t2 & MAY_BE_ANY) == MAY_BE_STRING) {
				return 0;
			}
			return (t1 & (MAY_BE_STRING|MAY_BE_ARRAY|MAY_BE_OBJECT)) ||
				(t2 & (MAY_BE_STRING|MAY_BE_ARRAY|MAY_BE_OBJECT));
		case ZEND_ASSIGN:
		case ZEND_UNSET_VAR:
			return (t1 & (MAY_BE_OBJECT|MAY_BE_RESOURCE|MAY_BE_ARRAY_OF_OBJECT|MAY_BE_ARRAY_OF_RESOURCE|MAY_BE_ARRAY_OF_ARRAY));
		case ZEND_ASSIGN_DIM:
			return (t1 & (MAY_BE_OBJECT|MAY_BE_RESOURCE|MAY_BE_TRUE|MAY_BE_STRING|MAY_BE_LONG|MAY_BE_DOUBLE)) || opline->op2_type == IS_UNUSED ||
				(t2 & (MAY_BE_UNDEF|MAY_BE_ARRAY|MAY_BE_OBJECT|MAY_BE_RESOURCE));
		case ZEND_ROPE_INIT:
		case ZEND_ROPE_ADD:
		case ZEND_ROPE_END:
			return t2 & (MAY_BE_ARRAY|MAY_BE_OBJECT);
		case ZEND_INIT_ARRAY:
		case ZEND_ADD_ARRAY_ELEMENT:
			return (opline->op2_type != IS_UNUSED) && (t2 & (MAY_BE_ARRAY|MAY_BE_OBJECT|MAY_BE_RESOURCE));
		case ZEND_STRLEN:
			return (t1 & MAY_BE_ANY) != MAY_BE_STRING;
		case ZEND_COUNT:
			return (t1 & MAY_BE_ANY) != MAY_BE_ARRAY;
		case ZEND_RECV_INIT:
			if (Z_CONSTANT_P(CRT_CONSTANT_EX(op_array, opline->op2, ssa->rt_constants))) {
				return 1;
			}
			if (op_array->fn_flags & ZEND_ACC_HAS_TYPE_HINTS) {
				uint32_t arg_num = opline->op1.num;
				zend_arg_info *cur_arg_info;

				if (EXPECTED(arg_num <= op_array->num_args)) {
					cur_arg_info = &op_array->arg_info[arg_num-1];
				} else if (UNEXPECTED(op_array->fn_flags & ZEND_ACC_VARIADIC)) {
					cur_arg_info = &op_array->arg_info[op_array->num_args];
				} else {
					return 0;
				}
				return ZEND_TYPE_IS_SET(cur_arg_info->type);
			} else {
				return 0;
			}
		case ZEND_FETCH_IS:
			return (t2 & (MAY_BE_ARRAY|MAY_BE_OBJECT));
		case ZEND_ISSET_ISEMPTY_DIM_OBJ:
			return (t1 & MAY_BE_OBJECT) || (t2 & (MAY_BE_ARRAY|MAY_BE_OBJECT));
		case ZEND_FETCH_DIM_IS:
			return (t1 & MAY_BE_OBJECT) || (t2 & (MAY_BE_ARRAY|MAY_BE_OBJECT|MAY_BE_RESOURCE));
		case ZEND_CAST:
			switch (opline->extended_value) {
				case IS_NULL:
					return 0;
				case _IS_BOOL:
					return (t1 & MAY_BE_OBJECT);
				case IS_LONG:
				case IS_DOUBLE:
					return (t1 & MAY_BE_OBJECT);
				case IS_STRING:
					return (t1 & (MAY_BE_ARRAY|MAY_BE_OBJECT));
				case IS_ARRAY:
					return (t1 & MAY_BE_OBJECT);
				case IS_OBJECT:
					return (t1 & MAY_BE_ARRAY);
				default:
					return 1;
			}
		default:
			return 1;
	}
}

/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
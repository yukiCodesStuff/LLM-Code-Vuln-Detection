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
					tmp->overflow |= ssa->var_info[p->sources[i]].range.overflow;
				} else if (narrowing) {
					tmp->underflow = 1;
					tmp->min = ZEND_LONG_MIN;
					tmp->max = ZEND_LONG_MAX;
					tmp->overflow = 1;
				}
			}
		}
		return (tmp->min <= tmp->max);
	} else if (ssa->vars[var].definition < 0) {
		if (var < op_array->last_var &&
		    op_array->function_name) {

			tmp->min = 0;
			tmp->max = 0;
			tmp->underflow = 0;
			tmp->overflow = 0;
			return 1;
		}
		return 0;
	}
	line = ssa->vars[var].definition;
	opline = op_array->opcodes + line;

	tmp->underflow = 0;
	tmp->overflow = 0;
	switch (opline->opcode) {
		case ZEND_ADD:
		case ZEND_SUB:
		case ZEND_MUL:
		case ZEND_DIV:
		case ZEND_MOD:
		case ZEND_SL:
		case ZEND_SR:
		case ZEND_BW_OR:
		case ZEND_BW_AND:
		case ZEND_BW_XOR:
			if (ssa->ops[line].result_def == var) {
				return zend_inference_calc_binary_op_range(
					op_array, ssa, opline, &ssa->ops[line], opline->opcode, tmp);
			}
			break;

		case ZEND_BW_NOT:
			if (ssa->ops[line].result_def == var) {
				if (OP1_HAS_RANGE()) {
					if (OP1_RANGE_UNDERFLOW() ||
					    OP1_RANGE_OVERFLOW()) {
						tmp->min = ZEND_LONG_MIN;
						tmp->max = ZEND_LONG_MAX;
					} else {
						op1_min = OP1_MIN_RANGE();
						op1_max = OP1_MAX_RANGE();
						tmp->min = ~op1_max;
						tmp->max = ~op1_min;
					}
					return 1;
				}
			}
			break;
		case ZEND_CAST:
			if (ssa->ops[line].op1_def == var) {
				if (ssa->ops[line].op1_def >= 0) {
					if (OP1_HAS_RANGE()) {
						tmp->underflow = OP1_RANGE_UNDERFLOW();
						tmp->min = OP1_MIN_RANGE();
						tmp->max = OP1_MAX_RANGE();
						tmp->overflow  = OP1_RANGE_OVERFLOW();
						return 1;
					}
				}
			} else if (ssa->ops[line].result_def == var) {
				if (opline->extended_value == IS_NULL) {
					tmp->min = 0;
					tmp->max = 0;
					return 1;
				} else if (opline->extended_value == _IS_BOOL) {
					if (OP1_HAS_RANGE()) {
						op1_min = OP1_MIN_RANGE();
						op1_max = OP1_MAX_RANGE();
						tmp->min = (op1_min > 0 || op1_max < 0);
						tmp->max = (op1_min != 0 || op1_max != 0);
						return 1;
					} else {
						tmp->min = 0;
						tmp->max = 1;
						return 1;
					}
				} else if (opline->extended_value == IS_LONG) {
					if (OP1_HAS_RANGE()) {
						tmp->min = OP1_MIN_RANGE();
						tmp->max = OP1_MAX_RANGE();
						return 1;
					} else {
						tmp->min = ZEND_LONG_MIN;
						tmp->max = ZEND_LONG_MAX;
						return 1;
					}
				}
			}
			break;
		case ZEND_BOOL:
		case ZEND_JMPZ_EX:
		case ZEND_JMPNZ_EX:
			if (ssa->ops[line].result_def == var) {
				if (OP1_HAS_RANGE()) {
					op1_min = OP1_MIN_RANGE();
					op1_max = OP1_MAX_RANGE();
					tmp->min = (op1_min > 0 || op1_max < 0);
					tmp->max = (op1_min != 0 || op1_max != 0);
					return 1;
				} else {
					tmp->min = 0;
					tmp->max = 1;
					return 1;
				}
			}
			break;
		case ZEND_BOOL_NOT:
			if (ssa->ops[line].result_def == var) {
				if (OP1_HAS_RANGE()) {
					op1_min = OP1_MIN_RANGE();
					op1_max = OP1_MAX_RANGE();
					tmp->min = (op1_min == 0 && op1_max == 0);
					tmp->max = (op1_min <= 0 && op1_max >= 0);
					return 1;
				} else {
					tmp->min = 0;
					tmp->max = 1;
					return 1;
				}
			}
			break;
		case ZEND_BOOL_XOR:
			if (ssa->ops[line].result_def == var) {
				if (OP1_HAS_RANGE() && OP2_HAS_RANGE()) {
					op1_min = OP1_MIN_RANGE();
					op2_min = OP2_MIN_RANGE();
					op1_max = OP1_MAX_RANGE();
					op2_max = OP2_MAX_RANGE();
					op1_min = (op1_min > 0 || op1_max < 0);
					op1_max = (op1_min != 0 || op1_max != 0);
					op2_min = (op2_min > 0 || op2_max < 0);
					op2_max = (op2_min != 0 || op2_max != 0);
					tmp->min = 0;
					tmp->max = 1;
					if (op1_min == op1_max && op2_min == op2_max) {
						if (op1_min == op2_min) {
							tmp->max = 0;
						} else {
							tmp->min = 1;
						}
					}
					return 1;
				} else {
					tmp->min = 0;
					tmp->max = 1;
					return 1;
				}
			}
			break;
		case ZEND_IS_IDENTICAL:
		case ZEND_IS_EQUAL:
			if (ssa->ops[line].result_def == var) {
				if (OP1_HAS_RANGE() && OP2_HAS_RANGE()) {
					op1_min = OP1_MIN_RANGE();
					op2_min = OP2_MIN_RANGE();
					op1_max = OP1_MAX_RANGE();
					op2_max = OP2_MAX_RANGE();

					tmp->min = (op1_min == op1_max &&
					           op2_min == op2_max &&
					           op1_min == op2_max);
					tmp->max = (op1_min <= op2_max && op1_max >= op2_min);
					return 1;
				} else {
					tmp->min = 0;
					tmp->max = 1;
					return 1;
				}
			}
			break;
		case ZEND_IS_NOT_IDENTICAL:
		case ZEND_IS_NOT_EQUAL:
			if (ssa->ops[line].result_def == var) {
				if (OP1_HAS_RANGE() && OP2_HAS_RANGE()) {
					op1_min = OP1_MIN_RANGE();
					op2_min = OP2_MIN_RANGE();
					op1_max = OP1_MAX_RANGE();
					op2_max = OP2_MAX_RANGE();

					tmp->min = (op1_min > op2_max || op1_max < op2_min);
					tmp->max = (op1_min != op1_max ||
					           op2_min != op2_max ||
					           op1_min != op2_max);
					return 1;
				} else {
					tmp->min = 0;
					tmp->max = 1;
					return 1;
				}
			}
			break;
		case ZEND_IS_SMALLER:
			if (ssa->ops[line].result_def == var) {
				if (OP1_HAS_RANGE() && OP2_HAS_RANGE()) {
					op1_min = OP1_MIN_RANGE();
					op2_min = OP2_MIN_RANGE();
					op1_max = OP1_MAX_RANGE();
					op2_max = OP2_MAX_RANGE();

					tmp->min = op1_max < op2_min;
					tmp->max = op1_min < op2_max;
					return 1;
				} else {
					tmp->min = 0;
					tmp->max = 1;
					return 1;
				}
			}
			break;
		case ZEND_IS_SMALLER_OR_EQUAL:
			if (ssa->ops[line].result_def == var) {
				if (OP1_HAS_RANGE() && OP2_HAS_RANGE()) {
					op1_min = OP1_MIN_RANGE();
					op2_min = OP2_MIN_RANGE();
					op1_max = OP1_MAX_RANGE();
					op2_max = OP2_MAX_RANGE();

					tmp->min = op1_max <= op2_min;
					tmp->max = op1_min <= op2_max;
					return 1;
				} else {
					tmp->min = 0;
					tmp->max = 1;
					return 1;
				}
			}
			break;
		case ZEND_QM_ASSIGN:
		case ZEND_JMP_SET:
		case ZEND_COALESCE:
			if (ssa->ops[line].op1_def == var) {
				if (ssa->ops[line].op1_def >= 0) {
					if (OP1_HAS_RANGE()) {
						tmp->underflow = OP1_RANGE_UNDERFLOW();
						tmp->min = OP1_MIN_RANGE();
						tmp->max = OP1_MAX_RANGE();
						tmp->overflow  = OP1_RANGE_OVERFLOW();
						return 1;
					}
				}
			}
			if (ssa->ops[line].result_def == var) {
				if (OP1_HAS_RANGE()) {
					tmp->min = OP1_MIN_RANGE();
					tmp->max = OP1_MAX_RANGE();
					tmp->underflow = OP1_RANGE_UNDERFLOW();
					tmp->overflow  = OP1_RANGE_OVERFLOW();
					return 1;
				}
			}
			break;
		case ZEND_ASSERT_CHECK:
			if (ssa->ops[line].result_def == var) {
				tmp->min = 0;
				tmp->max = 1;
				return 1;
			}
			break;
		case ZEND_SEND_VAR:
			if (ssa->ops[line].op1_def == var) {
				if (ssa->ops[line].op1_def >= 0) {
					if (OP1_HAS_RANGE()) {
						tmp->underflow = OP1_RANGE_UNDERFLOW();
						tmp->min = OP1_MIN_RANGE();
						tmp->max = OP1_MAX_RANGE();
						tmp->overflow  = OP1_RANGE_OVERFLOW();
						return 1;
					}
				}
			}
			break;
		case ZEND_PRE_INC:
			if (ssa->ops[line].op1_def == var || ssa->ops[line].result_def == var) {
				if (OP1_HAS_RANGE()) {
					tmp->min = OP1_MIN_RANGE();
					tmp->max = OP1_MAX_RANGE();
					tmp->underflow = OP1_RANGE_UNDERFLOW();
					tmp->overflow = OP1_RANGE_OVERFLOW();
					if (tmp->max < ZEND_LONG_MAX) {
						tmp->max++;
					} else {
						tmp->overflow = 1;
					}
					if (tmp->min < ZEND_LONG_MAX && !tmp->underflow) {
						tmp->min++;
					}
					return 1;
				}
			}
			break;
		case ZEND_PRE_DEC:
			if (ssa->ops[line].op1_def == var || ssa->ops[line].result_def == var) {
				if (OP1_HAS_RANGE()) {
					tmp->min = OP1_MIN_RANGE();
					tmp->max = OP1_MAX_RANGE();
					tmp->underflow = OP1_RANGE_UNDERFLOW();
					tmp->overflow = OP1_RANGE_OVERFLOW();
					if (tmp->min > ZEND_LONG_MIN) {
						tmp->min--;
					} else {
						tmp->underflow = 1;
					}
					if (tmp->max > ZEND_LONG_MIN && !tmp->overflow) {
						tmp->max--;
					}
					return 1;
				}
			}
			break;
		case ZEND_POST_INC:
			if (ssa->ops[line].op1_def == var || ssa->ops[line].result_def == var) {
				if (OP1_HAS_RANGE()) {
					tmp->min = OP1_MIN_RANGE();
					tmp->max = OP1_MAX_RANGE();
					tmp->underflow = OP1_RANGE_UNDERFLOW();
					tmp->overflow = OP1_RANGE_OVERFLOW();
					if (ssa->ops[line].result_def == var) {
						return 1;
					}
					if (tmp->max < ZEND_LONG_MAX) {
						tmp->max++;
					} else {
						tmp->overflow = 1;
					}
					if (tmp->min < ZEND_LONG_MAX && !tmp->underflow) {
						tmp->min++;
					}
					return 1;
				}
			}
			break;
		case ZEND_POST_DEC:
			if (ssa->ops[line].op1_def == var || ssa->ops[line].result_def == var) {
				if (OP1_HAS_RANGE()) {
					tmp->min = OP1_MIN_RANGE();
					tmp->max = OP1_MAX_RANGE();
					tmp->underflow = OP1_RANGE_UNDERFLOW();
					tmp->overflow = OP1_RANGE_OVERFLOW();
					if (ssa->ops[line].result_def == var) {
						return 1;
					}
					if (tmp->min > ZEND_LONG_MIN) {
						tmp->min--;
					} else {
						tmp->underflow = 1;
					}
					if (tmp->max > ZEND_LONG_MIN && !tmp->overflow) {
						tmp->max--;
					}
					return 1;
				}
			}
			break;
		case ZEND_UNSET_DIM:
		case ZEND_UNSET_OBJ:
			if (ssa->ops[line].op1_def == var) {
				/* If op1 is scalar, UNSET_DIM and UNSET_OBJ have no effect, so we can keep
				 * the previous ranges. */
				if (OP1_HAS_RANGE()) {
					tmp->min = OP1_MIN_RANGE();
					tmp->max = OP1_MAX_RANGE();
					tmp->underflow = OP1_RANGE_UNDERFLOW();
					tmp->overflow  = OP1_RANGE_OVERFLOW();
					return 1;
				}
			}
			break;
		case ZEND_ASSIGN:
			if (ssa->ops[line].op1_def == var || ssa->ops[line].op2_def == var || ssa->ops[line].result_def == var) {
				if (OP2_HAS_RANGE()) {
					tmp->min = OP2_MIN_RANGE();
					tmp->max = OP2_MAX_RANGE();
					tmp->underflow = OP2_RANGE_UNDERFLOW();
					tmp->overflow  = OP2_RANGE_OVERFLOW();
					return 1;
				}
			}
			break;
		case ZEND_ASSIGN_DIM:
		case ZEND_ASSIGN_OBJ:
			if (ssa->ops[line+1].op1_def == var) {
				if ((opline+1)->opcode == ZEND_OP_DATA) {
					opline++;
					tmp->min = OP1_MIN_RANGE();
					tmp->max = OP1_MAX_RANGE();
					tmp->underflow = OP1_RANGE_UNDERFLOW();
					tmp->overflow  = OP1_RANGE_OVERFLOW();
					return 1;
				}
			}
			break;
		case ZEND_ASSIGN_ADD:
		case ZEND_ASSIGN_SUB:
		case ZEND_ASSIGN_MUL:
		case ZEND_ASSIGN_DIV:
		case ZEND_ASSIGN_MOD:
		case ZEND_ASSIGN_SL:
		case ZEND_ASSIGN_SR:
		case ZEND_ASSIGN_BW_OR:
		case ZEND_ASSIGN_BW_AND:
		case ZEND_ASSIGN_BW_XOR:
			if (opline->extended_value == 0) {
				if (ssa->ops[line].op1_def == var || ssa->ops[line].result_def == var) {
					return zend_inference_calc_binary_op_range(
						op_array, ssa, opline, &ssa->ops[line],
						get_compound_assign_op(opline->opcode), tmp);
				}
			} else if ((opline+1)->opcode == ZEND_OP_DATA) {
				if (ssa->ops[line+1].op1_def == var) {
					opline++;
					if (OP1_HAS_RANGE()) {
						tmp->min = OP1_MIN_RANGE();
						tmp->max = OP1_MAX_RANGE();
						tmp->underflow = OP1_RANGE_UNDERFLOW();
						tmp->overflow  = OP1_RANGE_OVERFLOW();
						return 1;
					}
				}
			}
			break;
//		case ZEND_ASSIGN_CONCAT:
		case ZEND_OP_DATA:
			if ((opline-1)->opcode == ZEND_ASSIGN_DIM ||
			    (opline-1)->opcode == ZEND_ASSIGN_OBJ ||
			    (opline-1)->opcode == ZEND_ASSIGN_ADD ||
			    (opline-1)->opcode == ZEND_ASSIGN_SUB ||
			    (opline-1)->opcode == ZEND_ASSIGN_MUL) {
				if (ssa->ops[line].op1_def == var) {
					if (OP1_HAS_RANGE()) {
						tmp->min = OP1_MIN_RANGE();
						tmp->max = OP1_MAX_RANGE();
						tmp->underflow = OP1_RANGE_UNDERFLOW();
						tmp->overflow  = OP1_RANGE_OVERFLOW();
						return 1;
					}
				}
				break;
			}
			break;
		case ZEND_RECV:
		case ZEND_RECV_INIT:
			if (ssa->ops[line].result_def == var) {
				zend_func_info *func_info = ZEND_FUNC_INFO(op_array);

				if (func_info &&
				    (int)opline->op1.num-1 < func_info->num_args &&
				    func_info->arg_info[opline->op1.num-1].info.has_range) {
					*tmp = func_info->arg_info[opline->op1.num-1].info.range;
					return 1;
				} else if (op_array->arg_info &&
				    opline->op1.num <= op_array->num_args) {
					if (ZEND_TYPE_CODE(op_array->arg_info[opline->op1.num-1].type) == IS_LONG) {
						tmp->underflow = 0;
						tmp->min = ZEND_LONG_MIN;
						tmp->max = ZEND_LONG_MAX;
						tmp->overflow = 0;
						return 1;
					} else if (ZEND_TYPE_CODE(op_array->arg_info[opline->op1.num-1].type) == _IS_BOOL) {
						tmp->underflow = 0;
						tmp->min = 0;
						tmp->max = 1;
						tmp->overflow = 0;
						return 1;
					}
				}
			}
			break;
		case ZEND_STRLEN:
			if (ssa->ops[line].result_def == var) {
#if SIZEOF_ZEND_LONG == 4
				/* The length of a string is a non-negative integer. However, on 32-bit
				 * platforms overflows into negative lengths may occur, so it's better
				 * to not assume any particular range. */
				tmp->min = ZEND_LONG_MIN;
#else
				tmp->min = 0;
#endif
				tmp->max = ZEND_LONG_MAX;
				return 1;
			}
			break;
		case ZEND_COUNT:
		case ZEND_FUNC_NUM_ARGS:
			tmp->min = 0;
			tmp->max = ZEND_LONG_MAX;
			return 1;
		case ZEND_DO_FCALL:
		case ZEND_DO_ICALL:
		case ZEND_DO_UCALL:
		case ZEND_DO_FCALL_BY_NAME:
			if (ssa->ops[line].result_def == var) {
				zend_func_info *func_info = ZEND_FUNC_INFO(op_array);
				zend_call_info *call_info;
				if (!func_info || !func_info->call_map) {
					break;
				}

				call_info = func_info->call_map[opline - op_array->opcodes];
				if (!call_info) {
					break;
				}
				if (call_info->callee_func->type == ZEND_USER_FUNCTION) {
					func_info = ZEND_FUNC_INFO(&call_info->callee_func->op_array);
					if (func_info && func_info->return_info.has_range) {
						*tmp = func_info->return_info.range;
						return 1;
					}
				}
//TODO: we can't use type inference for internal functions at this point ???
#if 0
					uint32_t type;

					type = zend_get_func_info(call_info, ssa);
					if (!(type & (MAY_BE_ANY - (MAY_BE_NULL|MAY_BE_FALSE|MAY_BE_TRUE|MAY_BE_LONG)))) {
						tmp->underflow = 0;
						tmp->min = 0;
						tmp->max = 0;
						tmp->overflow = 0;
						if (type & MAY_BE_LONG) {
							tmp->min = ZEND_LONG_MIN;
							tmp->max = ZEND_LONG_MAX;
						} else if (type & MAY_BE_TRUE) {
							if (!(type & (MAY_BE_NULL|MAY_BE_FALSE))) {
								tmp->min = 1;
							}
							tmp->max = 1;
						}
						return 1;
					}
#endif
			}
			break;
		// FIXME: support for more opcodes
		default:
			break;
	}
	return 0;
}

void zend_inference_init_range(const zend_op_array *op_array, zend_ssa *ssa, int var, zend_bool underflow, zend_long min, zend_long max, zend_bool overflow)
{
	if (underflow) {
		min = ZEND_LONG_MIN;
	}
	if (overflow) {
		max = ZEND_LONG_MAX;
	}
	ssa->var_info[var].has_range = 1;
	ssa->var_info[var].range.underflow = underflow;
	ssa->var_info[var].range.min = min;
	ssa->var_info[var].range.max = max;
	ssa->var_info[var].range.overflow = overflow;
	LOG_SSA_RANGE("  change range (init      SCC %2d) %2d [%s%ld..%ld%s]\n", ssa->vars[var].scc, var, (underflow?"-- ":""), min, max, (overflow?" ++":""));
}

int zend_inference_widening_meet(zend_ssa_var_info *var_info, zend_ssa_range *r)
{
	if (!var_info->has_range) {
		var_info->has_range = 1;
	} else {
		if (r->underflow ||
		    var_info->range.underflow ||
		    r->min < var_info->range.min) {
			r->underflow = 1;
			r->min = ZEND_LONG_MIN;
		}
		if (r->overflow ||
		    var_info->range.overflow ||
		    r->max > var_info->range.max) {
			r->overflow = 1;
			r->max = ZEND_LONG_MAX;
		}
		if (var_info->range.min == r->min &&
		    var_info->range.max == r->max &&
		    var_info->range.underflow == r->underflow &&
		    var_info->range.overflow == r->overflow) {
			return 0;
		}
	}
	var_info->range = *r;
	return 1;
}

static int zend_ssa_range_widening(const zend_op_array *op_array, zend_ssa *ssa, int var, int scc)
{
	zend_ssa_range tmp;

	if (zend_inference_calc_range(op_array, ssa, var, 1, 0, &tmp)) {
		if (zend_inference_widening_meet(&ssa->var_info[var], &tmp)) {
			LOG_SSA_RANGE("  change range (widening  SCC %2d) %2d [%s%ld..%ld%s]\n", scc, var, (tmp.underflow?"-- ":""), tmp.min, tmp.max, (tmp.overflow?" ++":""));
			return 1;
		}
	}
	return 0;
}

int zend_inference_narrowing_meet(zend_ssa_var_info *var_info, zend_ssa_range *r)
{
	if (!var_info->has_range) {
		var_info->has_range = 1;
	} else {
		if (!r->underflow &&
		    !var_info->range.underflow &&
		    var_info->range.min < r->min) {
			r->min = var_info->range.min;
		}
		if (!r->overflow &&
		    !var_info->range.overflow &&
		    var_info->range.max > r->max) {
			r->max = var_info->range.max;
		}
		if (r->underflow) {
			r->min = ZEND_LONG_MIN;
		}
		if (r->overflow) {
			r->max = ZEND_LONG_MAX;
		}
		if (var_info->range.min == r->min &&
		    var_info->range.max == r->max &&
		    var_info->range.underflow == r->underflow &&
		    var_info->range.overflow == r->overflow) {
			return 0;
		}
	}
	var_info->range = *r;
	return 1;
}

static int zend_ssa_range_narrowing(const zend_op_array *op_array, zend_ssa *ssa, int var, int scc)
{
	zend_ssa_range tmp;

	if (zend_inference_calc_range(op_array, ssa, var, 0, 1, &tmp)) {
		if (zend_inference_narrowing_meet(&ssa->var_info[var], &tmp)) {
			LOG_SSA_RANGE("  change range (narrowing SCC %2d) %2d [%s%ld..%ld%s]\n", scc, var, (tmp.underflow?"-- ":""), tmp.min, tmp.max, (tmp.overflow?" ++":""));
			return 1;
		}
	}
	return 0;
}

#ifdef NEG_RANGE
# define CHECK_INNER_CYCLE(var2) \
	do { \
		if (ssa->vars[var2].scc == ssa->vars[var].scc && \
		    !ssa->vars[var2].scc_entry && \
		    !zend_bitset_in(visited, var2) && \
			zend_check_inner_cycles(op_array, ssa, worklist, visited, var2)) { \
			return 1; \
		} \
	} while (0)

static int zend_check_inner_cycles(const zend_op_array *op_array, zend_ssa *ssa, zend_bitset worklist, zend_bitset visited, int var)
{
	if (zend_bitset_in(worklist, var)) {
		return 1;
	}
	zend_bitset_incl(worklist, var);
	FOR_EACH_VAR_USAGE(var, CHECK_INNER_CYCLE);
	zend_bitset_incl(visited, var);
	return 0;
}
#endif

static void zend_infer_ranges_warmup(const zend_op_array *op_array, zend_ssa *ssa, int *scc_var, int *next_scc_var, int scc)
{
	int worklist_len = zend_bitset_len(ssa->vars_count);
	int j, n;
	zend_ssa_range tmp;
	ALLOCA_FLAG(use_heap)
	zend_bitset worklist = do_alloca(sizeof(zend_ulong) * worklist_len * 2, use_heap);
	zend_bitset visited = worklist + worklist_len;
#ifdef NEG_RANGE
	int has_inner_cycles = 0;

	memset(worklist, 0, sizeof(zend_ulong) * worklist_len);
	memset(visited, 0, sizeof(zend_ulong) * worklist_len);
	j = scc_var[scc];
	while (j >= 0) {
		if (!zend_bitset_in(visited, j) &&
		    zend_check_inner_cycles(op_array, ssa, worklist, visited, j)) {
			has_inner_cycles = 1;
			break;
		}
		j = next_scc_var[j];
	}
#endif

	memset(worklist, 0, sizeof(zend_ulong) * worklist_len);

	for (n = 0; n < RANGE_WARMUP_PASSES; n++) {
		j= scc_var[scc];
		while (j >= 0) {
			if (ssa->vars[j].scc_entry) {
				zend_bitset_incl(worklist, j);
			}
			j = next_scc_var[j];
		}

		memset(visited, 0, sizeof(zend_ulong) * worklist_len);

		WHILE_WORKLIST(worklist, worklist_len, j) {
			if (zend_inference_calc_range(op_array, ssa, j, 0, 0, &tmp)) {
#ifdef NEG_RANGE
				if (!has_inner_cycles &&
				    ssa->var_info[j].has_range &&
				    ssa->vars[j].definition_phi &&
				    ssa->vars[j].definition_phi->pi >= 0 &&
					ssa->vars[j].definition_phi->has_range_constraint &&
				    ssa->vars[j].definition_phi->constraint.range.negative &&
				    ssa->vars[j].definition_phi->constraint.range.min_ssa_var < 0 &&
				    ssa->vars[j].definition_phi->constraint.range.min_ssa_var < 0) {
					zend_ssa_range_constraint *constraint =
						&ssa->vars[j].definition_phi->constraint.range;
					if (tmp.min == ssa->var_info[j].range.min &&
					    tmp.max == ssa->var_info[j].range.max) {
						if (constraint->negative == NEG_INIT) {
							LOG_NEG_RANGE("#%d INVARIANT\n", j);
							constraint->negative = NEG_INVARIANT;
						}
					} else if (tmp.min == ssa->var_info[j].range.min &&
					           tmp.max == ssa->var_info[j].range.max + 1 &&
					           tmp.max < constraint->range.min) {
						if (constraint->negative == NEG_INIT ||
						    constraint->negative == NEG_INVARIANT) {
							LOG_NEG_RANGE("#%d LT\n", j);
							constraint->negative = NEG_USE_LT;
//???NEG
						} else if (constraint->negative == NEG_USE_GT) {
							LOG_NEG_RANGE("#%d UNKNOWN\n", j);
							constraint->negative = NEG_UNKNOWN;
						}
					} else if (tmp.max == ssa->var_info[j].range.max &&
				               tmp.min == ssa->var_info[j].range.min - 1 &&
					           tmp.min > constraint->range.max) {
						if (constraint->negative == NEG_INIT ||
						    constraint->negative == NEG_INVARIANT) {
							LOG_NEG_RANGE("#%d GT\n", j);
							constraint->negative = NEG_USE_GT;
//???NEG
						} else if (constraint->negative == NEG_USE_LT) {
							LOG_NEG_RANGE("#%d UNKNOWN\n", j);
							constraint->negative = NEG_UNKNOWN;
						}
					} else {
						LOG_NEG_RANGE("#%d UNKNOWN\n", j);
						constraint->negative = NEG_UNKNOWN;
					}
				}
#endif
				if (zend_inference_narrowing_meet(&ssa->var_info[j], &tmp)) {
					LOG_SSA_RANGE("  change range (warmup %2d SCC %2d) %2d [%s%ld..%ld%s]\n", n, scc, j, (tmp.underflow?"-- ":""), tmp.min, tmp.max, (tmp.overflow?" ++":""));
					zend_bitset_incl(visited, j);
					FOR_EACH_VAR_USAGE(j, ADD_SCC_VAR_1);
				}
			}
		} WHILE_WORKLIST_END();
	}
	free_alloca(worklist, use_heap);
}

static int zend_infer_ranges(const zend_op_array *op_array, zend_ssa *ssa) /* {{{ */
{
	int worklist_len = zend_bitset_len(ssa->vars_count);
	zend_bitset worklist;
	int *next_scc_var;
	int *scc_var;
	zend_ssa_phi *p;
	zend_ssa_range tmp;
	int scc, j;
	ALLOCA_FLAG(use_heap);

	worklist = do_alloca(
		ZEND_MM_ALIGNED_SIZE(sizeof(zend_ulong) * worklist_len) +
		ZEND_MM_ALIGNED_SIZE(sizeof(int) * ssa->vars_count) +
		sizeof(int) * ssa->sccs, use_heap);
	next_scc_var = (int*)((char*)worklist + ZEND_MM_ALIGNED_SIZE(sizeof(zend_ulong) * worklist_len));
	scc_var = (int*)((char*)next_scc_var + ZEND_MM_ALIGNED_SIZE(sizeof(int) * ssa->vars_count));

	LOG_SSA_RANGE("Range Inference\n");

	/* Create linked lists of SSA variables for each SCC */
	memset(scc_var, -1, sizeof(int) * ssa->sccs);
	for (j = 0; j < ssa->vars_count; j++) {
		if (ssa->vars[j].scc >= 0) {
			next_scc_var[j] = scc_var[ssa->vars[j].scc];
			scc_var[ssa->vars[j].scc] = j;
		}
	}

	for (scc = 0; scc < ssa->sccs; scc++) {
		j = scc_var[scc];
		if (next_scc_var[j] < 0) {
			/* SCC with a single element */
			if (zend_inference_calc_range(op_array, ssa, j, 0, 1, &tmp)) {
				zend_inference_init_range(op_array, ssa, j, tmp.underflow, tmp.min, tmp.max, tmp.overflow);
			} else {
				zend_inference_init_range(op_array, ssa, j, 1, ZEND_LONG_MIN, ZEND_LONG_MAX, 1);
			}
		} else {
			/* Find SCC entry points */
			memset(worklist, 0, sizeof(zend_ulong) * worklist_len);
			do {
				if (ssa->vars[j].scc_entry) {
					zend_bitset_incl(worklist, j);
				}
				j = next_scc_var[j];
			} while (j >= 0);

#if RANGE_WARMUP_PASSES > 0
			zend_infer_ranges_warmup(op_array, ssa, scc_var, next_scc_var, scc);
			j = scc_var[scc];
			do {
				zend_bitset_incl(worklist, j);
				j = next_scc_var[j];
			} while (j >= 0);
#endif

			/* widening */
			WHILE_WORKLIST(worklist, worklist_len, j) {
				if (zend_ssa_range_widening(op_array, ssa, j, scc)) {
					FOR_EACH_VAR_USAGE(j, ADD_SCC_VAR);
				}
			} WHILE_WORKLIST_END();

			/* Add all SCC entry variables into worklist for narrowing */
			for (j = scc_var[scc]; j >= 0; j = next_scc_var[j]) {
				if (!ssa->var_info[j].has_range) {
					zend_inference_init_range(op_array, ssa, j, 1, ZEND_LONG_MIN, ZEND_LONG_MAX, 1);
				}
				zend_bitset_incl(worklist, j);
			}

			/* narrowing */
			WHILE_WORKLIST(worklist, worklist_len, j) {
				if (zend_ssa_range_narrowing(op_array, ssa, j, scc)) {
					FOR_EACH_VAR_USAGE(j, ADD_SCC_VAR);
#ifdef SYM_RANGE
					/* Process symbolic control-flow constraints */
					p = ssa->vars[j].sym_use_chain;
					while (p) {
						ADD_SCC_VAR(p->ssa_var);
						p = p->sym_use_chain;
					}
#endif
				}
			} WHILE_WORKLIST_END();
		}
	}

	free_alloca(worklist, use_heap);

	return SUCCESS;
}
/* }}} */

#define UPDATE_SSA_TYPE(_type, _var)									\
	do {																\
		uint32_t __type = (_type);										\
		int __var = (_var);												\
		if (__type & MAY_BE_REF) {										\
			__type |= MAY_BE_RC1 | MAY_BE_RCN | MAY_BE_ANY | MAY_BE_ARRAY_KEY_ANY | MAY_BE_ARRAY_OF_ANY | MAY_BE_ARRAY_OF_REF; \
		}																\
		if (__var >= 0) {												\
			if (ssa_vars[__var].var < op_array->last_var) {				\
				if (__type & (MAY_BE_REF|MAY_BE_RCN)) {					\
					__type |= MAY_BE_RC1 | MAY_BE_RCN;					\
				}														\
				if ((__type & MAY_BE_RC1) && (__type & MAY_BE_STRING)) {\
					/* TODO: support for array keys and ($str . "")*/   \
					__type |= MAY_BE_RCN;                               \
				}                                                       \
			}															\
			if (ssa_var_info[__var].type != __type) { 					\
				if (ssa_var_info[__var].type & ~__type) {				\
					handle_type_narrowing(op_array, ssa, worklist,		\
						__var, ssa_var_info[__var].type, __type);		\
					return FAILURE;										\
				}														\
				ssa_var_info[__var].type = __type;						\
				add_usages(op_array, ssa, worklist, __var);				\
			}															\
			/*zend_bitset_excl(worklist, var);*/						\
		}																\
	} while (0)

#define UPDATE_SSA_OBJ_TYPE(_ce, _is_instanceof, var)				    \
	do {                                                                \
		if (var >= 0) {													\
			if (ssa_var_info[var].ce != (_ce) ||                        \
			    ssa_var_info[var].is_instanceof != (_is_instanceof)) {  \
				ssa_var_info[var].ce = (_ce);						    \
				ssa_var_info[var].is_instanceof = (_is_instanceof);     \
				add_usages(op_array, ssa, worklist, var);				\
			}															\
			/*zend_bitset_excl(worklist, var);*/						\
		}																\
	} while (0)

#define COPY_SSA_OBJ_TYPE(from_var, to_var) do { \
	if ((from_var) >= 0 && (ssa_var_info[(from_var)].type & MAY_BE_OBJECT) \
			&& ssa_var_info[(from_var)].ce) { \
		UPDATE_SSA_OBJ_TYPE(ssa_var_info[(from_var)].ce, \
			ssa_var_info[(from_var)].is_instanceof, (to_var)); \
	} else { \
		UPDATE_SSA_OBJ_TYPE(NULL, 0, (to_var)); \
	} \
} while (0)

static void add_usages(const zend_op_array *op_array, zend_ssa *ssa, zend_bitset worklist, int var)
{
	if (ssa->vars[var].phi_use_chain) {
		zend_ssa_phi *p = ssa->vars[var].phi_use_chain;
		do {
			zend_bitset_incl(worklist, p->ssa_var);
			p = zend_ssa_next_use_phi(ssa, var, p);
		} while (p);
	}
	if (ssa->vars[var].use_chain >= 0) {
		int use = ssa->vars[var].use_chain;
		zend_ssa_op *op;

		do {
			op = ssa->ops + use;
			if (op->result_def >= 0) {
				zend_bitset_incl(worklist, op->result_def);
			}
			if (op->op1_def >= 0) {
				zend_bitset_incl(worklist, op->op1_def);
			}
			if (op->op2_def >= 0) {
				zend_bitset_incl(worklist, op->op2_def);
			}
			if (op_array->opcodes[use].opcode == ZEND_OP_DATA) {
				op--;
				if (op->result_def >= 0) {
					zend_bitset_incl(worklist, op->result_def);
				}
				if (op->op1_def >= 0) {
					zend_bitset_incl(worklist, op->op1_def);
				}
				if (op->op2_def >= 0) {
					zend_bitset_incl(worklist, op->op2_def);
				}
			}
			use = zend_ssa_next_use(ssa->ops, var, use);
		} while (use >= 0);
	}
}

static void reset_dependent_vars(const zend_op_array *op_array, zend_ssa *ssa, zend_bitset worklist, int var)
{
	zend_ssa_op *ssa_ops = ssa->ops;
	zend_ssa_var *ssa_vars = ssa->vars;
	zend_ssa_var_info *ssa_var_info = ssa->var_info;
	zend_ssa_phi *p;
	int use;

	p = ssa_vars[var].phi_use_chain;
	while (p) {
		if (ssa_var_info[p->ssa_var].type) {
			ssa_var_info[p->ssa_var].type = 0;
			zend_bitset_incl(worklist, p->ssa_var);
			reset_dependent_vars(op_array, ssa, worklist, p->ssa_var);
		}
		p = zend_ssa_next_use_phi(ssa, var, p);
	}
	use = ssa_vars[var].use_chain;
	while (use >= 0) {
		if (ssa_ops[use].op1_def >= 0 && ssa_var_info[ssa_ops[use].op1_def].type) {
			ssa_var_info[ssa_ops[use].op1_def].type = 0;
			zend_bitset_incl(worklist, ssa_ops[use].op1_def);
			reset_dependent_vars(op_array, ssa, worklist, ssa_ops[use].op1_def);
		}
		if (ssa_ops[use].op2_def >= 0 && ssa_var_info[ssa_ops[use].op2_def].type) {
			ssa_var_info[ssa_ops[use].op2_def].type = 0;
			zend_bitset_incl(worklist, ssa_ops[use].op2_def);
			reset_dependent_vars(op_array, ssa, worklist, ssa_ops[use].op2_def);
		}
		if (ssa_ops[use].result_def >= 0 && ssa_var_info[ssa_ops[use].result_def].type) {
			ssa_var_info[ssa_ops[use].result_def].type = 0;
			zend_bitset_incl(worklist, ssa_ops[use].result_def);
			reset_dependent_vars(op_array, ssa, worklist, ssa_ops[use].result_def);
		}
		if (op_array->opcodes[use+1].opcode == ZEND_OP_DATA) {
			if (ssa_ops[use+1].op1_def >= 0 && ssa_var_info[ssa_ops[use+1].op1_def].type) {
				ssa_var_info[ssa_ops[use+1].op1_def].type = 0;
				zend_bitset_incl(worklist, ssa_ops[use+1].op1_def);
				reset_dependent_vars(op_array, ssa, worklist, ssa_ops[use+1].op1_def);
			}
			if (ssa_ops[use+1].op2_def >= 0 && ssa_var_info[ssa_ops[use+1].op2_def].type) {
				ssa_var_info[ssa_ops[use+1].op2_def].type = 0;
				zend_bitset_incl(worklist, ssa_ops[use+1].op2_def);
				reset_dependent_vars(op_array, ssa, worklist, ssa_ops[use+1].op2_def);
			}
			if (ssa_ops[use+1].result_def >= 0 && ssa_var_info[ssa_ops[use+1].result_def].type) {
				ssa_var_info[ssa_ops[use+1].result_def].type = 0;
				zend_bitset_incl(worklist, ssa_ops[use+1].result_def);
				reset_dependent_vars(op_array, ssa, worklist, ssa_ops[use+1].result_def);
			}
		}
		use = zend_ssa_next_use(ssa_ops, var, use);
	}
#ifdef SYM_RANGE
	/* Process symbolic control-flow constraints */
	p = ssa->vars[var].sym_use_chain;
	while (p) {
		ssa_var_info[p->ssa_var].type = 0;
		zend_bitset_incl(worklist, p->ssa_var);
		reset_dependent_vars(op_array, ssa, worklist, p->ssa_var);
		p = p->sym_use_chain;
	}
#endif
}

static void handle_type_narrowing(const zend_op_array *op_array, zend_ssa *ssa, zend_bitset worklist, int var, uint32_t old_type, uint32_t new_type)
{
	if (1) {
		/* Right now, this is always a bug */
		zend_error(E_WARNING, "Narrowing occurred during type inference. Please file a bug report on bugs.php.net");
	} else {
		/* if new_type set resets some bits from old_type set
		 * We have completely recalculate types of some dependent SSA variables
		 * (this may occurs mainly because of incremental inter-precudure
		 * type inference)
		 */
		reset_dependent_vars(op_array, ssa, worklist, var);
	}
}

uint32_t zend_array_element_type(uint32_t t1, int write, int insert)
{
	uint32_t tmp = 0;

	if (t1 & MAY_BE_OBJECT) {
		tmp |= MAY_BE_ANY | MAY_BE_REF | MAY_BE_RC1 | MAY_BE_RCN | MAY_BE_ARRAY_KEY_ANY | MAY_BE_ARRAY_OF_ANY | MAY_BE_ARRAY_OF_REF;
	}
	if (t1 & MAY_BE_ARRAY) {
		if (insert) {
			tmp |= MAY_BE_NULL;
		} else {
			tmp |= MAY_BE_NULL | ((t1 & MAY_BE_ARRAY_OF_ANY) >> MAY_BE_ARRAY_SHIFT);
			if (tmp & MAY_BE_ARRAY) {
				tmp |= MAY_BE_ARRAY_KEY_ANY | MAY_BE_ARRAY_OF_ANY | MAY_BE_ARRAY_OF_REF;
			}
			if (t1 & MAY_BE_ARRAY_OF_REF) {
				tmp |= MAY_BE_REF | MAY_BE_RC1 | MAY_BE_RCN;
			} else if (tmp & (MAY_BE_STRING|MAY_BE_ARRAY|MAY_BE_OBJECT|MAY_BE_RESOURCE)) {
				tmp |= MAY_BE_RC1 | MAY_BE_RCN;
			}
		}
	}
	if (t1 & MAY_BE_STRING) {
		tmp |= MAY_BE_STRING | MAY_BE_RC1;
		if (write) {
			tmp |= MAY_BE_NULL;
		}
	}
	if (t1 & (MAY_BE_UNDEF|MAY_BE_NULL|MAY_BE_FALSE)) {
		tmp |= MAY_BE_NULL;
		if (t1 & MAY_BE_ERROR) {
			if (write) {
				tmp |= MAY_BE_ERROR;
			}
		}
	}
	if (t1 & (MAY_BE_TRUE|MAY_BE_LONG|MAY_BE_DOUBLE|MAY_BE_RESOURCE)) {
		tmp |= MAY_BE_NULL;
		if (write) {
			tmp |= MAY_BE_ERROR;
		}
	}
	return tmp;
}

static uint32_t assign_dim_result_type(
		uint32_t arr_type, uint32_t dim_type, uint32_t value_type, zend_uchar dim_op_type) {
	uint32_t tmp = arr_type & ~(MAY_BE_RC1|MAY_BE_RCN);

	if (arr_type & (MAY_BE_UNDEF|MAY_BE_NULL|MAY_BE_FALSE)) {
		tmp &= ~(MAY_BE_UNDEF|MAY_BE_NULL|MAY_BE_FALSE);
		tmp |= MAY_BE_ARRAY|MAY_BE_RC1;
	}
	if (tmp & (MAY_BE_ARRAY|MAY_BE_STRING)) {
		tmp |= MAY_BE_RC1;
	}
	if (tmp & (MAY_BE_OBJECT|MAY_BE_RESOURCE)) {
		tmp |= MAY_BE_RC1 | MAY_BE_RCN;
	}
	if (tmp & MAY_BE_ARRAY) {
		tmp |= (value_type & MAY_BE_ANY) << MAY_BE_ARRAY_SHIFT;
		if (value_type & MAY_BE_UNDEF) {
			tmp |= MAY_BE_ARRAY_OF_NULL;
		}
		if (dim_op_type == IS_UNUSED) {
			tmp |= MAY_BE_ARRAY_KEY_LONG;
		} else {
			if (dim_type & (MAY_BE_LONG|MAY_BE_FALSE|MAY_BE_TRUE|MAY_BE_RESOURCE|MAY_BE_DOUBLE)) {
				tmp |= MAY_BE_ARRAY_KEY_LONG;
			}
			if (dim_type & MAY_BE_STRING) {
				tmp |= MAY_BE_ARRAY_KEY_STRING;
				if (dim_op_type != IS_CONST) {
					// FIXME: numeric string
					tmp |= MAY_BE_ARRAY_KEY_LONG;
				}
			}
			if (dim_type & (MAY_BE_UNDEF|MAY_BE_NULL)) {
				tmp |= MAY_BE_ARRAY_KEY_STRING;
			}
		}
	}
	return tmp;
}

/* For binary ops that have compound assignment operators */
static uint32_t binary_op_result_type(
		zend_ssa *ssa, zend_uchar opcode, uint32_t t1, uint32_t t2, uint32_t result_var) {
	uint32_t tmp = 0;
	uint32_t t1_type = (t1 & MAY_BE_ANY) | (t1 & MAY_BE_UNDEF ? MAY_BE_NULL : 0);
	uint32_t t2_type = (t2 & MAY_BE_ANY) | (t2 & MAY_BE_UNDEF ? MAY_BE_NULL : 0);
	switch (opcode) {
		case ZEND_ADD:
			if (t1_type == MAY_BE_LONG && t2_type == MAY_BE_LONG) {
				if (!ssa->var_info[result_var].has_range ||
				    ssa->var_info[result_var].range.underflow ||
				    ssa->var_info[result_var].range.overflow) {
					/* may overflow */
					tmp |= MAY_BE_LONG | MAY_BE_DOUBLE;
				} else {
					tmp |= MAY_BE_LONG;
				}
			} else if (t1_type == MAY_BE_DOUBLE || t2_type == MAY_BE_DOUBLE) {
				tmp |= MAY_BE_DOUBLE;
			} else if (t1_type == MAY_BE_ARRAY && t2_type == MAY_BE_ARRAY) {
				tmp |= MAY_BE_ARRAY | MAY_BE_RC1;
				tmp |= t1 & (MAY_BE_ARRAY_KEY_ANY|MAY_BE_ARRAY_OF_ANY|MAY_BE_ARRAY_OF_REF);
				tmp |= t2 & (MAY_BE_ARRAY_KEY_ANY|MAY_BE_ARRAY_OF_ANY|MAY_BE_ARRAY_OF_REF);
			} else {
				tmp |= MAY_BE_LONG | MAY_BE_DOUBLE;
				if ((t1_type & MAY_BE_ARRAY) && (t2_type & MAY_BE_ARRAY)) {
					tmp |= MAY_BE_ARRAY | MAY_BE_RC1;
					tmp |= t1 & (MAY_BE_ARRAY_KEY_ANY|MAY_BE_ARRAY_OF_ANY|MAY_BE_ARRAY_OF_REF);
					tmp |= t2 & (MAY_BE_ARRAY_KEY_ANY|MAY_BE_ARRAY_OF_ANY|MAY_BE_ARRAY_OF_REF);
				}
			}
			break;
		case ZEND_SUB:
		case ZEND_MUL:
			if (t1_type == MAY_BE_LONG && t2_type == MAY_BE_LONG) {
				if (!ssa->var_info[result_var].has_range ||
				    ssa->var_info[result_var].range.underflow ||
				    ssa->var_info[result_var].range.overflow) {
					/* may overflow */
					tmp |= MAY_BE_LONG | MAY_BE_DOUBLE;
				} else {
					tmp |= MAY_BE_LONG;
				}
			} else if (t1_type == MAY_BE_DOUBLE || t2_type == MAY_BE_DOUBLE) {
				tmp |= MAY_BE_DOUBLE;
			} else {
				tmp |= MAY_BE_LONG | MAY_BE_DOUBLE;
			}
			break;
		case ZEND_DIV:
		case ZEND_POW:
			if (t1_type == MAY_BE_DOUBLE || t2_type == MAY_BE_DOUBLE) {
				tmp |= MAY_BE_DOUBLE;
			} else {
				tmp |= MAY_BE_LONG | MAY_BE_DOUBLE;
			}
			/* Division by zero results in Inf/-Inf/Nan (double), so it doesn't need any special
			 * handling */
			break;
		case ZEND_MOD:
			tmp = MAY_BE_LONG;
			/* Division by zero results in an exception, so it doesn't need any special handling */
			break;
		case ZEND_BW_OR:
		case ZEND_BW_AND:
		case ZEND_BW_XOR:
			if ((t1_type & MAY_BE_STRING) && (t2_type & MAY_BE_STRING)) {
				tmp |= MAY_BE_STRING | MAY_BE_RC1;
			}
			if ((t1_type & ~MAY_BE_STRING) || (t2_type & ~MAY_BE_STRING)) {
				tmp |= MAY_BE_LONG;
			}
			break;
		case ZEND_SL:
		case ZEND_SR:
			tmp = MAY_BE_LONG;
			break;
		case ZEND_CONCAT:
		case ZEND_FAST_CONCAT:
			/* TODO: +MAY_BE_OBJECT ??? */
			tmp = MAY_BE_STRING | MAY_BE_RC1 | MAY_BE_RCN;
			break;
		EMPTY_SWITCH_DEFAULT_CASE()
	}
	return tmp;
}

static inline zend_class_entry *get_class_entry(const zend_script *script, zend_string *lcname) {
	zend_class_entry *ce = script ? zend_hash_find_ptr(&script->class_table, lcname) : NULL;
	if (ce) {
		return ce;
	}

	ce = zend_hash_find_ptr(CG(class_table), lcname);
	if (ce && ce->type == ZEND_INTERNAL_CLASS) {
		return ce;
	}

	return NULL;
}

static uint32_t zend_fetch_arg_info(const zend_script *script, zend_arg_info *arg_info, zend_class_entry **pce)
{
	uint32_t tmp = 0;

	*pce = NULL;
	if (ZEND_TYPE_IS_CLASS(arg_info->type)) {
		// class type hinting...
		zend_string *lcname = zend_string_tolower(ZEND_TYPE_NAME(arg_info->type));
		tmp |= MAY_BE_OBJECT;
		*pce = get_class_entry(script, lcname);
		zend_string_release(lcname);
	} else if (ZEND_TYPE_IS_CODE(arg_info->type)) {
		zend_uchar type_hint = ZEND_TYPE_CODE(arg_info->type);

		if (type_hint == IS_VOID) {
			tmp |= MAY_BE_NULL;
		} else if (type_hint == IS_CALLABLE) {
			tmp |= MAY_BE_STRING|MAY_BE_OBJECT|MAY_BE_ARRAY|MAY_BE_ARRAY_KEY_ANY|MAY_BE_ARRAY_OF_ANY|MAY_BE_ARRAY_OF_REF;
		} else if (type_hint == IS_ITERABLE) {
			tmp |= MAY_BE_OBJECT|MAY_BE_ARRAY|MAY_BE_ARRAY_KEY_ANY|MAY_BE_ARRAY_OF_ANY|MAY_BE_ARRAY_OF_REF;
		} else if (type_hint == IS_ARRAY) {
			tmp |= MAY_BE_ARRAY|MAY_BE_ARRAY_KEY_ANY|MAY_BE_ARRAY_OF_ANY|MAY_BE_ARRAY_OF_REF;
		} else if (type_hint == _IS_BOOL) {
			tmp |= MAY_BE_TRUE|MAY_BE_FALSE;
		} else {
			ZEND_ASSERT(type_hint < IS_REFERENCE);
			tmp |= 1 << type_hint;
		}
	} else {
		tmp |= MAY_BE_ANY|MAY_BE_ARRAY_KEY_ANY|MAY_BE_ARRAY_OF_ANY|MAY_BE_ARRAY_OF_REF;
	}
	if (ZEND_TYPE_ALLOW_NULL(arg_info->type)) {
		tmp |= MAY_BE_NULL;
	}
	return tmp;
}

static int zend_update_type_info(const zend_op_array *op_array,
                                  zend_ssa            *ssa,
                                  const zend_script   *script,
                                  zend_bitset          worklist,
                                  int                  i)
{
	uint32_t t1, t2;
	uint32_t tmp, orig;
	zend_op *opline = op_array->opcodes + i;
	zend_ssa_op *ssa_ops = ssa->ops;
	zend_ssa_var *ssa_vars = ssa->vars;
	zend_ssa_var_info *ssa_var_info = ssa->var_info;
	zend_class_entry *ce;
	int j;

	if (opline->opcode == ZEND_OP_DATA) {
		opline--;
		i--;
	}

	t1 = OP1_INFO();
	t2 = OP2_INFO();

	switch (opline->opcode) {
		case ZEND_ADD:
		case ZEND_SUB:
		case ZEND_MUL:
		case ZEND_DIV:
		case ZEND_POW:
		case ZEND_MOD:
		case ZEND_BW_OR:
		case ZEND_BW_AND:
		case ZEND_BW_XOR:
		case ZEND_SL:
		case ZEND_SR:
		case ZEND_CONCAT:
			tmp = binary_op_result_type(ssa, opline->opcode, t1, t2, ssa_ops[i].result_def);
			UPDATE_SSA_TYPE(tmp, ssa_ops[i].result_def);
			break;
		case ZEND_BW_NOT:
			tmp = 0;
			if (t1 & MAY_BE_STRING) {
				tmp |= MAY_BE_STRING | MAY_BE_RC1;
			}
			if (t1 & (MAY_BE_ANY-MAY_BE_STRING)) {
				tmp |= MAY_BE_LONG;
			}
			UPDATE_SSA_TYPE(tmp, ssa_ops[i].result_def);
			break;
		case ZEND_BEGIN_SILENCE:
			UPDATE_SSA_TYPE(MAY_BE_LONG, ssa_ops[i].result_def);
			break;
		case ZEND_BOOL_NOT:
		case ZEND_BOOL_XOR:
		case ZEND_IS_IDENTICAL:
		case ZEND_IS_NOT_IDENTICAL:
		case ZEND_IS_EQUAL:
		case ZEND_IS_NOT_EQUAL:
		case ZEND_IS_SMALLER:
		case ZEND_IS_SMALLER_OR_EQUAL:
		case ZEND_INSTANCEOF:
		case ZEND_JMPZ_EX:
		case ZEND_JMPNZ_EX:
		case ZEND_CASE:
		case ZEND_BOOL:
		case ZEND_ISSET_ISEMPTY_VAR:
		case ZEND_ISSET_ISEMPTY_DIM_OBJ:
		case ZEND_ISSET_ISEMPTY_PROP_OBJ:
		case ZEND_ISSET_ISEMPTY_STATIC_PROP:
		case ZEND_ASSERT_CHECK:
		case ZEND_IN_ARRAY:
			UPDATE_SSA_TYPE(MAY_BE_FALSE|MAY_BE_TRUE, ssa_ops[i].result_def);
			break;
		case ZEND_CAST:
			if (ssa_ops[i].op1_def >= 0) {
				tmp = t1;
				if ((t1 & (MAY_BE_ARRAY|MAY_BE_OBJECT)) &&
				    (opline->op1_type == IS_CV) &&
				    (opline->extended_value == IS_ARRAY ||
				     opline->extended_value == IS_OBJECT)) {
					tmp |= MAY_BE_RCN;
				} else if ((t1 & MAY_BE_STRING) &&
				    (opline->op1_type == IS_CV) &&
				    opline->extended_value == IS_STRING) {
					tmp |= MAY_BE_RCN;
				}
				UPDATE_SSA_TYPE(tmp, ssa_ops[i].op1_def);
				COPY_SSA_OBJ_TYPE(ssa_ops[i].op1_use, ssa_ops[i].op1_def);
			}
			tmp = 0;
			if (opline->extended_value == _IS_BOOL) {
				tmp |= MAY_BE_TRUE|MAY_BE_FALSE;
			} else {
				tmp |= 1 << opline->extended_value;
				if (tmp & (MAY_BE_STRING|MAY_BE_ARRAY|MAY_BE_OBJECT|MAY_BE_RESOURCE)) {
					if ((tmp & MAY_BE_ANY) == (t1 & MAY_BE_ANY)) {
						tmp |= (t1 & MAY_BE_RC1) | MAY_BE_RCN;
					} else if ((opline->extended_value == IS_ARRAY ||
					            opline->extended_value == IS_OBJECT) &&
					           (t1 & (MAY_BE_ARRAY|MAY_BE_OBJECT))) {
							tmp |= MAY_BE_RC1 | MAY_BE_RCN;
					} else if (opline->extended_value == IS_STRING &&
					           (t1 & (MAY_BE_STRING|MAY_BE_OBJECT))) {
						tmp |= MAY_BE_RC1 | MAY_BE_RCN;
					} else {
						tmp |= MAY_BE_RC1;
					}
				}
			}
			if (opline->extended_value == IS_ARRAY) {
				if (t1 & MAY_BE_ARRAY) {
					tmp |= t1 & (MAY_BE_ARRAY_KEY_ANY | MAY_BE_ARRAY_OF_ANY | MAY_BE_ARRAY_OF_REF);
				}
				if (t1 & MAY_BE_OBJECT) {
					tmp |= MAY_BE_ARRAY_KEY_ANY | MAY_BE_ARRAY_OF_ANY | MAY_BE_ARRAY_OF_REF;
				} else {
					tmp |= ((t1 & MAY_BE_ANY) << MAY_BE_ARRAY_SHIFT) | MAY_BE_ARRAY_KEY_LONG;
				}
			}
			UPDATE_SSA_TYPE(tmp, ssa_ops[i].result_def);
			break;
		case ZEND_QM_ASSIGN:
		case ZEND_JMP_SET:
		case ZEND_COALESCE:
			if (ssa_ops[i].op1_def >= 0) {
				tmp = t1;
				if ((t1 & (MAY_BE_RC1|MAY_BE_REF)) && (opline->op1_type == IS_CV)) {
					tmp |= MAY_BE_RCN;
				}
				UPDATE_SSA_TYPE(tmp, ssa_ops[i].op1_def);
				COPY_SSA_OBJ_TYPE(ssa_ops[i].op1_use, ssa_ops[i].op1_def);
			}
			tmp = t1 & ~(MAY_BE_UNDEF|MAY_BE_REF);
			if (t1 & MAY_BE_UNDEF) {
				tmp |= MAY_BE_NULL;
			}
			if (t1 & (MAY_BE_RC1|MAY_BE_RCN)) {
				tmp |= (t1 & (MAY_BE_RC1|MAY_BE_RCN));
				if (opline->op1_type == IS_CV) {
					tmp |= MAY_BE_RCN;
				}
			}
			if (opline->opcode != ZEND_QM_ASSIGN) {
				/* COALESCE and JMP_SET result can't be null */
				tmp &= ~MAY_BE_NULL;
				if (opline->opcode == ZEND_JMP_SET) {
					/* JMP_SET result can't be false either */
					tmp &= ~MAY_BE_FALSE;
				}
			}
			UPDATE_SSA_TYPE(tmp, ssa_ops[i].result_def);
			COPY_SSA_OBJ_TYPE(ssa_ops[i].op1_use, ssa_ops[i].result_def);
			break;
		case ZEND_ASSIGN_ADD:
		case ZEND_ASSIGN_SUB:
		case ZEND_ASSIGN_MUL:
		case ZEND_ASSIGN_DIV:
		case ZEND_ASSIGN_POW:
		case ZEND_ASSIGN_MOD:
		case ZEND_ASSIGN_SL:
		case ZEND_ASSIGN_SR:
		case ZEND_ASSIGN_BW_OR:
		case ZEND_ASSIGN_BW_AND:
		case ZEND_ASSIGN_BW_XOR:
		case ZEND_ASSIGN_CONCAT:
			orig = 0;
			tmp = 0;
			if (opline->extended_value == ZEND_ASSIGN_OBJ) {
		        tmp |= MAY_BE_REF;
				orig = t1;
				t1 = MAY_BE_ANY | MAY_BE_ARRAY_KEY_ANY | MAY_BE_ARRAY_OF_ANY | MAY_BE_ARRAY_OF_REF;
				t2 = OP1_DATA_INFO();
			} else if (opline->extended_value == ZEND_ASSIGN_DIM) {
				if (t1 & MAY_BE_ARRAY_OF_REF) {
			        tmp |= MAY_BE_REF;
				}
				orig = t1;
				t1 = zend_array_element_type(t1, 1, 0);
				t2 = OP1_DATA_INFO();
			} else {
				if (t1 & MAY_BE_REF) {
			        tmp |= MAY_BE_REF;
				}
			}

			tmp |= binary_op_result_type(
				ssa, get_compound_assign_op(opline->opcode), t1, t2, ssa_ops[i].op1_def);
			if (tmp & (MAY_BE_STRING|MAY_BE_ARRAY)) {
				tmp |= MAY_BE_RC1;
			}
			if (tmp & (MAY_BE_OBJECT|MAY_BE_RESOURCE)) {
				tmp |= MAY_BE_RC1 | MAY_BE_RCN;
			}

			if (opline->extended_value == ZEND_ASSIGN_DIM) {
				if (opline->op1_type == IS_CV) {
					orig = assign_dim_result_type(orig, OP2_INFO(), tmp, opline->op1_type);
					UPDATE_SSA_TYPE(orig, ssa_ops[i].op1_def);
					COPY_SSA_OBJ_TYPE(ssa_ops[i].op1_use, ssa_ops[i].op1_def);
				}
			} else if (opline->extended_value == ZEND_ASSIGN_OBJ) {
				if (opline->op1_type == IS_CV) {
					if (orig & (MAY_BE_UNDEF|MAY_BE_NULL|MAY_BE_FALSE)) {
						orig &= (MAY_BE_UNDEF|MAY_BE_NULL|MAY_BE_FALSE);
						orig |= MAY_BE_OBJECT | MAY_BE_RC1 | MAY_BE_RCN;
					}
					if (orig & MAY_BE_OBJECT) {
						orig |= (MAY_BE_RC1|MAY_BE_RCN);
					}
					UPDATE_SSA_TYPE(orig, ssa_ops[i].op1_def);
					COPY_SSA_OBJ_TYPE(ssa_ops[i].op1_use, ssa_ops[i].op1_def);
				}
			} else {
				UPDATE_SSA_TYPE(tmp, ssa_ops[i].op1_def);
			}
			if (ssa_ops[i].result_def >= 0) {
				if (opline->extended_value == ZEND_ASSIGN_DIM) {
					if (opline->op2_type == IS_UNUSED) {
						/* When appending to an array and the LONG_MAX key is already used
						 * null will be returned. */
						tmp |= MAY_BE_NULL;
					}
					if (t2 & (MAY_BE_ARRAY | MAY_BE_OBJECT)) {
						/* Arrays and objects cannot be used as keys. */
						tmp |= MAY_BE_NULL;
					}
					if (t1 & (MAY_BE_ANY - (MAY_BE_NULL | MAY_BE_FALSE | MAY_BE_STRING | MAY_BE_ARRAY))) {
						/* null and false are implicitly converted to array, anything else
						 * results in a null return value. */
						tmp |= MAY_BE_NULL;
					}
				} else if (opline->extended_value == ZEND_ASSIGN_OBJ) {
					if (orig & (MAY_BE_ANY - (MAY_BE_NULL | MAY_BE_FALSE | MAY_BE_OBJECT))) {
						/* null and false (and empty string) are implicitly converted to object,
						 * anything else results in a null return value. */
						tmp |= MAY_BE_NULL;
					}
				}
				UPDATE_SSA_TYPE(tmp, ssa_ops[i].result_def);
			}
			break;
		case ZEND_PRE_INC:
		case ZEND_PRE_DEC:
			tmp = 0;
			if (t1 & MAY_BE_REF) {
				tmp |= MAY_BE_REF;
			}
			if (t1 & (MAY_BE_RC1|MAY_BE_RCN)) {
				tmp |= MAY_BE_RC1;
				if (ssa_ops[i].result_def >= 0) {
					tmp |= MAY_BE_RCN;
				}
			}
			if ((t1 & MAY_BE_ANY) == MAY_BE_LONG) {
				if (!ssa_var_info[ssa_ops[i].op1_use].has_range ||
				    (opline->opcode == ZEND_PRE_DEC &&
				     (ssa_var_info[ssa_ops[i].op1_use].range.underflow ||
				      ssa_var_info[ssa_ops[i].op1_use].range.min == ZEND_LONG_MIN)) ||
				     (opline->opcode == ZEND_PRE_INC &&
				      (ssa_var_info[ssa_ops[i].op1_use].range.overflow ||
				       ssa_var_info[ssa_ops[i].op1_use].range.max == ZEND_LONG_MAX))) {
					/* may overflow */
					tmp |= MAY_BE_LONG | MAY_BE_DOUBLE;
				} else {
					tmp |= MAY_BE_LONG;
				}
			} else {
				if (t1 & MAY_BE_ERROR) {
					tmp |= MAY_BE_NULL;
				}
				if (t1 & (MAY_BE_UNDEF | MAY_BE_NULL)) {
					if (opline->opcode == ZEND_PRE_INC) {
						tmp |= MAY_BE_LONG;
					} else {
						tmp |= MAY_BE_NULL;
					}
				}
				if (t1 & MAY_BE_LONG) {
					tmp |= MAY_BE_LONG | MAY_BE_DOUBLE;
				}
				if (t1 & MAY_BE_DOUBLE) {
					tmp |= MAY_BE_DOUBLE;
				}
				if (t1 & MAY_BE_STRING) {
					tmp |= MAY_BE_STRING | MAY_BE_LONG | MAY_BE_DOUBLE;
				}
				tmp |= t1 & (MAY_BE_FALSE | MAY_BE_TRUE | MAY_BE_RESOURCE | MAY_BE_ARRAY | MAY_BE_OBJECT | MAY_BE_ARRAY_OF_ANY | MAY_BE_ARRAY_OF_REF | MAY_BE_ARRAY_KEY_ANY);
			}
			if (ssa_ops[i].op1_def >= 0) {
				UPDATE_SSA_TYPE(tmp, ssa_ops[i].op1_def);
			}
			if (ssa_ops[i].result_def >= 0) {
				UPDATE_SSA_TYPE(tmp, ssa_ops[i].result_def);
			}
			break;
		case ZEND_POST_INC:
		case ZEND_POST_DEC:
			if (ssa_ops[i].result_def >= 0) {
				tmp = 0;
				if (t1 & (MAY_BE_RC1|MAY_BE_RCN)) {
					tmp |= MAY_BE_RC1|MAY_BE_RCN;
				}
				tmp |= t1 & ~(MAY_BE_UNDEF|MAY_BE_ERROR|MAY_BE_REF|MAY_BE_RCN);
				if (t1 & MAY_BE_UNDEF) {
					tmp |= MAY_BE_NULL;
				}
				UPDATE_SSA_TYPE(tmp, ssa_ops[i].result_def);
			}
			tmp = 0;
			if (t1 & MAY_BE_REF) {
				tmp |= MAY_BE_REF;
			}
			if (t1 & (MAY_BE_RC1|MAY_BE_RCN)) {
				tmp |= MAY_BE_RC1;
			}
			if ((t1 & MAY_BE_ANY) == MAY_BE_LONG) {
				if (!ssa_var_info[ssa_ops[i].op1_use].has_range ||
				     (opline->opcode == ZEND_PRE_DEC &&
				      (ssa_var_info[ssa_ops[i].op1_use].range.underflow ||
				       ssa_var_info[ssa_ops[i].op1_use].range.min == ZEND_LONG_MIN)) ||
				      (opline->opcode == ZEND_PRE_INC &&
				       (ssa_var_info[ssa_ops[i].op1_use].range.overflow ||
				        ssa_var_info[ssa_ops[i].op1_use].range.max == ZEND_LONG_MAX))) {
					/* may overflow */
					tmp |= MAY_BE_LONG | MAY_BE_DOUBLE;
				} else {
					tmp |= MAY_BE_LONG;
				}
			} else {
				if (t1 & MAY_BE_ERROR) {
					tmp |= MAY_BE_NULL;
				}
				if (t1 & (MAY_BE_UNDEF | MAY_BE_NULL)) {
					if (opline->opcode == ZEND_POST_INC) {
						tmp |= MAY_BE_LONG;
					} else {
						tmp |= MAY_BE_NULL;
					}
				}
				if (t1 & MAY_BE_LONG) {
					tmp |= MAY_BE_LONG | MAY_BE_DOUBLE;
				}
				if (t1 & MAY_BE_DOUBLE) {
					tmp |= MAY_BE_DOUBLE;
				}
				if (t1 & MAY_BE_STRING) {
					tmp |= MAY_BE_STRING | MAY_BE_LONG | MAY_BE_DOUBLE;
				}
				tmp |= t1 & (MAY_BE_FALSE | MAY_BE_TRUE | MAY_BE_RESOURCE | MAY_BE_ARRAY | MAY_BE_OBJECT | MAY_BE_ARRAY_OF_ANY | MAY_BE_ARRAY_OF_REF | MAY_BE_ARRAY_KEY_ANY);
			}
			if (ssa_ops[i].op1_def >= 0) {
				UPDATE_SSA_TYPE(tmp, ssa_ops[i].op1_def);
			}
			break;
		case ZEND_ASSIGN_DIM:
			if (opline->op1_type == IS_CV) {
				tmp = assign_dim_result_type(t1, t2, OP1_DATA_INFO(), opline->op2_type);
				UPDATE_SSA_TYPE(tmp, ssa_ops[i].op1_def);
				COPY_SSA_OBJ_TYPE(ssa_ops[i].op1_use, ssa_ops[i].op1_def);
			}
			if (ssa_ops[i].result_def >= 0) {
				tmp = 0;
				if (t1 & MAY_BE_STRING) {
					tmp |= MAY_BE_STRING;
				}
				if (t1 & ((MAY_BE_ANY|MAY_BE_UNDEF) - MAY_BE_STRING)) {
					tmp |= (OP1_DATA_INFO() & (MAY_BE_ANY | MAY_BE_ARRAY_KEY_ANY | MAY_BE_ARRAY_OF_ANY | MAY_BE_ARRAY_OF_REF));

					if (opline->op2_type == IS_UNUSED) {
						/* When appending to an array and the LONG_MAX key is already used
						 * null will be returned. */
						tmp |= MAY_BE_NULL;
					}
					if (t2 & (MAY_BE_ARRAY | MAY_BE_OBJECT)) {
						/* Arrays and objects cannot be used as keys. */
						tmp |= MAY_BE_NULL;
					}
					if (t1 & (MAY_BE_ANY - (MAY_BE_NULL | MAY_BE_FALSE | MAY_BE_STRING | MAY_BE_ARRAY))) {
						/* undef, null and false are implicitly converted to array, anything else
						 * results in a null return value. */
						tmp |= MAY_BE_NULL;
					}
				}
				tmp |= MAY_BE_RC1 | MAY_BE_RCN;
				if (t1 & MAY_BE_OBJECT) {
					tmp |= MAY_BE_REF;
				}
				UPDATE_SSA_TYPE(tmp, ssa_ops[i].result_def);
			}
			if ((opline+1)->op1_type == IS_CV && ssa_ops[i+1].op1_def >= 0) {
				opline++;
				i++;
				tmp = OP1_INFO();
				if (tmp & (MAY_BE_ANY | MAY_BE_REF)) {
					if (tmp & MAY_BE_RC1) {
						tmp |= MAY_BE_RCN;
					}
				}
				UPDATE_SSA_TYPE(tmp, ssa_ops[i].op1_def);
			}
			break;
		case ZEND_ASSIGN_OBJ:
			if (opline->op1_type == IS_CV) {
				tmp = t1;
				if (t1 & (MAY_BE_UNDEF|MAY_BE_NULL|MAY_BE_FALSE)) {
					tmp &= ~(MAY_BE_UNDEF|MAY_BE_NULL|MAY_BE_FALSE);
					tmp |= MAY_BE_OBJECT | MAY_BE_RC1 | MAY_BE_RCN;
				}
				if (tmp & MAY_BE_OBJECT) {
					tmp |= MAY_BE_RC1 | MAY_BE_RCN;
				}
				UPDATE_SSA_TYPE(tmp, ssa_ops[i].op1_def);
				COPY_SSA_OBJ_TYPE(ssa_ops[i].op1_use, ssa_ops[i].op1_def);
			}
			if (ssa_ops[i].result_def >= 0) {
				// TODO: ???
				tmp = MAY_BE_REF | MAY_BE_RC1 | MAY_BE_RCN | MAY_BE_ANY | MAY_BE_ARRAY_KEY_ANY | MAY_BE_ARRAY_OF_ANY | MAY_BE_ARRAY_OF_REF;
				UPDATE_SSA_TYPE(tmp, ssa_ops[i].result_def);
			}
			if ((opline+1)->op1_type == IS_CV) {
				opline++;
				i++;
				tmp = OP1_INFO();
				if (tmp & (MAY_BE_ANY | MAY_BE_REF)) {
					if (tmp & MAY_BE_RC1) {
						tmp |= MAY_BE_RCN;
					}
				}
				UPDATE_SSA_TYPE(tmp, ssa_ops[i].op1_def);
			}
			break;
		case ZEND_ASSIGN:
			if (opline->op2_type == IS_CV && ssa_ops[i].op2_def >= 0) {
				tmp = t2;
				if (tmp & (MAY_BE_ANY | MAY_BE_REF)) {
					if (tmp & MAY_BE_RC1) {
						tmp |= MAY_BE_RCN;
					}
				}
				UPDATE_SSA_TYPE(tmp, ssa_ops[i].op2_def);
			}
			tmp = t2 & ~(MAY_BE_UNDEF|MAY_BE_REF|MAY_BE_RC1|MAY_BE_RCN);
			if (t2 & MAY_BE_UNDEF) {
				tmp |= MAY_BE_NULL;
			}
			if (t1 & MAY_BE_REF) {
				tmp |= MAY_BE_REF;
			}
			if (t2 & MAY_BE_REF) {
				tmp |= MAY_BE_RC1 | MAY_BE_RCN;
			} else if (opline->op2_type & (IS_TMP_VAR|IS_VAR)) {
				tmp |= t2 & (MAY_BE_RC1|MAY_BE_RCN);
			} else if (t2 & (MAY_BE_RC1|MAY_BE_RCN)) {
				tmp |= MAY_BE_RCN;
			}
			if (RETURN_VALUE_USED(opline) && (tmp & MAY_BE_RC1)) {
				tmp |= MAY_BE_RCN;
			}
			if (ssa_ops[i].op1_def >= 0) {
				if (ssa_var_info[ssa_ops[i].op1_def].use_as_double) {
					tmp &= ~MAY_BE_LONG;
					tmp |= MAY_BE_DOUBLE;
				}
				UPDATE_SSA_TYPE(tmp, ssa_ops[i].op1_def);
				COPY_SSA_OBJ_TYPE(ssa_ops[i].op2_use, ssa_ops[i].op1_def);
			}
			if (ssa_ops[i].result_def >= 0) {
				UPDATE_SSA_TYPE(tmp & ~MAY_BE_REF, ssa_ops[i].result_def);
				COPY_SSA_OBJ_TYPE(ssa_ops[i].op2_use, ssa_ops[i].result_def);
			}
			break;
		case ZEND_ASSIGN_REF:
// TODO: ???
			if (opline->op2_type == IS_CV) {
				tmp = (MAY_BE_REF | t2) & ~(MAY_BE_UNDEF|MAY_BE_RC1|MAY_BE_RCN);
				if (t2 & MAY_BE_UNDEF) {
					tmp |= MAY_BE_NULL;
				}
				UPDATE_SSA_TYPE(tmp, ssa_ops[i].op2_def);
			}
			if (opline->op2_type == IS_VAR && opline->extended_value == ZEND_RETURNS_FUNCTION) {
				tmp = (MAY_BE_REF | MAY_BE_RCN | MAY_BE_RC1 | t2) & ~MAY_BE_UNDEF;
			} else {
				tmp = (MAY_BE_REF | t2) & ~(MAY_BE_UNDEF|MAY_BE_ERROR|MAY_BE_RC1|MAY_BE_RCN);
			}
			if (t2 & MAY_BE_UNDEF) {
				tmp |= MAY_BE_NULL;
			}
			UPDATE_SSA_TYPE(tmp, ssa_ops[i].op1_def);
			if (ssa_ops[i].result_def >= 0) {
				UPDATE_SSA_TYPE(tmp, ssa_ops[i].result_def);
			}
			break;
		case ZEND_BIND_GLOBAL:
			tmp = MAY_BE_REF | MAY_BE_ANY
				| MAY_BE_ARRAY_KEY_ANY | MAY_BE_ARRAY_OF_ANY | MAY_BE_ARRAY_OF_REF;
			UPDATE_SSA_TYPE(tmp, ssa_ops[i].op1_def);
			break;
		case ZEND_BIND_STATIC:
			tmp = MAY_BE_ANY | MAY_BE_ARRAY_KEY_ANY | MAY_BE_ARRAY_OF_ANY | MAY_BE_ARRAY_OF_REF
				| (opline->extended_value ? MAY_BE_REF : (MAY_BE_RC1 | MAY_BE_RCN));
			UPDATE_SSA_TYPE(tmp, ssa_ops[i].op1_def);
			break;
		case ZEND_SEND_VAR:
			if (ssa_ops[i].op1_def >= 0) {
				tmp = t1;
				if ((t1 & (MAY_BE_RC1|MAY_BE_REF)) && (opline->op1_type == IS_CV)) {
					tmp |= MAY_BE_RCN;
				}
				UPDATE_SSA_TYPE(tmp, ssa_ops[i].op1_def);
				COPY_SSA_OBJ_TYPE(ssa_ops[i].op1_use, ssa_ops[i].op1_def);
			}
			break;
		case ZEND_BIND_LEXICAL:
			if (ssa_ops[i].op2_def >= 0) {
				if (opline->extended_value) {
					tmp = t2 | MAY_BE_REF;
				} else {
					tmp = t2 & ~(MAY_BE_RC1|MAY_BE_RCN);
					if (t2 & (MAY_BE_RC1|MAY_BE_RCN)) {
						tmp |= MAY_BE_RCN;
					}
				}
				UPDATE_SSA_TYPE(tmp, ssa_ops[i].op2_def);
				COPY_SSA_OBJ_TYPE(ssa_ops[i].op2_use, ssa_ops[i].op2_def);
			}
			break;
		case ZEND_YIELD:
			if (ssa_ops[i].op1_def >= 0) {
				if (op_array->fn_flags & ZEND_ACC_RETURN_REFERENCE) {
					tmp = t1 | MAY_BE_REF;
				} else {
					tmp = t1 & ~(MAY_BE_RC1|MAY_BE_RCN);
					if (t1 & (MAY_BE_RC1|MAY_BE_RCN)) {
						tmp |= MAY_BE_RCN;
					}
				}
				UPDATE_SSA_TYPE(tmp, ssa_ops[i].op1_def);
				COPY_SSA_OBJ_TYPE(ssa_ops[i].op1_use, ssa_ops[i].op1_def);
			}
			if (ssa_ops[i].result_def >= 0) {
				tmp = MAY_BE_ANY | MAY_BE_ARRAY_KEY_ANY | MAY_BE_ARRAY_OF_ANY | MAY_BE_ARRAY_OF_REF
					| MAY_BE_RC1 | MAY_BE_RCN;
				UPDATE_SSA_TYPE(tmp, ssa_ops[i].result_def);
			}
			break;
		case ZEND_SEND_VAR_EX:
			if (ssa_ops[i].op1_def >= 0) {
				tmp = (t1 & MAY_BE_UNDEF)|MAY_BE_REF|MAY_BE_RC1|MAY_BE_RCN|MAY_BE_ANY|MAY_BE_ARRAY_KEY_ANY|MAY_BE_ARRAY_OF_ANY|MAY_BE_ARRAY_OF_REF;
				UPDATE_SSA_TYPE(tmp, ssa_ops[i].op1_def);
			}
			break;
		case ZEND_SEND_REF:
			if (ssa_ops[i].op1_def >= 0) {
				tmp = MAY_BE_REF|MAY_BE_RC1|MAY_BE_RCN|MAY_BE_ANY|MAY_BE_ARRAY_KEY_ANY|MAY_BE_ARRAY_OF_ANY|MAY_BE_ARRAY_OF_REF;
				UPDATE_SSA_TYPE(tmp, ssa_ops[i].op1_def);
			}
			break;
		case ZEND_SEND_UNPACK:
			if (ssa_ops[i].op1_def >= 0) {
				tmp = t1;
				if (t1 & MAY_BE_ARRAY) {
					tmp |= MAY_BE_RC1 | MAY_BE_RCN;
					/* SEND_UNPACK may acquire references into the array */
					tmp |= MAY_BE_ARRAY_OF_ANY | MAY_BE_ARRAY_OF_REF;
				}
				if (t1 & MAY_BE_OBJECT) {
					tmp |= MAY_BE_RC1 | MAY_BE_RCN;
				}
				UPDATE_SSA_TYPE(tmp, ssa_ops[i].op1_def);
			}
			break;
		case ZEND_FAST_CONCAT:
		case ZEND_ROPE_INIT:
		case ZEND_ROPE_ADD:
		case ZEND_ROPE_END:
			UPDATE_SSA_TYPE(MAY_BE_STRING|MAY_BE_RC1|MAY_BE_RCN, ssa_ops[i].result_def);
			break;
		case ZEND_RECV:
		case ZEND_RECV_INIT:
		{
			/* Typehinting */
			zend_func_info *func_info;
			zend_arg_info *arg_info = NULL;
			if (op_array->arg_info && opline->op1.num <= op_array->num_args) {
				arg_info = &op_array->arg_info[opline->op1.num-1];
			}

			ce = NULL;
			if (arg_info) {
				tmp = zend_fetch_arg_info(script, arg_info, &ce);
				if (opline->opcode == ZEND_RECV_INIT &&
				           Z_CONSTANT_P(CRT_CONSTANT_EX(op_array, opline->op2, ssa->rt_constants))) {
					/* The constant may resolve to NULL */
					tmp |= MAY_BE_NULL;
				}
				if (arg_info->pass_by_reference) {
					tmp |= MAY_BE_REF;
				} else if (tmp & (MAY_BE_STRING|MAY_BE_ARRAY|MAY_BE_OBJECT|MAY_BE_RESOURCE)) {
					tmp |= MAY_BE_RC1|MAY_BE_RCN;
				}
			} else {
				tmp = MAY_BE_REF|MAY_BE_RC1|MAY_BE_RCN|MAY_BE_ANY|MAY_BE_ARRAY_KEY_ANY|MAY_BE_ARRAY_OF_ANY|MAY_BE_ARRAY_OF_REF;
			}
			func_info = ZEND_FUNC_INFO(op_array);
			if (func_info && (int)opline->op1.num-1 < func_info->num_args) {
				tmp = (tmp & (MAY_BE_RC1|MAY_BE_RCN|MAY_BE_REF)) |
					(tmp & func_info->arg_info[opline->op1.num-1].info.type);
			}
#if 0
			/* We won't recieve unused arguments */
			if (ssa_vars[ssa_ops[i].result_def].use_chain < 0 &&
			    ssa_vars[ssa_ops[i].result_def].phi_use_chain == NULL &&
			    op_array->arg_info &&
			    opline->op1.num <= op_array->num_args &&
			    op_array->arg_info[opline->op1.num-1].class_name == NULL &&
			    !op_array->arg_info[opline->op1.num-1].type_hint) {
				tmp = MAY_BE_UNDEF|MAY_BE_RCN;
			}
#endif
			UPDATE_SSA_TYPE(tmp, ssa_ops[i].result_def);
			if (func_info &&
			    (int)opline->op1.num-1 < func_info->num_args &&
			    func_info->arg_info[opline->op1.num-1].info.ce) {
				UPDATE_SSA_OBJ_TYPE(
					func_info->arg_info[opline->op1.num-1].info.ce,
					func_info->arg_info[opline->op1.num-1].info.is_instanceof,
					ssa_ops[i].result_def);
			} else if (ce) {
				UPDATE_SSA_OBJ_TYPE(ce, 1, ssa_ops[i].result_def);
			} else {
				UPDATE_SSA_OBJ_TYPE(NULL, 0, ssa_ops[i].result_def);
			}
			break;
		}
		case ZEND_DECLARE_CLASS:
		case ZEND_DECLARE_INHERITED_CLASS:
		case ZEND_DECLARE_ANON_CLASS:
		case ZEND_DECLARE_ANON_INHERITED_CLASS:
			UPDATE_SSA_TYPE(MAY_BE_CLASS, ssa_ops[i].result_def);
			if (script && (ce = zend_hash_find_ptr(&script->class_table, Z_STR_P(CRT_CONSTANT_EX(op_array, opline->op1, ssa->rt_constants)))) != NULL) {
				UPDATE_SSA_OBJ_TYPE(ce, 0, ssa_ops[i].result_def);
			}
			break;
		case ZEND_FETCH_CLASS:
			UPDATE_SSA_TYPE(MAY_BE_CLASS, ssa_ops[i].result_def);
			if (opline->op2_type == IS_UNUSED) {
				switch (opline->extended_value & ZEND_FETCH_CLASS_MASK) {
					case ZEND_FETCH_CLASS_SELF:
						if (op_array->scope) {
							UPDATE_SSA_OBJ_TYPE(op_array->scope, 0, ssa_ops[i].result_def);
						} else {
							UPDATE_SSA_OBJ_TYPE(NULL, 0, ssa_ops[i].result_def);
						}
						break;
					case ZEND_FETCH_CLASS_PARENT:
						if (op_array->scope && op_array->scope->parent) {
							UPDATE_SSA_OBJ_TYPE(op_array->scope->parent, 0, ssa_ops[i].result_def);
						} else {
							UPDATE_SSA_OBJ_TYPE(NULL, 0, ssa_ops[i].result_def);
						}
						break;
					case ZEND_FETCH_CLASS_STATIC:
					default:
						UPDATE_SSA_OBJ_TYPE(NULL, 0, ssa_ops[i].result_def);
						break;
				}
			} else if (opline->op2_type == IS_CONST) {
				zval *zv = CRT_CONSTANT_EX(op_array, opline->op2, ssa->rt_constants);
				if (Z_TYPE_P(zv) == IS_STRING) {
					ce = get_class_entry(script, Z_STR_P(zv+1));
					UPDATE_SSA_OBJ_TYPE(ce, 0, ssa_ops[i].result_def);
				} else {
					UPDATE_SSA_OBJ_TYPE(NULL, 0, ssa_ops[i].result_def);
				}
			} else {
				COPY_SSA_OBJ_TYPE(ssa_ops[i].op2_use, ssa_ops[i].result_def);
			}
			break;
		case ZEND_NEW:
			tmp = MAY_BE_RC1|MAY_BE_RCN|MAY_BE_OBJECT;
			if (opline->op1_type == IS_CONST &&
			    (ce = get_class_entry(script, Z_STR_P(CRT_CONSTANT_EX(op_array, opline->op1, ssa->rt_constants)+1))) != NULL) {
				UPDATE_SSA_OBJ_TYPE(ce, 0, ssa_ops[i].result_def);
			} else if ((t1 & MAY_BE_CLASS) && ssa_ops[i].op1_use >= 0 && ssa_var_info[ssa_ops[i].op1_use].ce) {
				UPDATE_SSA_OBJ_TYPE(ssa_var_info[ssa_ops[i].op1_use].ce, ssa_var_info[ssa_ops[i].op1_use].is_instanceof, ssa_ops[i].result_def);
			} else {
				UPDATE_SSA_OBJ_TYPE(NULL, 0, ssa_ops[i].result_def);
			}
			UPDATE_SSA_TYPE(tmp, ssa_ops[i].result_def);
			break;
		case ZEND_CLONE:
			UPDATE_SSA_TYPE(MAY_BE_RC1|MAY_BE_RCN|MAY_BE_OBJECT, ssa_ops[i].result_def);
			COPY_SSA_OBJ_TYPE(ssa_ops[i].op1_use, ssa_ops[i].result_def);
			break;
		case ZEND_INIT_ARRAY:
		case ZEND_ADD_ARRAY_ELEMENT:
			if (opline->op1_type == IS_CV && ssa_ops[i].op1_def >= 0) {
				if (opline->extended_value & ZEND_ARRAY_ELEMENT_REF) {
					tmp = (MAY_BE_REF | t1) & ~(MAY_BE_UNDEF|MAY_BE_RC1|MAY_BE_RCN);
					if (t1 & MAY_BE_UNDEF) {
						tmp |= MAY_BE_NULL;
					}
				} else if ((t1 & (MAY_BE_REF|MAY_BE_RC1|MAY_BE_RCN)) == MAY_BE_REF) {
					tmp = (MAY_BE_REF | t1) & ~(MAY_BE_UNDEF|MAY_BE_RC1|MAY_BE_RCN);
					if (t1 & MAY_BE_UNDEF) {
						tmp |= MAY_BE_NULL;
					}
				} else if (t1 & MAY_BE_REF) {
					tmp = (MAY_BE_RC1 | MAY_BE_RCN | MAY_BE_REF | t1);
				} else {
					tmp = t1;
					if (t1 & MAY_BE_RC1) {
						tmp |= MAY_BE_RCN;
					}
				}
				UPDATE_SSA_TYPE(tmp, ssa_ops[i].op1_def);
			}
			if (ssa_ops[i].result_def >= 0) {
				tmp = MAY_BE_RC1|MAY_BE_ARRAY;
				if (opline->op1_type != IS_UNUSED) {
					tmp |= (t1 & MAY_BE_ANY) << MAY_BE_ARRAY_SHIFT;
					if (t1 & MAY_BE_UNDEF) {
						tmp |= MAY_BE_ARRAY_OF_NULL;
					}
					if (opline->extended_value & ZEND_ARRAY_ELEMENT_REF) {
						tmp |= MAY_BE_ARRAY_OF_ANY|MAY_BE_ARRAY_OF_REF;
					}
				}
				if (ssa_ops[i].result_use >= 0) {
					tmp |= ssa_var_info[ssa_ops[i].result_use].type;
				}
				if (opline->op2_type == IS_UNUSED) {
					tmp |= MAY_BE_ARRAY_KEY_LONG;
				} else {
					if (t2 & (MAY_BE_LONG|MAY_BE_FALSE|MAY_BE_TRUE|MAY_BE_DOUBLE)) {
						tmp |= MAY_BE_ARRAY_KEY_LONG;
					}
					if (t2 & (MAY_BE_STRING)) {
						tmp |= MAY_BE_ARRAY_KEY_STRING;
						if (opline->op2_type != IS_CONST) {
							// FIXME: numeric string
							tmp |= MAY_BE_ARRAY_KEY_LONG;
						}
					}
					if (t2 & (MAY_BE_UNDEF | MAY_BE_NULL)) {
						tmp |= MAY_BE_ARRAY_KEY_STRING;
					}
				}
				UPDATE_SSA_TYPE(tmp, ssa_ops[i].result_def);
			}
			break;
		case ZEND_UNSET_VAR:
			ZEND_ASSERT(opline->extended_value & ZEND_QUICK_SET);
			tmp = MAY_BE_UNDEF;
			if (!op_array->function_name) {
				/* In global scope, we know nothing */
				tmp |= MAY_BE_REF;
			}
			UPDATE_SSA_TYPE(tmp, ssa_ops[i].op1_def);
			break;
		case ZEND_UNSET_DIM:
		case ZEND_UNSET_OBJ:
			if (ssa_ops[i].op1_def >= 0) {
				UPDATE_SSA_TYPE(t1, ssa_ops[i].op1_def);
				COPY_SSA_OBJ_TYPE(ssa_ops[i].op1_use, ssa_ops[i].op1_def);
			}
			break;
		case ZEND_FE_RESET_R:
		case ZEND_FE_RESET_RW:
			if (ssa_ops[i].op1_def >= 0) {
				tmp = t1;
				if (opline->opcode == ZEND_FE_RESET_RW) {
					tmp |= MAY_BE_REF;
				} else {
					if ((t1 & MAY_BE_RC1) && opline->op1_type != IS_TMP_VAR) {
						tmp |= MAY_BE_RCN;
					}
				}
				UPDATE_SSA_TYPE(tmp, ssa_ops[i].op1_def);
				COPY_SSA_OBJ_TYPE(ssa_ops[i].op1_use, ssa_ops[i].op1_def);
			}
			if (opline->opcode == ZEND_FE_RESET_RW) {
//???
				tmp = MAY_BE_REF | (t1 & (MAY_BE_ARRAY | MAY_BE_OBJECT));
			} else {
				tmp = MAY_BE_RC1 | MAY_BE_RCN | (t1 & (MAY_BE_ARRAY | MAY_BE_OBJECT | MAY_BE_ARRAY_KEY_ANY | MAY_BE_ARRAY_OF_ANY | MAY_BE_ARRAY_OF_REF));
			}
			UPDATE_SSA_TYPE(tmp, ssa_ops[i].result_def);
			COPY_SSA_OBJ_TYPE(ssa_ops[i].op1_use, ssa_ops[i].result_def);
			break;
		case ZEND_FE_FETCH_R:
		case ZEND_FE_FETCH_RW:
			tmp = (t2 & MAY_BE_REF);
			if (t1 & MAY_BE_OBJECT) {
				if (opline->opcode == ZEND_FE_FETCH_RW) {
					tmp |= MAY_BE_REF | MAY_BE_ANY | MAY_BE_ARRAY_KEY_ANY | MAY_BE_ARRAY_OF_ANY | MAY_BE_ARRAY_OF_REF;
				} else {
					tmp |= MAY_BE_REF | MAY_BE_RCN | MAY_BE_ANY | MAY_BE_ARRAY_KEY_ANY | MAY_BE_ARRAY_OF_ANY | MAY_BE_ARRAY_OF_REF;
				}
			}
			if (t1 & MAY_BE_ARRAY) {
				if (opline->opcode == ZEND_FE_FETCH_RW) {
					tmp |= MAY_BE_REF | MAY_BE_RCN | MAY_BE_ANY | MAY_BE_ARRAY_KEY_ANY | MAY_BE_ARRAY_OF_ANY | MAY_BE_ARRAY_OF_REF;
				} else {
					tmp |= ((t1 & MAY_BE_ARRAY_OF_ANY) >> MAY_BE_ARRAY_SHIFT);
					if (tmp & MAY_BE_ARRAY) {
						tmp |= MAY_BE_ARRAY_KEY_ANY | MAY_BE_ARRAY_OF_ANY | MAY_BE_ARRAY_OF_REF;
					}
					if (t1 & MAY_BE_ARRAY_OF_REF) {
						tmp |= MAY_BE_RC1 | MAY_BE_RCN;
					} else if (tmp & (MAY_BE_STRING|MAY_BE_ARRAY|MAY_BE_OBJECT|MAY_BE_RESOURCE)) {
						tmp |= MAY_BE_RC1 | MAY_BE_RCN;
					}
				}
			}
			UPDATE_SSA_TYPE(tmp, ssa_ops[i].op2_def);
			if (ssa_ops[i].result_def >= 0) {
				tmp = 0;
				if (t1 & MAY_BE_OBJECT) {
					tmp |= MAY_BE_RC1 | MAY_BE_RCN | MAY_BE_ANY | MAY_BE_ARRAY_KEY_ANY | MAY_BE_ARRAY_OF_ANY | MAY_BE_ARRAY_OF_REF;
				}
				if (t1 & MAY_BE_ARRAY) {
					if (t1 & MAY_BE_ARRAY_KEY_LONG) {
						tmp |= MAY_BE_LONG;
					}
					if (t1 & MAY_BE_ARRAY_KEY_STRING) {
						tmp |= MAY_BE_STRING | MAY_BE_RCN;
					}
				}
				UPDATE_SSA_TYPE(tmp, ssa_ops[i].result_def);
			}
			break;
		case ZEND_FETCH_DIM_R:
		case ZEND_FETCH_DIM_IS:
		case ZEND_FETCH_DIM_RW:
		case ZEND_FETCH_DIM_W:
		case ZEND_FETCH_DIM_UNSET:
		case ZEND_FETCH_DIM_FUNC_ARG:
		case ZEND_FETCH_LIST:
			if (ssa_ops[i].op1_def >= 0) {
				tmp = t1 & ~(MAY_BE_RC1|MAY_BE_RCN);
				if (opline->opcode == ZEND_FETCH_DIM_W ||
				    opline->opcode == ZEND_FETCH_DIM_RW ||
				    opline->opcode == ZEND_FETCH_DIM_FUNC_ARG) {
					if (t1 & (MAY_BE_UNDEF|MAY_BE_NULL|MAY_BE_FALSE)) {
						if (opline->opcode != ZEND_FETCH_DIM_FUNC_ARG) {
							tmp &= ~(MAY_BE_UNDEF|MAY_BE_NULL|MAY_BE_FALSE);
						}
						tmp |= MAY_BE_ARRAY | MAY_BE_RC1;
					}
					if (t1 & (MAY_BE_STRING|MAY_BE_ARRAY)) {
						tmp |= MAY_BE_RC1;
					}
					if (t1 & (MAY_BE_OBJECT|MAY_BE_RESOURCE)) {
						tmp |= t1 & (MAY_BE_RC1|MAY_BE_RCN);
					}
					if (opline->op2_type == IS_UNUSED) {
						tmp |= MAY_BE_ARRAY_KEY_LONG;
					} else {
						if (t2 & (MAY_BE_LONG|MAY_BE_FALSE|MAY_BE_TRUE|MAY_BE_RESOURCE|MAY_BE_DOUBLE)) {
							tmp |= MAY_BE_ARRAY_KEY_LONG;
						}
						if (t2 & MAY_BE_STRING) {
							tmp |= MAY_BE_ARRAY_KEY_STRING;
							if (opline->op2_type != IS_CONST) {
								// FIXME: numeric string
								tmp |= MAY_BE_ARRAY_KEY_LONG;
							}
						}
						if (t2 & (MAY_BE_UNDEF | MAY_BE_NULL)) {
							tmp |= MAY_BE_ARRAY_KEY_STRING;
						}
					}
				}
				j = ssa_vars[ssa_ops[i].result_def].use_chain;
				while (j >= 0) {
					switch (op_array->opcodes[j].opcode) {
						case ZEND_FETCH_DIM_W:
						case ZEND_FETCH_DIM_RW:
						case ZEND_FETCH_DIM_FUNC_ARG:
						case ZEND_ASSIGN_ADD:
						case ZEND_ASSIGN_SUB:
						case ZEND_ASSIGN_MUL:
						case ZEND_ASSIGN_DIV:
						case ZEND_ASSIGN_MOD:
						case ZEND_ASSIGN_SL:
						case ZEND_ASSIGN_SR:
						case ZEND_ASSIGN_CONCAT:
						case ZEND_ASSIGN_BW_OR:
						case ZEND_ASSIGN_BW_AND:
						case ZEND_ASSIGN_BW_XOR:
						case ZEND_ASSIGN_POW:
						case ZEND_ASSIGN_DIM:
							tmp |= MAY_BE_ARRAY | MAY_BE_ARRAY_OF_ARRAY;
							break;
						case ZEND_FETCH_OBJ_W:
						case ZEND_FETCH_OBJ_RW:
						case ZEND_FETCH_OBJ_FUNC_ARG:
						case ZEND_ASSIGN_OBJ:
						case ZEND_PRE_INC_OBJ:
						case ZEND_PRE_DEC_OBJ:
						case ZEND_POST_INC_OBJ:
						case ZEND_POST_DEC_OBJ:
							tmp |= MAY_BE_ARRAY_OF_OBJECT;
							break;
						case ZEND_SEND_VAR_EX:
						case ZEND_SEND_VAR_NO_REF:
						case ZEND_SEND_VAR_NO_REF_EX:
						case ZEND_SEND_REF:
						case ZEND_ASSIGN_REF:
						case ZEND_YIELD:
						case ZEND_INIT_ARRAY:
						case ZEND_ADD_ARRAY_ELEMENT:
						case ZEND_RETURN_BY_REF:
						case ZEND_VERIFY_RETURN_TYPE:
						case ZEND_MAKE_REF:
							tmp |= MAY_BE_ARRAY_OF_ANY | MAY_BE_ARRAY_OF_REF;
							break;
						case ZEND_PRE_INC:
						case ZEND_PRE_DEC:
						case ZEND_POST_INC:
						case ZEND_POST_DEC:
							if (tmp & MAY_BE_ARRAY_OF_LONG) {
								/* may overflow */
								tmp |= MAY_BE_ARRAY_OF_DOUBLE;
							} else if (!(tmp & (MAY_BE_ARRAY_OF_LONG|MAY_BE_ARRAY_OF_DOUBLE))) {
								tmp |= MAY_BE_ARRAY_OF_LONG | MAY_BE_ARRAY_OF_DOUBLE;
							}
							break;
						case ZEND_UNSET_DIM:
						case ZEND_UNSET_OBJ:
						case ZEND_FETCH_DIM_UNSET:
						case ZEND_FETCH_OBJ_UNSET:
							break;
						default	:
							break;
					}
					j = zend_ssa_next_use(ssa_ops, ssa_ops[i].result_def, j);
				}
				UPDATE_SSA_TYPE(tmp, ssa_ops[i].op1_def);
				COPY_SSA_OBJ_TYPE(ssa_ops[i].op1_use, ssa_ops[i].op1_def);
			}
			/* FETCH_LIST on a string behaves like FETCH_R on null */
			tmp = zend_array_element_type(
				opline->opcode != ZEND_FETCH_LIST ? t1 : ((t1 & ~MAY_BE_STRING) | MAY_BE_NULL),
				opline->opcode != ZEND_FETCH_DIM_R && opline->opcode != ZEND_FETCH_DIM_IS
					&& opline->opcode != ZEND_FETCH_LIST,
				opline->op2_type == IS_UNUSED);
			if (opline->opcode == ZEND_FETCH_DIM_W ||
			    opline->opcode == ZEND_FETCH_DIM_RW ||
			    opline->opcode == ZEND_FETCH_DIM_FUNC_ARG) {
				if (t1 & (MAY_BE_ERROR|MAY_BE_TRUE|MAY_BE_LONG|MAY_BE_DOUBLE|MAY_BE_RESOURCE|MAY_BE_OBJECT)) {
					tmp |= MAY_BE_ERROR;
				} else if (opline->op2_type == IS_UNUSED) {
					tmp |= MAY_BE_ERROR;
				} else if (t2 & (MAY_BE_ARRAY|MAY_BE_OBJECT)) {
					tmp |= MAY_BE_ERROR;
				}
			} else if (opline->opcode == ZEND_FETCH_DIM_IS && (t1 & MAY_BE_STRING)) {
				tmp |= MAY_BE_NULL;
			}
			UPDATE_SSA_TYPE(tmp, ssa_ops[i].result_def);
			break;
		case ZEND_FETCH_THIS:
			UPDATE_SSA_OBJ_TYPE(op_array->scope, 1, ssa_ops[i].result_def);
			UPDATE_SSA_TYPE(MAY_BE_RC1|MAY_BE_RCN|MAY_BE_OBJECT, ssa_ops[i].result_def);
			break;
		case ZEND_FETCH_OBJ_R:
		case ZEND_FETCH_OBJ_IS:
		case ZEND_FETCH_OBJ_RW:
		case ZEND_FETCH_OBJ_W:
		case ZEND_FETCH_OBJ_UNSET:
		case ZEND_FETCH_OBJ_FUNC_ARG:
			if (ssa_ops[i].op1_def >= 0) {
				tmp = t1;
				if (opline->opcode == ZEND_FETCH_OBJ_W ||
				    opline->opcode == ZEND_FETCH_OBJ_RW ||
				    opline->opcode == ZEND_FETCH_OBJ_FUNC_ARG) {
					if (opline->opcode != ZEND_FETCH_DIM_FUNC_ARG) {
						if (t1 & (MAY_BE_UNDEF|MAY_BE_NULL|MAY_BE_FALSE)) {
							tmp &= ~(MAY_BE_UNDEF|MAY_BE_NULL|MAY_BE_FALSE);
							tmp |= MAY_BE_OBJECT | MAY_BE_RC1 | MAY_BE_RCN;
						}
					}
				}
				UPDATE_SSA_TYPE(tmp, ssa_ops[i].op1_def);
				COPY_SSA_OBJ_TYPE(ssa_ops[i].op1_use, ssa_ops[i].op1_def);
			}
			if (ssa_ops[i].result_def >= 0) {
				tmp = MAY_BE_ANY | MAY_BE_ARRAY_KEY_ANY | MAY_BE_ARRAY_OF_ANY | MAY_BE_ARRAY_OF_REF;
				if (opline->opcode != ZEND_FETCH_OBJ_R && opline->opcode != ZEND_FETCH_OBJ_IS) {
					tmp |= MAY_BE_ERROR;
				}
				if (opline->result_type == IS_TMP_VAR) {
					tmp |= MAY_BE_RC1 | MAY_BE_RCN;
				} else {
					tmp |= MAY_BE_REF | MAY_BE_RC1 | MAY_BE_RCN;
				}
				UPDATE_SSA_TYPE(tmp, ssa_ops[i].result_def);
			}
			break;
		case ZEND_DO_FCALL:
		case ZEND_DO_ICALL:
		case ZEND_DO_UCALL:
		case ZEND_DO_FCALL_BY_NAME:
			if (ssa_ops[i].result_def >= 0) {
				zend_func_info *func_info = ZEND_FUNC_INFO(op_array);
				zend_call_info *call_info;

				if (!func_info || !func_info->call_map) {
					goto unknown_opcode;
				}
				call_info = func_info->call_map[opline - op_array->opcodes];
				if (!call_info) {
					goto unknown_opcode;
				}
				tmp = zend_get_func_info(call_info, ssa) & ~FUNC_MAY_WARN;
				UPDATE_SSA_TYPE(tmp, ssa_ops[i].result_def);
				if (call_info->callee_func->type == ZEND_USER_FUNCTION) {
					func_info = ZEND_FUNC_INFO(&call_info->callee_func->op_array);
					if (func_info) {
						UPDATE_SSA_OBJ_TYPE(
							func_info->return_info.ce,
							func_info->return_info.is_instanceof,
							ssa_ops[i].result_def);
					}
				}
			}
			break;
		case ZEND_FETCH_CONSTANT:
		case ZEND_FETCH_CLASS_CONSTANT:
			UPDATE_SSA_TYPE(MAY_BE_RC1|MAY_BE_RCN|MAY_BE_NULL|MAY_BE_FALSE|MAY_BE_TRUE|MAY_BE_LONG|MAY_BE_DOUBLE|MAY_BE_STRING|MAY_BE_RESOURCE|MAY_BE_ARRAY|MAY_BE_ARRAY_KEY_ANY|MAY_BE_ARRAY_OF_ANY, ssa_ops[i].result_def);
			break;
		case ZEND_STRLEN:
			tmp = MAY_BE_LONG;
			if (t1 & (MAY_BE_ANY - (MAY_BE_NULL|MAY_BE_FALSE|MAY_BE_TRUE|MAY_BE_LONG|MAY_BE_DOUBLE|MAY_BE_STRING))) {
				tmp |= MAY_BE_NULL;
			}
			UPDATE_SSA_TYPE(tmp, ssa_ops[i].result_def);
			break;
		case ZEND_COUNT:
		case ZEND_FUNC_NUM_ARGS:
			UPDATE_SSA_TYPE(MAY_BE_LONG, ssa_ops[i].result_def);
			break;
		case ZEND_FUNC_GET_ARGS:
			UPDATE_SSA_TYPE(MAY_BE_ARRAY | MAY_BE_ARRAY_KEY_LONG | MAY_BE_ARRAY_OF_ANY, ssa_ops[i].result_def);
			break;
		case ZEND_GET_CLASS:
		case ZEND_GET_CALLED_CLASS:
			UPDATE_SSA_TYPE(MAY_BE_FALSE|MAY_BE_STRING, ssa_ops[i].result_def);
			break;
		case ZEND_GET_TYPE:
			UPDATE_SSA_TYPE(MAY_BE_STRING, ssa_ops[i].result_def);
			break;
		case ZEND_TYPE_CHECK:
		case ZEND_DEFINED:
			UPDATE_SSA_TYPE(MAY_BE_FALSE|MAY_BE_TRUE, ssa_ops[i].result_def);
			break;
		case ZEND_VERIFY_RETURN_TYPE:
			if (t1 & MAY_BE_REF) {
				tmp = t1;
				ce = NULL;
			} else {
				zend_arg_info *ret_info = op_array->arg_info - 1;

				tmp = zend_fetch_arg_info(script, ret_info, &ce);
				if (tmp & (MAY_BE_STRING|MAY_BE_ARRAY|MAY_BE_OBJECT|MAY_BE_RESOURCE)) {
					tmp |= MAY_BE_RC1 | MAY_BE_RCN;
				}
			}
			if (opline->op1_type & (IS_TMP_VAR|IS_VAR|IS_CV)) {
				UPDATE_SSA_TYPE(tmp, ssa_ops[i].op1_def);
				if (ce) {
					UPDATE_SSA_OBJ_TYPE(ce, 1, ssa_ops[i].op1_def);
				} else {
					UPDATE_SSA_OBJ_TYPE(NULL, 0, ssa_ops[i].op1_def);
				}
			} else {
				UPDATE_SSA_TYPE(tmp, ssa_ops[i].result_def);
				if (ce) {
					UPDATE_SSA_OBJ_TYPE(ce, 1, ssa_ops[i].result_def);
				} else {
					UPDATE_SSA_OBJ_TYPE(NULL, 0, ssa_ops[i].result_def);
				}
			}
			break;
		case ZEND_CATCH:
		case ZEND_INCLUDE_OR_EVAL:
			/* Forbidden opcodes */
			ZEND_ASSERT(0);
			break;
		default:
unknown_opcode:
			if (ssa_ops[i].op1_def >= 0) {
				tmp = MAY_BE_ANY | MAY_BE_REF | MAY_BE_RC1 | MAY_BE_RCN | MAY_BE_ARRAY_KEY_ANY | MAY_BE_ARRAY_OF_ANY | MAY_BE_ARRAY_OF_REF;
				UPDATE_SSA_TYPE(tmp, ssa_ops[i].op1_def);
			}
			if (ssa_ops[i].result_def >= 0) {
				tmp = MAY_BE_ANY | MAY_BE_ARRAY_KEY_ANY | MAY_BE_ARRAY_OF_ANY | MAY_BE_ARRAY_OF_REF;
				if (opline->result_type == IS_TMP_VAR) {
					tmp |= MAY_BE_RC1 | MAY_BE_RCN;
				} else {
					tmp |= MAY_BE_REF | MAY_BE_RC1 | MAY_BE_RCN;
				}
				UPDATE_SSA_TYPE(tmp, ssa_ops[i].result_def);
			}
			break;
	}

	return SUCCESS;
}

static uint32_t get_class_entry_rank(zend_class_entry *ce) {
	uint32_t rank = 0;
	while (ce->parent) {
		rank++;
		ce = ce->parent;
	}
	return rank;
}

/* Compute least common ancestor on class inheritance tree only */
static zend_class_entry *join_class_entries(
		zend_class_entry *ce1, zend_class_entry *ce2, int *is_instanceof) {
	uint32_t rank1, rank2;
	if (ce1 == ce2) {
		return ce1;
	}
	if (!ce1 || !ce2) {
		return NULL;
	}

	rank1 = get_class_entry_rank(ce1);
	rank2 = get_class_entry_rank(ce2);

	while (rank1 != rank2) {
		if (rank1 > rank2) {
			ce1 = ce1->parent;
			rank1--;
		} else {
			ce2 = ce2->parent;
			rank2--;
		}
	}

	while (ce1 != ce2) {
		ce1 = ce1->parent;
		ce2 = ce2->parent;
	}

	if (ce1) {
		*is_instanceof = 1;
	}
	return ce1;
}

int zend_infer_types_ex(const zend_op_array *op_array, const zend_script *script, zend_ssa *ssa, zend_bitset worklist)
{
	zend_basic_block *blocks = ssa->cfg.blocks;
	zend_ssa_var *ssa_vars = ssa->vars;
	zend_ssa_var_info *ssa_var_info = ssa->var_info;
	int ssa_vars_count = ssa->vars_count;
	int i, j;
	uint32_t tmp;

	WHILE_WORKLIST(worklist, zend_bitset_len(ssa_vars_count), j) {
		if (ssa_vars[j].definition_phi) {
			zend_ssa_phi *p = ssa_vars[j].definition_phi;
			if (p->pi >= 0) {
				zend_class_entry *ce = ssa_var_info[p->sources[0]].ce;
				int is_instanceof = ssa_var_info[p->sources[0]].is_instanceof;
				tmp = get_ssa_var_info(ssa, p->sources[0]);

				if (!p->has_range_constraint) {
					zend_ssa_type_constraint *constraint = &p->constraint.type;
					tmp &= constraint->type_mask;
					if ((tmp & MAY_BE_OBJECT) && constraint->ce && ce != constraint->ce) {
						if (!ce) {
							ce = constraint->ce;
							is_instanceof = 1;
						} else if (is_instanceof && instanceof_function(constraint->ce, ce)) {
							ce = constraint->ce;
						} else {
							/* Ignore the constraint (either ce instanceof constraint->ce or
							 * they are unrelated, as far as we can statically determine) */
						}
					}
				}

				UPDATE_SSA_TYPE(tmp, j);
				UPDATE_SSA_OBJ_TYPE(ce, is_instanceof, j);
			} else {
				int first = 1;
				int is_instanceof = 0;
				zend_class_entry *ce = NULL;

				tmp = 0;
				for (i = 0; i < blocks[p->block].predecessors_count; i++) {
					tmp |= get_ssa_var_info(ssa, p->sources[i]);
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
			}
		} else if (ssa_vars[j].definition >= 0) {
			i = ssa_vars[j].definition;
			if (zend_update_type_info(op_array, ssa, script, worklist, i) == FAILURE) {
				return FAILURE;
			}
		}
	} WHILE_WORKLIST_END();
	return SUCCESS;
}

static zend_bool is_narrowable_instr(zend_op *opline)  {
	return opline->opcode == ZEND_ADD || opline->opcode == ZEND_SUB
		|| opline->opcode == ZEND_MUL || opline->opcode == ZEND_DIV;
}

static zend_bool is_effective_op1_double_cast(zend_op *opline, zval *op2) {
	return (opline->opcode == ZEND_ADD && Z_LVAL_P(op2) == 0)
		|| (opline->opcode == ZEND_SUB && Z_LVAL_P(op2) == 0)
		|| (opline->opcode == ZEND_MUL && Z_LVAL_P(op2) == 1)
		|| (opline->opcode == ZEND_DIV && Z_LVAL_P(op2) == 1);
}
static zend_bool is_effective_op2_double_cast(zend_op *opline, zval *op1) {
	/* In PHP it holds that (double)(0-$int) is bitwise identical to 0.0-(double)$int,
	 * so allowing SUB here is fine. */
	return (opline->opcode == ZEND_ADD && Z_LVAL_P(op1) == 0)
		|| (opline->opcode == ZEND_SUB && Z_LVAL_P(op1) == 0)
		|| (opline->opcode == ZEND_MUL && Z_LVAL_P(op1) == 1);
}

/* This function recursively checks whether it's possible to convert an integer variable
 * initialization to a double initialization. The basic idea is that if the value is used
 * only in add/sub/mul/div ("narrowable" instructions) with a double result value, then it
 * will be cast to double at that point anyway, so we may as well do it earlier already.
 *
 * The tricky case are chains of operations, where it's not necessarily a given that converting
 * an integer to double before the chain of operations is the same as converting it after the
 * chain. What this function does is detect two cases where it is safe:
 *  * If the operations only involve constants, then we can simply verify that performing the
 *    calculation on integers and doubles yields the same value.
 *  * Even if one operand is not known, we may be able to determine that the operations with the
 *    integer replaced by a double only acts as an effective double cast on the unknown operand.
 *    E.g. 0+$i and 0.0+$i only differ by that cast. If then the consuming instruction of this
 *    result will perform a double cast anyway, the conversion is safe.
 *
 * The checks happens recursively, while keeping track of which variables are already visisted to
 * avoid infinite loops. An iterative, worklist driven approach would be possible, but the state
 * management more cumbersome to implement, so we don't bother for now.
 */
static zend_bool can_convert_to_double(
		const zend_op_array *op_array, zend_ssa *ssa, int var_num,
		zval *value, zend_bitset visited) {
	zend_ssa_var *var = &ssa->vars[var_num];
	zend_ssa_phi *phi;
	int use;
	uint32_t type;

	if (zend_bitset_in(visited, var_num)) {
		return 1;
	}
	zend_bitset_incl(visited, var_num);

	for (use = var->use_chain; use >= 0; use = zend_ssa_next_use(ssa->ops, var_num, use)) {
		zend_op *opline = &op_array->opcodes[use];
		zend_ssa_op *ssa_op = &ssa->ops[use];

		if (zend_ssa_is_no_val_use(opline, ssa_op, var_num)) {
			continue;
		}

		if (!is_narrowable_instr(opline)) {
			return 0;
		}

		/* Instruction always returns double, the conversion is certainly fine */
		type = ssa->var_info[ssa_op->result_def].type;
		if ((type & MAY_BE_ANY) == MAY_BE_DOUBLE) {
			continue;
		}

		/* UNDEF signals that the previous result is an effective double cast, this is only allowed
		 * if this instruction would have done the cast anyway (previous check). */
		if (Z_ISUNDEF_P(value)) {
			return 0;
		}

		/* Check that narrowing can actually be useful */
		if ((type & MAY_BE_ANY) & ~(MAY_BE_LONG|MAY_BE_DOUBLE)) {
			return 0;
		}

		{
			/* For calculation on original values */
			zval orig_op1, orig_op2, orig_result;
			/* For calculation with var_num cast to double */
			zval dval_op1, dval_op2, dval_result;

			ZVAL_UNDEF(&orig_op1);
			ZVAL_UNDEF(&dval_op1);
			if (ssa_op->op1_use == var_num) {
				ZVAL_COPY_VALUE(&orig_op1, value);
				ZVAL_DOUBLE(&dval_op1, (double) Z_LVAL_P(value));
			} else if (opline->op1_type == IS_CONST) {
				zval *zv = CRT_CONSTANT_EX(op_array, opline->op1, ssa->rt_constants);
				if (Z_TYPE_P(zv) == IS_LONG || Z_TYPE_P(zv) == IS_DOUBLE) {
					ZVAL_COPY_VALUE(&orig_op1, zv);
					ZVAL_COPY_VALUE(&dval_op1, zv);
				}
			}

			ZVAL_UNDEF(&orig_op2);
			ZVAL_UNDEF(&dval_op2);
			if (ssa_op->op2_use == var_num) {
				ZVAL_COPY_VALUE(&orig_op2, value);
				ZVAL_DOUBLE(&dval_op2, (double) Z_LVAL_P(value));
			} else if (opline->op2_type == IS_CONST) {
				zval *zv = CRT_CONSTANT_EX(op_array, opline->op2, ssa->rt_constants);
				if (Z_TYPE_P(zv) == IS_LONG || Z_TYPE_P(zv) == IS_DOUBLE) {
					ZVAL_COPY_VALUE(&orig_op2, zv);
					ZVAL_COPY_VALUE(&dval_op2, zv);
				}
			}

			ZEND_ASSERT(!Z_ISUNDEF(orig_op1) || !Z_ISUNDEF(orig_op2));
			if (Z_ISUNDEF(orig_op1)) {
				if (opline->opcode == ZEND_MUL && Z_LVAL(orig_op2) == 0) {
					ZVAL_LONG(&orig_result, 0);
				} else if (is_effective_op1_double_cast(opline, &orig_op2)) {
					ZVAL_UNDEF(&orig_result);
				} else {
					return 0;
				}
			} else if (Z_ISUNDEF(orig_op2)) {
				if (opline->opcode == ZEND_MUL && Z_LVAL(orig_op1) == 0) {
					ZVAL_LONG(&orig_result, 0);
				} else if (is_effective_op2_double_cast(opline, &orig_op1)) {
					ZVAL_UNDEF(&orig_result);
				} else {
					return 0;
				}
			} else {
				/* Avoid division by zero */
				if (opline->opcode == ZEND_DIV && zval_get_double(&orig_op2) == 0.0) {
					return 0;
				}

				get_binary_op(opline->opcode)(&orig_result, &orig_op1, &orig_op2);
				get_binary_op(opline->opcode)(&dval_result, &dval_op1, &dval_op2);
				ZEND_ASSERT(Z_TYPE(dval_result) == IS_DOUBLE);
				if (zval_get_double(&orig_result) != Z_DVAL(dval_result)) {
					return 0;
				}
			}

			if (!can_convert_to_double(op_array, ssa, ssa_op->result_def, &orig_result, visited)) {
				return 0;
			}
		}
	}

	for (phi = var->phi_use_chain; phi; phi = zend_ssa_next_use_phi(ssa, var_num, phi)) {
		/* Check that narrowing can actually be useful */
		type = ssa->var_info[phi->ssa_var].type;
		if ((type & MAY_BE_ANY) & ~(MAY_BE_LONG|MAY_BE_DOUBLE)) {
			return 0;
		}

		if (!can_convert_to_double(op_array, ssa, phi->ssa_var, value, visited)) {
			return 0;
		}
	}

	return 1;
}

static int zend_type_narrowing(const zend_op_array *op_array, const zend_script *script, zend_ssa *ssa)
{
	uint32_t bitset_len = zend_bitset_len(ssa->vars_count);
	zend_bitset visited, worklist;
	int i, v;
	zend_op *opline;
	zend_bool narrowed = 0;
	ALLOCA_FLAG(use_heap)

	visited = ZEND_BITSET_ALLOCA(2 * bitset_len, use_heap);
	worklist = visited + bitset_len;

	zend_bitset_clear(worklist, bitset_len);

	for (v = op_array->last_var; v < ssa->vars_count; v++) {
		if ((ssa->var_info[v].type & (MAY_BE_REF | MAY_BE_ANY | MAY_BE_UNDEF)) != MAY_BE_LONG) continue;
		if (ssa->vars[v].definition < 0) continue;
		if (ssa->vars[v].no_val) continue;
		opline = op_array->opcodes + ssa->vars[v].definition;
		/* Go through assignments of literal integers and check if they can be converted to
		 * doubles instead, in the hope that we'll narrow long|double to double. */
		if (opline->opcode == ZEND_ASSIGN && opline->result_type == IS_UNUSED &&
				opline->op1_type == IS_CV && opline->op2_type == IS_CONST) {
			zval *value = CRT_CONSTANT_EX(op_array, opline->op2, ssa->rt_constants);

			zend_bitset_clear(visited, bitset_len);
			if (can_convert_to_double(op_array, ssa, v, value, visited)) {
				narrowed = 1;
				ssa->var_info[v].use_as_double = 1;
				/* The "visited" vars are exactly those which may change their type due to
				 * narrowing. Reset their types and add them to the type inference worklist */
				ZEND_BITSET_FOREACH(visited, bitset_len, i) {
					ssa->var_info[i].type &= ~MAY_BE_ANY;
				} ZEND_BITSET_FOREACH_END();
				zend_bitset_union(worklist, visited, bitset_len);
			}
		}
	}

	if (!narrowed) {
		free_alloca(visited, use_heap);
		return SUCCESS;
	}

	if (zend_infer_types_ex(op_array, script, ssa, worklist) != SUCCESS) {
		free_alloca(visited, use_heap);
		return FAILURE;
	}

	free_alloca(visited, use_heap);
	return SUCCESS;
}

static int is_recursive_tail_call(const zend_op_array *op_array,
                                  zend_op             *opline)
{
	zend_func_info *info = ZEND_FUNC_INFO(op_array);

	if (info->ssa.ops && info->ssa.vars && info->call_map &&
	    info->ssa.ops[opline - op_array->opcodes].op1_use >= 0 &&
	    info->ssa.vars[info->ssa.ops[opline - op_array->opcodes].op1_use].definition >= 0) {

		zend_op *op = op_array->opcodes + info->ssa.vars[info->ssa.ops[opline - op_array->opcodes].op1_use].definition;

		if (op->opcode == ZEND_DO_UCALL) {
			zend_call_info *call_info = info->call_map[op - op_array->opcodes];
			if (call_info && op_array == &call_info->callee_func->op_array) {
				return 1;
			}
		}
	}
	return 0;
}

void zend_init_func_return_info(const zend_op_array   *op_array,
                                const zend_script     *script,
                                zend_ssa_var_info     *ret)
{
	if (op_array->fn_flags & ZEND_ACC_HAS_RETURN_TYPE) {
		zend_arg_info *ret_info = op_array->arg_info - 1;
		zend_ssa_range tmp_range = {0, 0, 0, 0};

		ret->type = zend_fetch_arg_info(script, ret_info, &ret->ce);
		if (op_array->fn_flags & ZEND_ACC_RETURN_REFERENCE) {
			ret->type |= MAY_BE_REF;
		}
		ret->is_instanceof = (ret->ce) ? 1 : 0;
		ret->range = tmp_range;
		ret->has_range = 0;
	}
}

void zend_func_return_info(const zend_op_array   *op_array,
                           const zend_script     *script,
                           int                    recursive,
                           int                    widening,
                           zend_ssa_var_info     *ret)
{
	zend_func_info *info = ZEND_FUNC_INFO(op_array);
	zend_ssa *ssa = &info->ssa;
	int blocks_count = info->ssa.cfg.blocks_count;
	zend_basic_block *blocks = info->ssa.cfg.blocks;
	int j;
	uint32_t t1;
	uint32_t tmp = 0;
	zend_class_entry *tmp_ce = NULL;
	int tmp_is_instanceof = -1;
	zend_class_entry *arg_ce;
	int arg_is_instanceof;
	zend_ssa_range tmp_range = {0, 0, 0, 0};
	int tmp_has_range = -1;

	if (op_array->fn_flags & ZEND_ACC_GENERATOR) {
		ret->type = MAY_BE_OBJECT | MAY_BE_RC1 | MAY_BE_RCN;
		ret->ce = zend_ce_generator;
		ret->is_instanceof = 0;
		ret->range = tmp_range;
		ret->has_range = 0;
		return;
	}

	for (j = 0; j < blocks_count; j++) {
		if ((blocks[j].flags & ZEND_BB_REACHABLE) && blocks[j].len != 0) {
			zend_op *opline = op_array->opcodes + blocks[j].start + blocks[j].len - 1;

			if (opline->opcode == ZEND_RETURN || opline->opcode == ZEND_RETURN_BY_REF) {
				if (!recursive &&
				    info->ssa.ops &&
				    info->ssa.var_info &&
				    info->ssa.ops[opline - op_array->opcodes].op1_use >= 0 &&
				    info->ssa.var_info[info->ssa.ops[opline - op_array->opcodes].op1_use].recursive) {
					continue;
				}
				if (is_recursive_tail_call(op_array, opline)) {
					continue;
				}
				t1 = OP1_INFO();
				if (t1 & MAY_BE_UNDEF) {
					t1 |= MAY_BE_NULL;
				}
				if (opline->opcode == ZEND_RETURN) {
					if (t1 & MAY_BE_RC1) {
						t1 |= MAY_BE_RCN;
					}
					t1 &= ~(MAY_BE_UNDEF | MAY_BE_REF);
				} else {
					t1 |= MAY_BE_REF;
					t1 &= ~(MAY_BE_UNDEF | MAY_BE_RC1 | MAY_BE_RCN);
				}
				tmp |= t1;

				if (info->ssa.ops &&
				    info->ssa.var_info &&
				    info->ssa.ops[opline - op_array->opcodes].op1_use >= 0 &&
				    info->ssa.var_info[info->ssa.ops[opline - op_array->opcodes].op1_use].ce) {
					arg_ce = info->ssa.var_info[info->ssa.ops[opline - op_array->opcodes].op1_use].ce;
					arg_is_instanceof = info->ssa.var_info[info->ssa.ops[opline - op_array->opcodes].op1_use].is_instanceof;
				} else {
					arg_ce = NULL;
					arg_is_instanceof = 0;
				}

				if (tmp_is_instanceof < 0) {
					tmp_ce = arg_ce;
					tmp_is_instanceof = arg_is_instanceof;
				} else if (arg_ce && arg_ce == tmp_ce) {
					if (tmp_is_instanceof != arg_is_instanceof) {
						tmp_is_instanceof = 1;
					}
				} else {
					tmp_ce = NULL;
					tmp_is_instanceof = 0;
				}

				if (opline->op1_type == IS_CONST) {
					zval *zv = CRT_CONSTANT_EX(op_array, opline->op1, info->ssa.rt_constants);

					if (Z_TYPE_P(zv) == IS_NULL) {
						if (tmp_has_range < 0) {
							tmp_has_range = 1;
							tmp_range.underflow = 0;
							tmp_range.min = 0;
							tmp_range.max = 0;
							tmp_range.overflow = 0;
						} else if (tmp_has_range) {
							if (!tmp_range.underflow) {
								tmp_range.min = MIN(tmp_range.min, 0);
							}
							if (!tmp_range.overflow) {
								tmp_range.max = MAX(tmp_range.max, 0);
							}
						}
					} else if (Z_TYPE_P(zv) == IS_FALSE) {
						if (tmp_has_range < 0) {
							tmp_has_range = 1;
							tmp_range.underflow = 0;
							tmp_range.min = 0;
							tmp_range.max = 0;
							tmp_range.overflow = 0;
						} else if (tmp_has_range) {
							if (!tmp_range.underflow) {
								tmp_range.min = MIN(tmp_range.min, 0);
							}
							if (!tmp_range.overflow) {
								tmp_range.max = MAX(tmp_range.max, 0);
							}
						}
					} else if (Z_TYPE_P(zv) == IS_TRUE) {
						if (tmp_has_range < 0) {
							tmp_has_range = 1;
							tmp_range.underflow = 0;
							tmp_range.min = 1;
							tmp_range.max = 1;
							tmp_range.overflow = 0;
						} else if (tmp_has_range) {
							if (!tmp_range.underflow) {
								tmp_range.min = MIN(tmp_range.min, 1);
							}
							if (!tmp_range.overflow) {
								tmp_range.max = MAX(tmp_range.max, 1);
							}
						}
					} else if (Z_TYPE_P(zv) == IS_LONG) {
						if (tmp_has_range < 0) {
							tmp_has_range = 1;
							tmp_range.underflow = 0;
							tmp_range.min = Z_LVAL_P(zv);
							tmp_range.max = Z_LVAL_P(zv);
							tmp_range.overflow = 0;
						} else if (tmp_has_range) {
							if (!tmp_range.underflow) {
								tmp_range.min = MIN(tmp_range.min, Z_LVAL_P(zv));
							}
							if (!tmp_range.overflow) {
								tmp_range.max = MAX(tmp_range.max, Z_LVAL_P(zv));
							}
						}
					} else {
						tmp_has_range = 0;
					}
				} else if (info->ssa.ops &&
				           info->ssa.var_info &&
				           info->ssa.ops[opline - op_array->opcodes].op1_use >= 0) {
					if (info->ssa.var_info[info->ssa.ops[opline - op_array->opcodes].op1_use].has_range) {
						if (tmp_has_range < 0) {
							tmp_has_range = 1;
							tmp_range = info->ssa.var_info[info->ssa.ops[opline - op_array->opcodes].op1_use].range;
						} else if (tmp_has_range) {
							/* union */
							if (info->ssa.var_info[info->ssa.ops[opline - op_array->opcodes].op1_use].range.underflow) {
								tmp_range.underflow = 1;
								tmp_range.min = ZEND_LONG_MIN;
							} else {
								tmp_range.min = MIN(tmp_range.min, info->ssa.var_info[info->ssa.ops[opline - op_array->opcodes].op1_use].range.min);
							}
							if (info->ssa.var_info[info->ssa.ops[opline - op_array->opcodes].op1_use].range.overflow) {
								tmp_range.overflow = 1;
								tmp_range.max = ZEND_LONG_MAX;
							} else {
								tmp_range.max = MAX(tmp_range.max, info->ssa.var_info[info->ssa.ops[opline - op_array->opcodes].op1_use].range.max);
							}
						}
					} else if (!widening) {
						tmp_has_range = 1;
						tmp_range.underflow = 1;
						tmp_range.min = ZEND_LONG_MIN;
						tmp_range.max = ZEND_LONG_MAX;
						tmp_range.overflow = 1;
					}
				} else {
					tmp_has_range = 0;
				}
			}
		}
	}

	if (!(op_array->fn_flags & ZEND_ACC_HAS_RETURN_TYPE)) {
		if (tmp_is_instanceof < 0) {
			tmp_is_instanceof = 0;
			tmp_ce = NULL;
		}
		if (tmp_has_range < 0) {
			tmp_has_range = 0;
		}
		ret->type = tmp;
		ret->ce = tmp_ce;
		ret->is_instanceof = tmp_is_instanceof;
	}
	ret->range = tmp_range;
	ret->has_range = tmp_has_range;
}

static int zend_infer_types(const zend_op_array *op_array, const zend_script *script, zend_ssa *ssa)
{
	zend_ssa_var_info *ssa_var_info = ssa->var_info;
	int ssa_vars_count = ssa->vars_count;
	int j;
	zend_bitset worklist;
	ALLOCA_FLAG(use_heap);

	worklist = do_alloca(sizeof(zend_ulong) * zend_bitset_len(ssa_vars_count), use_heap);
	memset(worklist, 0, sizeof(zend_ulong) * zend_bitset_len(ssa_vars_count));

	/* Type Inference */
	for (j = op_array->last_var; j < ssa_vars_count; j++) {
		zend_bitset_incl(worklist, j);
		ssa_var_info[j].type = 0;
	}

	if (zend_infer_types_ex(op_array, script, ssa, worklist) != SUCCESS) {
		free_alloca(worklist,  use_heap);
		return FAILURE;
	}

	/* Narrowing integer initialization to doubles */
	zend_type_narrowing(op_array, script, ssa);

	for (j = 0; j < ssa_vars_count; j++) {
		if (ssa->vars[j].alias) {
			if (ssa->vars[j].alias == PHP_ERRORMSG_ALIAS) {
				ssa_var_info[j].type |= MAY_BE_STRING | MAY_BE_RC1 | MAY_BE_RCN;
			} else if (ssa->vars[j].alias == PHP_ERRORMSG_ALIAS) {
				ssa_var_info[j].type |= MAY_BE_ARRAY | MAY_BE_ARRAY_KEY_LONG | MAY_BE_ARRAY_OF_STRING | MAY_BE_RC1 | MAY_BE_RCN;
			} else {
				ssa_var_info[j].type = MAY_BE_UNDEF | MAY_BE_RC1 | MAY_BE_RCN | MAY_BE_REF | MAY_BE_ANY | MAY_BE_ARRAY_KEY_ANY | MAY_BE_ARRAY_OF_ANY | MAY_BE_ARRAY_OF_REF;
			}
		}
	}

	if (ZEND_FUNC_INFO(op_array)) {
		zend_func_return_info(op_array, script, 1, 0, &ZEND_FUNC_INFO(op_array)->return_info);
	}

	free_alloca(worklist,  use_heap);
	return SUCCESS;
}

int zend_ssa_inference(zend_arena **arena, const zend_op_array *op_array, const zend_script *script, zend_ssa *ssa) /* {{{ */
{
	zend_ssa_var_info *ssa_var_info;
	int i;

	if (!ssa->var_info) {
		ssa->var_info = zend_arena_calloc(arena, ssa->vars_count, sizeof(zend_ssa_var_info));
	}
	ssa_var_info = ssa->var_info;

	if (!op_array->function_name) {
		for (i = 0; i < op_array->last_var; i++) {
			ssa_var_info[i].type = MAY_BE_UNDEF | MAY_BE_RC1 | MAY_BE_RCN | MAY_BE_REF | MAY_BE_ANY  | MAY_BE_ARRAY_KEY_ANY | MAY_BE_ARRAY_OF_ANY | MAY_BE_ARRAY_OF_REF;
			ssa_var_info[i].has_range = 0;
		}
	} else {
		for (i = 0; i < op_array->last_var; i++) {
			ssa_var_info[i].type = MAY_BE_UNDEF;
			ssa_var_info[i].has_range = 0;
		}
	}
	for (i = op_array->last_var; i < ssa->vars_count; i++) {
		ssa_var_info[i].type = 0;
		ssa_var_info[i].has_range = 0;
	}

	if (zend_infer_ranges(op_array, ssa) != SUCCESS) {
		return FAILURE;
	}

	if (zend_infer_types(op_array, script, ssa) != SUCCESS) {
		return FAILURE;
	}

	return SUCCESS;
}
/* }}} */

void zend_inference_check_recursive_dependencies(zend_op_array *op_array)
{
	zend_func_info *info = ZEND_FUNC_INFO(op_array);
	zend_call_info *call_info;
	zend_bitset worklist;
	int worklist_len, i;
	ALLOCA_FLAG(use_heap);

	if (!info->ssa.var_info || !(info->flags & ZEND_FUNC_RECURSIVE)) {
		return;
	}
	worklist_len = zend_bitset_len(info->ssa.vars_count);
	worklist = do_alloca(sizeof(zend_ulong) * worklist_len, use_heap);
	memset(worklist, 0, sizeof(zend_ulong) * worklist_len);
	call_info = info->callee_info;
	while (call_info) {
		if (call_info->recursive &&
		    info->ssa.ops[call_info->caller_call_opline - op_array->opcodes].result_def >= 0) {
			zend_bitset_incl(worklist, info->ssa.ops[call_info->caller_call_opline - op_array->opcodes].result_def);
		}
		call_info = call_info->next_callee;
	}
	WHILE_WORKLIST(worklist, worklist_len, i) {
		if (!info->ssa.var_info[i].recursive) {
			info->ssa.var_info[i].recursive = 1;
			add_usages(op_array, &info->ssa, worklist, i);
		}
	} WHILE_WORKLIST_END();
	free_alloca(worklist, use_heap);
}

/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * indent-tabs-mode: t
 * End:
 */
				for (i = 0; i < blocks[p->block].predecessors_count; i++) {
					tmp |= get_ssa_var_info(ssa, p->sources[i]);
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
			}
		if (!info->ssa.var_info[i].recursive) {
			info->ssa.var_info[i].recursive = 1;
			add_usages(op_array, &info->ssa, worklist, i);
		}
	} WHILE_WORKLIST_END();
	free_alloca(worklist, use_heap);
}

/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * indent-tabs-mode: t
 * End:
 */
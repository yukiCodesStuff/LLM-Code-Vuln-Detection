				           !op_array->function_name) {
					flags |= ZEND_FUNC_INDIRECT_VAR_ACCESS;
				}
				break;
		}
	}

	/* If the entry block has predecessors, we may need to split it */
	if ((build_flags & ZEND_CFG_NO_ENTRY_PREDECESSORS)
			&& op_array->last > 0 && block_map[0] > 1) {
		extra_entry_block = 1;
	}

	if (cfg->split_at_live_ranges) {
		for (j = 0; j < op_array->last_live_range; j++) {
			BB_START(op_array->live_range[j].start);
			BB_START(op_array->live_range[j].end);
		}
	}

	if (op_array->last_try_catch) {
		for (j = 0; j < op_array->last_try_catch; j++) {
			BB_START(op_array->try_catch_array[j].try_op);
			if (op_array->try_catch_array[j].catch_op) {
				BB_START(op_array->try_catch_array[j].catch_op);
			}
			if (op_array->try_catch_array[j].finally_op) {
				BB_START(op_array->try_catch_array[j].finally_op);
			}
			if (op_array->try_catch_array[j].finally_end) {
				BB_START(op_array->try_catch_array[j].finally_end);
			}
		}
	}

	blocks_count += extra_entry_block;
	cfg->blocks_count = blocks_count;

	/* Build CFG, Step 2: Build Array of Basic Blocks */
	cfg->blocks = blocks = zend_arena_calloc(arena, sizeof(zend_basic_block), blocks_count);

	blocks_count = -1;

	if (extra_entry_block) {
		initialize_block(&blocks[0]);
		blocks[0].start = 0;
		blocks[0].len = 0;
		blocks_count++;
	}

	for (i = 0; i < op_array->last; i++) {
		if (block_map[i]) {
			if (blocks_count >= 0) {
				blocks[blocks_count].len = i - blocks[blocks_count].start;
			}
			blocks_count++;
			initialize_block(&blocks[blocks_count]);
			blocks[blocks_count].start = i;
		}
		block_map[i] = blocks_count;
	}

	blocks[blocks_count].len = i - blocks[blocks_count].start;
	blocks_count++;

	/* Build CFG, Step 3: Calculate successors */
	for (j = 0; j < blocks_count; j++) {
		zend_basic_block *block = &blocks[j];
		zend_op *opline;
		if (block->len == 0) {
			block->successors_count = 1;
			block->successors[0] = j + 1;
			continue;
		}

		opline = op_array->opcodes + block->start + block->len - 1;
		switch (opline->opcode) {
			case ZEND_FAST_RET:
			case ZEND_RETURN:
			case ZEND_RETURN_BY_REF:
			case ZEND_GENERATOR_RETURN:
			case ZEND_EXIT:
			case ZEND_THROW:
				break;
			case ZEND_JMP:
				block->successors_count = 1;
				block->successors[0] = block_map[OP_JMP_ADDR(opline, opline->op1) - op_array->opcodes];
				break;
			case ZEND_JMPZNZ:
				block->successors_count = 2;
				block->successors[0] = block_map[OP_JMP_ADDR(opline, opline->op2) - op_array->opcodes];
				block->successors[1] = block_map[ZEND_OFFSET_TO_OPLINE_NUM(op_array, opline, opline->extended_value)];
				break;
			case ZEND_JMPZ:
			case ZEND_JMPNZ:
			case ZEND_JMPZ_EX:
			case ZEND_JMPNZ_EX:
			case ZEND_JMP_SET:
			case ZEND_COALESCE:
			case ZEND_ASSERT_CHECK:
				block->successors_count = 2;
				block->successors[0] = block_map[OP_JMP_ADDR(opline, opline->op2) - op_array->opcodes];
				block->successors[1] = j + 1;
				break;
			case ZEND_CATCH:
				if (!opline->result.num) {
					block->successors_count = 2;
					block->successors[0] = block_map[ZEND_OFFSET_TO_OPLINE_NUM(op_array, opline, opline->extended_value)];
					block->successors[1] = j + 1;
				} else {
					block->successors_count = 1;
					block->successors[0] = j + 1;
				}
				ZEND_HASH_FOREACH_VAL(jumptable, zv) {
					block->successors[s++] = block_map[ZEND_OFFSET_TO_OPLINE_NUM(op_array, opline, Z_LVAL_P(zv))];
				} ZEND_HASH_FOREACH_END();

				block->successors[s++] = block_map[ZEND_OFFSET_TO_OPLINE_NUM(op_array, opline, opline->extended_value)];
				block->successors[s++] = j + 1;
				break;
			}
			default:
				block->successors_count = 1;
				block->successors[0] = j + 1;
				break;
		}
	}

	/* Build CFG, Step 4, Mark Reachable Basic Blocks */
	zend_mark_reachable_blocks(op_array, cfg, 0);

	cfg->dynamic = (flags & ZEND_FUNC_INDIRECT_VAR_ACCESS);

	if (func_flags) {
		*func_flags |= flags;
	}
		if (blocks[j].flags & ZEND_BB_REACHABLE) {
			for (s = 0; s < blocks[j].successors_count; s++) {
				zend_basic_block *b = blocks + blocks[j].successors[s];
				predecessors[b->predecessor_offset + b->predecessors_count] = j;
				b->predecessors_count++;
			}
		}
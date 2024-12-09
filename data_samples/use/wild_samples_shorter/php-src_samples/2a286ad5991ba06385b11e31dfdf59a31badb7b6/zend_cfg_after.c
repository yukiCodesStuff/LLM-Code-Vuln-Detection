					flags |= ZEND_FUNC_INDIRECT_VAR_ACCESS;
				}
				break;
			case ZEND_FUNC_GET_ARGS:
				flags |= ZEND_FUNC_VARARG;
				break;
		}
	}

	/* If the entry block has predecessors, we may need to split it */
	/* Build CFG, Step 4, Mark Reachable Basic Blocks */
	zend_mark_reachable_blocks(op_array, cfg, 0);

	cfg->dynamic = (flags & ZEND_FUNC_INDIRECT_VAR_ACCESS) != 0;
	cfg->vararg = (flags & ZEND_FUNC_VARARG) != 0;

	if (func_flags) {
		*func_flags |= flags;
	}
	for (j = 0; j < cfg->blocks_count; j++) {
		if (blocks[j].flags & ZEND_BB_REACHABLE) {
			for (s = 0; s < blocks[j].successors_count; s++) {
				/* SWITCH_STRING/LONG may have few following identical successors */
				if (s == 0 || blocks[j].successors[s-1] != blocks[j].successors[s]) {
					zend_basic_block *b = blocks + blocks[j].successors[s];

					predecessors[b->predecessor_offset + b->predecessors_count] = j;
					b->predecessors_count++;
				}
			}
		}
	}

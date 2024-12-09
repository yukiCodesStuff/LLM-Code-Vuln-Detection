	if (base > __pa(high_memory-1))
		return 0;

	id_sz = (__pa(high_memory-1) <= base + size) ?
				__pa(high_memory) - base :
				size;

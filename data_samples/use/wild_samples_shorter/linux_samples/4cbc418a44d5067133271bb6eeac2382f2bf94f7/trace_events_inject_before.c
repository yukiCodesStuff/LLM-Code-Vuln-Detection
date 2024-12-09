	unsigned long irq_flags;
	void *entry = NULL;
	int entry_size;
	u64 val;
	int len;

	entry = trace_alloc_entry(call, &entry_size);
	*pentry = entry;
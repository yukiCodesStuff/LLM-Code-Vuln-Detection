{
	unsigned long id_sz;

	if (base > __pa(high_memory-1))
		return 0;

	id_sz = (__pa(high_memory-1) <= base + size) ?
				__pa(high_memory) - base :
				size;

	if (ioremap_change_attr((unsigned long)__va(base), id_sz, flags) < 0) {
		printk(KERN_INFO "%s:%d ioremap_change_attr failed %s "
			"for [mem %#010Lx-%#010Lx]\n",
			current->comm, current->pid,
			cattr_name(flags),
			base, (unsigned long long)(base + size-1));
		return -EINVAL;
	}
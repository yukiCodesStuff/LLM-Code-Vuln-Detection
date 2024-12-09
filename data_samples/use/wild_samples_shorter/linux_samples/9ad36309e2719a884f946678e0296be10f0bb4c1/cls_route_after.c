	rcu_assign_pointer(f->next, f1);
	rcu_assign_pointer(*fp, f);

	if (fold) {
		th = to_hash(fold->handle);
		h = from_hash(fold->handle >> 16);
		b = rtnl_dereference(head->table[th]);
		if (b) {
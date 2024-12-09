{
	int i;

	if (!parent)
		return -EINVAL;

	for (i = 0; i < core->num_parents; i++) {
		if (core->parents[i] == parent)
			return i;

		if (core->parents[i])
			continue;

		/* Fallback to comparing globally unique names */
		if (!strcmp(parent->name, core->parent_names[i])) {
			core->parents[i] = parent;
			return i;
		}
	}
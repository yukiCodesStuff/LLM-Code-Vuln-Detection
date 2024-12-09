	if (!parent)
		return -EINVAL;

	for (i = 0; i < core->num_parents; i++)
		if (clk_core_get_parent_by_index(core, i) == parent)
			return i;

	return -EINVAL;
}

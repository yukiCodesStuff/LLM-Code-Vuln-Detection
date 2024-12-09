{
	const char *propname;
	int ct_idx;

	ct_idx = get_cacheinfo_idx(this_leaf->type);
	propname = cache_type_info[ct_idx].size_prop;

	of_property_read_u32(np, propname, &this_leaf->size);
}

/* not cache_line_size() because that's a macro in include/linux/cache.h */
static void cache_get_line_size(struct cacheinfo *this_leaf,
				struct device_node *np)
{
	int i, lim, ct_idx;

	ct_idx = get_cacheinfo_idx(this_leaf->type);
	lim = ARRAY_SIZE(cache_type_info[ct_idx].line_size_props);

	for (i = 0; i < lim; i++) {
		int ret;
		u32 line_size;
		const char *propname;

		propname = cache_type_info[ct_idx].line_size_props[i];
		ret = of_property_read_u32(np, propname, &line_size);
		if (!ret) {
			this_leaf->coherency_line_size = line_size;
			break;
		}
	}
{
	const char *propname;
	int ct_idx;

	ct_idx = get_cacheinfo_idx(this_leaf->type);
	propname = cache_type_info[ct_idx].nr_sets_prop;

	of_property_read_u32(np, propname, &this_leaf->number_of_sets);
}

static void cache_associativity(struct cacheinfo *this_leaf)
{
	unsigned int line_size = this_leaf->coherency_line_size;
	unsigned int nr_sets = this_leaf->number_of_sets;
	unsigned int size = this_leaf->size;

	/*
	 * If the cache is fully associative, there is no need to
	 * check the other properties.
	 */
	if (!(nr_sets == 1) && (nr_sets > 0 && size > 0 && line_size > 0))
		this_leaf->ways_of_associativity = (size / nr_sets) / line_size;
}

static bool cache_node_is_unified(struct cacheinfo *this_leaf,
				  struct device_node *np)
{
	return of_property_read_bool(np, "cache-unified");
}

static void cache_of_set_props(struct cacheinfo *this_leaf,
			       struct device_node *np)
{
	/*
	 * init_cache_level must setup the cache level correctly
	 * overriding the architecturally specified levels, so
	 * if type is NONE at this stage, it should be unified
	 */
	if (this_leaf->type == CACHE_TYPE_NOCACHE &&
	    cache_node_is_unified(this_leaf, np))
		this_leaf->type = CACHE_TYPE_UNIFIED;
	cache_size(this_leaf, np);
	cache_get_line_size(this_leaf, np);
	cache_nr_sets(this_leaf, np);
	cache_associativity(this_leaf);
}

static int cache_setup_of_node(unsigned int cpu)
{
	struct device_node *np;
	struct cacheinfo *this_leaf;
	struct device *cpu_dev = get_cpu_device(cpu);
	struct cpu_cacheinfo *this_cpu_ci = get_cpu_cacheinfo(cpu);
	unsigned int index = 0;

	/* skip if fw_token is already populated */
	if (this_cpu_ci->info_list->fw_token) {
		return 0;
	}
	ct_idx = get_cacheinfo_idx(this_leaf->type);
	propname = cache_type_info[ct_idx].size_prop;

	of_property_read_u32(np, propname, &this_leaf->size);
}

/* not cache_line_size() because that's a macro in include/linux/cache.h */
static void cache_get_line_size(struct cacheinfo *this_leaf,
	ct_idx = get_cacheinfo_idx(this_leaf->type);
	propname = cache_type_info[ct_idx].nr_sets_prop;

	of_property_read_u32(np, propname, &this_leaf->number_of_sets);
}

static void cache_associativity(struct cacheinfo *this_leaf)
{
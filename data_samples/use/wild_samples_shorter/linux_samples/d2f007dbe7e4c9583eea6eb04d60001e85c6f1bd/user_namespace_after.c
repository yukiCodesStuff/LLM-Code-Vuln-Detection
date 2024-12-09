	if (!new_idmap_permitted(file, ns, cap_setid, &new_map))
		goto out;

	ret = -EPERM;
	/* Map the lower ids from the parent user namespace to the
	 * kernel global id space.
	 */
		e->lower_first = lower_first;
	}

	/*
	 * If we want to use binary search for lookup, this clones the extent
	 * array and sorts both copies.
	 */
	ret = sort_idmaps(&new_map);
	if (ret < 0)
		goto out;

	/* Install the map */
	if (new_map.nr_extents <= UID_GID_MAP_MAX_BASE_EXTENTS) {
		memcpy(map->extent, new_map.extent,
		       new_map.nr_extents * sizeof(new_map.extent[0]));
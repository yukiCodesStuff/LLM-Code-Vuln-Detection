		if (next_line) {
			*next_line = '\0';
			next_line++;
			if (*next_line == '\0')
				next_line = NULL;
		}

		pos = skip_spaces(pos);
		extent.first = simple_strtoul(pos, &pos, 10);
		if (!isspace(*pos))
			goto out;

		pos = skip_spaces(pos);
		extent.lower_first = simple_strtoul(pos, &pos, 10);
		if (!isspace(*pos))
			goto out;

		pos = skip_spaces(pos);
		extent.count = simple_strtoul(pos, &pos, 10);
		if (*pos && !isspace(*pos))
			goto out;

		/* Verify there is not trailing junk on the line */
		pos = skip_spaces(pos);
		if (*pos != '\0')
			goto out;

		/* Verify we have been given valid starting values */
		if ((extent.first == (u32) -1) ||
		    (extent.lower_first == (u32) -1))
			goto out;

		/* Verify count is not zero and does not cause the
		 * extent to wrap
		 */
		if ((extent.first + extent.count) <= extent.first)
			goto out;
		if ((extent.lower_first + extent.count) <=
		     extent.lower_first)
			goto out;

		/* Do the ranges in extent overlap any previous extents? */
		if (mappings_overlap(&new_map, &extent))
			goto out;

		if ((new_map.nr_extents + 1) == UID_GID_MAP_MAX_EXTENTS &&
		    (next_line != NULL))
			goto out;

		ret = insert_extent(&new_map, &extent);
		if (ret < 0)
			goto out;
		ret = -EINVAL;
	}
	/* Be very certaint the new map actually exists */
	if (new_map.nr_extents == 0)
		goto out;

	ret = -EPERM;
	/* Validate the user is allowed to use user id's mapped to. */
	if (!new_idmap_permitted(file, ns, cap_setid, &new_map))
		goto out;

	ret = -EPERM;
	/* Map the lower ids from the parent user namespace to the
	 * kernel global id space.
	 */
	for (idx = 0; idx < new_map.nr_extents; idx++) {
		struct uid_gid_extent *e;
		u32 lower_first;

		if (new_map.nr_extents <= UID_GID_MAP_MAX_BASE_EXTENTS)
			e = &new_map.extent[idx];
		else
			e = &new_map.forward[idx];

		lower_first = map_id_range_down(parent_map,
						e->lower_first,
						e->count);

		/* Fail if we can not map the specified extent to
		 * the kernel global id space.
		 */
		if (lower_first == (u32) -1)
			goto out;

		e->lower_first = lower_first;
	}
	for (idx = 0; idx < new_map.nr_extents; idx++) {
		struct uid_gid_extent *e;
		u32 lower_first;

		if (new_map.nr_extents <= UID_GID_MAP_MAX_BASE_EXTENTS)
			e = &new_map.extent[idx];
		else
			e = &new_map.forward[idx];

		lower_first = map_id_range_down(parent_map,
						e->lower_first,
						e->count);

		/* Fail if we can not map the specified extent to
		 * the kernel global id space.
		 */
		if (lower_first == (u32) -1)
			goto out;

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
	} else {
		map->forward = new_map.forward;
		map->reverse = new_map.reverse;
	}
				struct user_namespace *ns, int cap_setid,
				struct uid_gid_map *new_map)
{
	/* Allow mapping to your own filesystem ids */
	if ((new_map->nr_extents == 1) && (new_map->extent[0].count == 1)) {
		u32 id = new_map->extent[0].lower_first;
		if (cap_setid == CAP_SETUID) {
			kuid_t uid = make_kuid(ns->parent, id);
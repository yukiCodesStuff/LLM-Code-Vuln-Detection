	return false;
}

bool userns_may_setgroups(const struct user_namespace *ns)
{
	bool allowed;

	mutex_lock(&id_map_mutex);
	/* It is not safe to use setgroups until a gid mapping in
	 * the user namespace has been established.
	 */
	allowed = ns->gid_map.nr_extents != 0;
	mutex_unlock(&id_map_mutex);

	return allowed;
}

static void *userns_get(struct task_struct *task)
{
	struct user_namespace *user_ns;

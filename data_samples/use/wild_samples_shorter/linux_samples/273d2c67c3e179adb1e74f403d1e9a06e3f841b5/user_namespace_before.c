	return false;
}

static void *userns_get(struct task_struct *task)
{
	struct user_namespace *user_ns;

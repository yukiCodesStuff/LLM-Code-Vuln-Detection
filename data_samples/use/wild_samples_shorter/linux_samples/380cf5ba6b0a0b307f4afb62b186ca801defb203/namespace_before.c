	return !mnt_already_visible(ns, mnt, new_mnt_flags);
}

static struct ns_common *mntns_get(struct task_struct *task)
{
	struct ns_common *ns = NULL;
	struct nsproxy *nsproxy;
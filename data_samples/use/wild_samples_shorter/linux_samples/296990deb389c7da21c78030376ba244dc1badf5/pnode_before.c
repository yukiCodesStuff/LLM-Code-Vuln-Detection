	return list_entry(p->mnt_slave_list.next, struct mount, mnt_slave);
}

static inline struct mount *next_slave(struct mount *p)
{
	return list_entry(p->mnt_slave.next, struct mount, mnt_slave);
}
	}
}

static struct mount *next_group(struct mount *m, struct mount *origin)
{
	while (1) {
		while (1) {
	}
}

/*
 * collect all mounts that receive propagation from the mount in @list,
 * and return these additional mounts in the same list.
 * @list: the list of mounts to be unmounted.
	struct mount *mnt;
	LIST_HEAD(to_restore);
	LIST_HEAD(to_umount);

	list_for_each_entry(mnt, list, mnt_list) {
		struct mount *parent = mnt->mnt_parent;
		struct mount *m;

		for (m = propagation_next(parent, parent); m;
		     m = propagation_next(m, parent)) {
			struct mount *child = __lookup_mnt(&m->mnt,
							   mnt->mnt_mountpoint);
			if (!child)
				continue;

			/* Check the child and parents while progress is made */
			while (__propagate_umount(child,
						  &to_umount, &to_restore)) {
				/* Is the parent a umount candidate? */

	umount_list(&to_umount, &to_restore);
	restore_mounts(&to_restore);
	list_splice_tail(&to_umount, list);

	return 0;
}
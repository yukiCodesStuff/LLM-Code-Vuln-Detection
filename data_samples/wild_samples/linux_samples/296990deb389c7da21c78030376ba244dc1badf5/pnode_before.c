{
	return list_entry(p->mnt_slave_list.next, struct mount, mnt_slave);
}

static inline struct mount *next_slave(struct mount *p)
{
	return list_entry(p->mnt_slave.next, struct mount, mnt_slave);
}

static struct mount *get_peer_under_root(struct mount *mnt,
					 struct mnt_namespace *ns,
					 const struct path *root)
{
	struct mount *m = mnt;

	do {
		/* Check the namespace first for optimization */
		if (m->mnt_ns == ns && is_path_reachable(m, m->mnt.mnt_root, root))
			return m;

		m = next_peer(m);
	} while (m != mnt);

	return NULL;
}
		if (master == origin->mnt_master) {
			struct mount *next = next_peer(m);
			return (next == origin) ? NULL : next;
		} else if (m->mnt_slave.next != &master->mnt_slave_list)
			return next_slave(m);

		/* back at master */
		m = master;
	}
}

static struct mount *next_group(struct mount *m, struct mount *origin)
{
	while (1) {
		while (1) {
			struct mount *next;
			if (!IS_MNT_NEW(m) && !list_empty(&m->mnt_slave_list))
				return first_slave(m);
			next = next_peer(m);
			if (m->mnt_group_id == origin->mnt_group_id) {
				if (next == origin)
					return NULL;
			} else if (m->mnt_slave.next != &next->mnt_slave)
				break;
			m = next;
		}
		/* m is the last peer */
		while (1) {
			struct mount *master = m->mnt_master;
			if (m->mnt_slave.next != &master->mnt_slave_list)
				return next_slave(m);
			m = next_peer(master);
			if (master->mnt_group_id == origin->mnt_group_id)
				break;
			if (master->mnt_slave.next == &m->mnt_slave)
				break;
			m = master;
		}
		if (m == origin)
			return NULL;
	}
}
		while (parent->mnt.mnt_flags & MNT_UMOUNT) {
			mp = parent->mnt_mp;
			parent = parent->mnt_parent;
		}
		if (parent != mnt->mnt_parent)
			mnt_change_mountpoint(parent, mp, mnt);
	}
}

/*
 * collect all mounts that receive propagation from the mount in @list,
 * and return these additional mounts in the same list.
 * @list: the list of mounts to be unmounted.
 *
 * vfsmount lock must be held for write
 */
int propagate_umount(struct list_head *list)
{
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
				child = child->mnt_parent;
				if (list_empty(&child->mnt_umounting))
					break;
			}
		}
	}
{
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
				child = child->mnt_parent;
				if (list_empty(&child->mnt_umounting))
					break;
			}
		}
	}
						  &to_umount, &to_restore)) {
				/* Is the parent a umount candidate? */
				child = child->mnt_parent;
				if (list_empty(&child->mnt_umounting))
					break;
			}
		}
	}

	umount_list(&to_umount, &to_restore);
	restore_mounts(&to_restore);
	list_splice_tail(&to_umount, list);

	return 0;
}
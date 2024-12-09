{
	return list_entry(p->mnt_slave_list.next, struct mount, mnt_slave);
}

static inline struct mount *last_slave(struct mount *p)
{
	return list_entry(p->mnt_slave_list.prev, struct mount, mnt_slave);
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

static struct mount *skip_propagation_subtree(struct mount *m,
						struct mount *origin)
{
	/*
	 * Advance m such that propagation_next will not return
	 * the slaves of m.
	 */
	if (!IS_MNT_NEW(m) && !list_empty(&m->mnt_slave_list))
		m = last_slave(m);

	return m;
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

static void cleanup_umount_visitations(struct list_head *visited)
{
	while (!list_empty(visited)) {
		struct mount *mnt =
			list_first_entry(visited, struct mount, mnt_umounting);
		list_del_init(&mnt->mnt_umounting);
	}
{
	struct mount *mnt;
	LIST_HEAD(to_restore);
	LIST_HEAD(to_umount);
	LIST_HEAD(visited);

	/* Find candidates for unmounting */
	list_for_each_entry_reverse(mnt, list, mnt_list) {
		struct mount *parent = mnt->mnt_parent;
		struct mount *m;

		/*
		 * If this mount has already been visited it is known that it's
		 * entire peer group and all of their slaves in the propagation
		 * tree for the mountpoint has already been visited and there is
		 * no need to visit them again.
		 */
		if (!list_empty(&mnt->mnt_umounting))
			continue;

		list_add_tail(&mnt->mnt_umounting, &visited);
		for (m = propagation_next(parent, parent); m;
		     m = propagation_next(m, parent)) {
			struct mount *child = __lookup_mnt(&m->mnt,
							   mnt->mnt_mountpoint);
			if (!child)
				continue;

			if (!list_empty(&child->mnt_umounting)) {
				/*
				 * If the child has already been visited it is
				 * know that it's entire peer group and all of
				 * their slaves in the propgation tree for the
				 * mountpoint has already been visited and there
				 * is no need to visit this subtree again.
				 */
				m = skip_propagation_subtree(m, parent);
				continue;
			} else if (child->mnt.mnt_flags & MNT_UMOUNT) {
				/*
				 * We have come accross an partially unmounted
				 * mount in list that has not been visited yet.
				 * Remember it has been visited and continue
				 * about our merry way.
				 */
				list_add_tail(&child->mnt_umounting, &visited);
				continue;
			}

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

	umount_list(&to_umount, &to_restore);
	restore_mounts(&to_restore);
	cleanup_umount_visitations(&visited);
	list_splice_tail(&to_umount, list);

	return 0;
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
	cleanup_umount_visitations(&visited);
	list_splice_tail(&to_umount, list);

	return 0;
}
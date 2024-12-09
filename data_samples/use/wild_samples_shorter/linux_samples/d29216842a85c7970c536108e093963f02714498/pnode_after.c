		read_sequnlock_excl(&mount_lock);
	}
	hlist_add_head(&child->mnt_hash, list);
	return count_mounts(m->mnt_ns, child);
}

/*
 * mount 'source_mnt' under the destination 'dest_mnt' at
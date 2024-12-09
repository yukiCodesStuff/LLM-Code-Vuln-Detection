		read_sequnlock_excl(&mount_lock);
	}
	hlist_add_head(&child->mnt_hash, list);
	return 0;
}

/*
 * mount 'source_mnt' under the destination 'dest_mnt' at
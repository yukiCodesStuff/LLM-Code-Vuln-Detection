		__del_gref(gref);
	}

	/* It's possible for the target domain to map the just-allocated grant
	 * references by blindly guessing their IDs; if this is done, then
	 * __del_gref will leave them in the queue_gref list. They need to be
	 * added to the global list so that we can free them when they are no
	 * longer referenced.
	 */
	if (unlikely(!list_empty(&queue_gref)))
		list_splice_tail(&queue_gref, &gref_list);
	mutex_unlock(&gref_mutex);
	return rc;
}

static void __del_gref(struct gntalloc_gref *gref)
{
	if (gref->notify.flags & UNMAP_NOTIFY_CLEAR_BYTE) {
		uint8_t *tmp = kmap(gref->page);
		tmp[gref->notify.pgoff] = 0;
		kunmap(gref->page);
	gref->notify.flags = 0;

	if (gref->gref_id) {
		if (gnttab_query_foreign_access(gref->gref_id))
			return;

		if (!gnttab_end_foreign_access_ref(gref->gref_id, 0))
			return;

		gnttab_free_grant_reference(gref->gref_id);
	}

	gref_size--;
	list_del(&gref->next_gref);

	if (gref->page)
		__free_page(gref->page);

	kfree(gref);
}

/* finds contiguous grant references in a file, returns the first */
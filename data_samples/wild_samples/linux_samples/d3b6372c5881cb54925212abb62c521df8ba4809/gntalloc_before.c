	list_for_each_entry_safe(gref, next, &queue_file, next_file) {
		list_del(&gref->next_file);
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
	}
	if (gref->notify.flags & UNMAP_NOTIFY_SEND_EVENT) {
		notify_remote_via_evtchn(gref->notify.event);
		evtchn_put(gref->notify.event);
	}

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
static struct gntalloc_gref *find_grefs(struct gntalloc_file_private_data *priv,
		uint64_t index, uint32_t count)
{
	struct gntalloc_gref *rv = NULL, *gref;
	list_for_each_entry(gref, &priv->list, next_file) {
		if (gref->file_index == index && !rv)
			rv = gref;
		if (rv) {
			if (gref->file_index != index)
				return NULL;
			index += PAGE_SIZE;
			count--;
			if (count == 0)
				return rv;
		}
	if (gref->notify.flags & UNMAP_NOTIFY_SEND_EVENT) {
		notify_remote_via_evtchn(gref->notify.event);
		evtchn_put(gref->notify.event);
	}

	gref->notify.flags = 0;

	if (gref->gref_id) {
		if (gnttab_query_foreign_access(gref->gref_id))
			return;

		if (!gnttab_end_foreign_access_ref(gref->gref_id, 0))
			return;

		gnttab_free_grant_reference(gref->gref_id);
	}
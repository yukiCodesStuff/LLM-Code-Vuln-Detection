	list_for_each_entry_safe(gref, next, &queue_file, next_file) {
		list_del(&gref->next_file);
		__del_gref(gref);
	}

	mutex_unlock(&gref_mutex);
	return rc;
}

static void __del_gref(struct gntalloc_gref *gref)
{
	unsigned long addr;

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
		if (gref->page) {
			addr = (unsigned long)page_to_virt(gref->page);
			gnttab_end_foreign_access(gref->gref_id, 0, addr);
		} else
			gnttab_free_grant_reference(gref->gref_id);
	}
	if (gref->notify.flags & UNMAP_NOTIFY_SEND_EVENT) {
		notify_remote_via_evtchn(gref->notify.event);
		evtchn_put(gref->notify.event);
	}

	gref->notify.flags = 0;

	if (gref->gref_id) {
		if (gref->page) {
			addr = (unsigned long)page_to_virt(gref->page);
			gnttab_end_foreign_access(gref->gref_id, 0, addr);
		} else
			gnttab_free_grant_reference(gref->gref_id);
	}

	gref_size--;
	list_del(&gref->next_gref);

	kfree(gref);
}
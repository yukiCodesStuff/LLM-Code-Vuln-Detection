			rinfo->ring_ref[i] = GRANT_INVALID_REF;
		}
	}
	free_pages_exact(rinfo->ring.sring,
			 info->nr_ring_pages * XEN_PAGE_SIZE);
	rinfo->ring.sring = NULL;

	if (rinfo->irq)
		unbind_from_irqhandler(rinfo->irq, rinfo);
	return BLKIF_RSP_OKAY;
}

/*
 * Return values:
 *  1 response processed.
 *  0 missing further responses.
 * -1 error while processing.
 */
static int blkif_completion(unsigned long *id,
			    struct blkfront_ring_info *rinfo,
			    struct blkif_response *bret)
{
	int i = 0;
	struct scatterlist *sg;
	int num_sg, num_grant;

		/* Wait the second response if not yet here. */
		if (s2->status < REQ_DONE)
			return 0;

		bret->status = blkif_get_final_status(s->status,
						      s2->status);

	}
	/* Add the persistent grant into the list of free grants */
	for (i = 0; i < num_grant; i++) {
		if (!gnttab_try_end_foreign_access(s->grants_used[i]->gref)) {
			/*
			 * If the grant is still mapped by the backend (the
			 * backend has chosen to make this grant persistent)
			 * we add it at the head of the list, so it will be
			 * reused first.
			 */
			if (!info->feature_persistent) {
				pr_alert("backed has not unmapped grant: %u\n",
					 s->grants_used[i]->gref);
				return -1;
			}
			list_add(&s->grants_used[i]->node, &rinfo->grants);
			rinfo->persistent_gnts_c++;
		} else {
			/*
			 * If the grant is not mapped by the backend we add it
			 * to the tail of the list, so it will not be picked
			 * again unless we run out of persistent grants.
			 */
			s->grants_used[i]->gref = GRANT_INVALID_REF;
			list_add_tail(&s->grants_used[i]->node, &rinfo->grants);
		}
	}
	if (s->req.operation == BLKIF_OP_INDIRECT) {
		for (i = 0; i < INDIRECT_GREFS(num_grant); i++) {
			if (!gnttab_try_end_foreign_access(s->indirect_grants[i]->gref)) {
				if (!info->feature_persistent) {
					pr_alert("backed has not unmapped grant: %u\n",
						 s->indirect_grants[i]->gref);
					return -1;
				}
				list_add(&s->indirect_grants[i]->node, &rinfo->grants);
				rinfo->persistent_gnts_c++;
			} else {
				struct page *indirect_page;

				/*
				 * Add the used indirect page back to the list of
				 * available pages for indirect grefs.
				 */
		}
	}

	return 1;
}

static irqreturn_t blkif_interrupt(int irq, void *dev_id)
{
		}

		if (bret.operation != BLKIF_OP_DISCARD) {
			int ret;

			/*
			 * We may need to wait for an extra response if the
			 * I/O request is split in 2
			 */
			ret = blkif_completion(&id, rinfo, &bret);
			if (!ret)
				continue;
			if (unlikely(ret < 0))
				goto err;
		}

		if (add_id_to_freelist(rinfo, id)) {
			WARN(1, "%s: response to %s (id %ld) couldn't be recycled!\n",
	for (i = 0; i < info->nr_ring_pages; i++)
		rinfo->ring_ref[i] = GRANT_INVALID_REF;

	sring = alloc_pages_exact(ring_size, GFP_NOIO);
	if (!sring) {
		xenbus_dev_fatal(dev, -ENOMEM, "allocating shared ring");
		return -ENOMEM;
	}

	err = xenbus_grant_ring(dev, rinfo->ring.sring, info->nr_ring_pages, gref);
	if (err < 0) {
		free_pages_exact(sring, ring_size);
		rinfo->ring.sring = NULL;
		goto fail;
	}
	for (i = 0; i < info->nr_ring_pages; i++)
		list_for_each_entry_safe(gnt_list_entry, tmp, &rinfo->grants,
					 node) {
			if (gnt_list_entry->gref == GRANT_INVALID_REF ||
			    !gnttab_try_end_foreign_access(gnt_list_entry->gref))
				continue;

			list_del(&gnt_list_entry->node);
			rinfo->persistent_gnts_c--;
			gnt_list_entry->gref = GRANT_INVALID_REF;
			list_add_tail(&gnt_list_entry->node, &rinfo->grants);
		}
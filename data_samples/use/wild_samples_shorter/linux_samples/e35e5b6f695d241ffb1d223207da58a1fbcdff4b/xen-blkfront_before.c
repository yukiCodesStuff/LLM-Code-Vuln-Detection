module_param_named(max_ring_page_order, xen_blkif_max_ring_order, int, 0444);
MODULE_PARM_DESC(max_ring_page_order, "Maximum order of pages to be used for the shared ring");

#define BLK_RING_SIZE(info)	\
	__CONST_RING_SIZE(blkif, XEN_PAGE_SIZE * (info)->nr_ring_pages)

/*
	unsigned int feature_discard:1;
	unsigned int feature_secdiscard:1;
	unsigned int feature_persistent:1;
	unsigned int discard_granularity;
	unsigned int discard_alignment;
	/* Number of 4KB segments handled */
	unsigned int max_indirect_segments;
		if (!gnt_list_entry)
			goto out_of_memory;

		if (info->feature_persistent) {
			granted_page = alloc_page(GFP_NOIO);
			if (!granted_page) {
				kfree(gnt_list_entry);
				goto out_of_memory;
			}
	list_for_each_entry_safe(gnt_list_entry, n,
	                         &rinfo->grants, node) {
		list_del(&gnt_list_entry->node);
		if (info->feature_persistent)
			__free_page(gnt_list_entry->page);
		kfree(gnt_list_entry);
		i--;
	}
	/* Assign a gref to this page */
	gnt_list_entry->gref = gnttab_claim_grant_reference(gref_head);
	BUG_ON(gnt_list_entry->gref == -ENOSPC);
	if (info->feature_persistent)
		grant_foreign_access(gnt_list_entry, info);
	else {
		/* Grant access to the GFN passed by the caller */
		gnttab_grant_foreign_access_ref(gnt_list_entry->gref,
	/* Assign a gref to this page */
	gnt_list_entry->gref = gnttab_claim_grant_reference(gref_head);
	BUG_ON(gnt_list_entry->gref == -ENOSPC);
	if (!info->feature_persistent) {
		struct page *indirect_page;

		/* Fetch a pre-allocated page to use for indirect grefs */
		BUG_ON(list_empty(&rinfo->indirect_pages));
		.grant_idx = 0,
		.segments = NULL,
		.rinfo = rinfo,
		.need_copy = rq_data_dir(req) && info->feature_persistent,
	};

	/*
	 * Used to store if we are able to queue the request by just using
{
	blk_queue_write_cache(info->rq, info->feature_flush ? true : false,
			      info->feature_fua ? true : false);
	pr_info("blkfront: %s: %s %s %s %s %s\n",
		info->gd->disk_name, flush_info(info),
		"persistent grants:", info->feature_persistent ?
		"enabled;" : "disabled;", "indirect descriptors:",
		info->max_indirect_segments ? "enabled;" : "disabled;");
}

static int xen_translate_vdev(int vdevice, int *minor, unsigned int *offset)
{
	if (!list_empty(&rinfo->indirect_pages)) {
		struct page *indirect_page, *n;

		BUG_ON(info->feature_persistent);
		list_for_each_entry_safe(indirect_page, n, &rinfo->indirect_pages, lru) {
			list_del(&indirect_page->lru);
			__free_page(indirect_page);
		}
							  NULL);
				rinfo->persistent_gnts_c--;
			}
			if (info->feature_persistent)
				__free_page(persistent_gnt->page);
			kfree(persistent_gnt);
		}
	}
		for (j = 0; j < segs; j++) {
			persistent_gnt = rinfo->shadow[i].grants_used[j];
			gnttab_end_foreign_access(persistent_gnt->gref, NULL);
			if (info->feature_persistent)
				__free_page(persistent_gnt->page);
			kfree(persistent_gnt);
		}

	data.s = s;
	num_sg = s->num_sg;

	if (bret->operation == BLKIF_OP_READ && info->feature_persistent) {
		for_each_sg(s->sg, sg, num_sg, i) {
			BUG_ON(sg->offset + sg->length > PAGE_SIZE);

			data.bvec_offset = sg->offset;
				 * Add the used indirect page back to the list of
				 * available pages for indirect grefs.
				 */
				if (!info->feature_persistent) {
					indirect_page = s->indirect_grants[i]->page;
					list_add(&indirect_page->lru, &rinfo->indirect_pages);
				}
				s->indirect_grants[i]->gref = INVALID_GRANT_REF;
	if (!info)
		return -ENODEV;

	max_page_order = xenbus_read_unsigned(info->xbdev->otherend,
					      "max-ring-page-order", 0);
	ring_page_order = min(xen_blkif_max_ring_order, max_page_order);
	info->nr_ring_pages = 1 << ring_page_order;
	if (err)
		goto out_of_memory;

	if (!info->feature_persistent && info->max_indirect_segments) {
		/*
		 * We are using indirect descriptors but not persistent
		 * grants, we need to allocate a set of pages that can be
		 * used for mapping indirect grefs
		 */
		int num = INDIRECT_GREFS(grants) * BLK_RING_SIZE(info);

		BUG_ON(!list_empty(&rinfo->indirect_pages));
		for (i = 0; i < num; i++) {
			struct page *indirect_page = alloc_page(GFP_KERNEL);
			if (!indirect_page)
				goto out_of_memory;
			list_add(&indirect_page->lru, &rinfo->indirect_pages);
		}
		info->feature_persistent =
			!!xenbus_read_unsigned(info->xbdev->otherend,
					       "feature-persistent", 0);

	indirect_segments = xenbus_read_unsigned(info->xbdev->otherend,
					"feature-max-indirect-segments", 0);
	if (indirect_segments > xen_blkif_max_segments)
	struct blkfront_info *info;
	bool need_schedule_work = false;

	mutex_lock(&blkfront_mutex);

	list_for_each_entry(info, &info_list, info_list) {
		if (info->feature_persistent) {
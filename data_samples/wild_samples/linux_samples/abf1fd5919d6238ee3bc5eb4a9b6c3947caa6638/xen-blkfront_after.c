		if (rinfo->ring_ref[i] != GRANT_INVALID_REF) {
			gnttab_end_foreign_access(rinfo->ring_ref[i], 0, 0);
			rinfo->ring_ref[i] = GRANT_INVALID_REF;
		}
	}
	free_pages_exact(rinfo->ring.sring,
			 info->nr_ring_pages * XEN_PAGE_SIZE);
	rinfo->ring.sring = NULL;

	if (rinfo->irq)
		unbind_from_irqhandler(rinfo->irq, rinfo);
	rinfo->evtchn = rinfo->irq = 0;
}

static void blkif_free(struct blkfront_info *info, int suspend)
{
	unsigned int i;
	struct blkfront_ring_info *rinfo;

	/* Prevent new requests being issued until we fix things up. */
	info->connected = suspend ?
		BLKIF_STATE_SUSPENDED : BLKIF_STATE_DISCONNECTED;
	/* No more blkif_request(). */
	if (info->rq)
		blk_mq_stop_hw_queues(info->rq);

	for_each_rinfo(info, rinfo, i)
		blkif_free_ring(rinfo);

	kvfree(info->rinfo);
	info->rinfo = NULL;
	info->nr_rings = 0;
}

struct copy_from_grant {
	const struct blk_shadow *s;
	unsigned int grant_idx;
	unsigned int bvec_offset;
	char *bvec_data;
};

static void blkif_copy_from_grant(unsigned long gfn, unsigned int offset,
				  unsigned int len, void *data)
{
	struct copy_from_grant *info = data;
	char *shared_data;
	/* Convenient aliases */
	const struct blk_shadow *s = info->s;

	shared_data = kmap_atomic(s->grants_used[info->grant_idx]->page);

	memcpy(info->bvec_data + info->bvec_offset,
	       shared_data + offset, len);

	info->bvec_offset += len;
	info->grant_idx++;

	kunmap_atomic(shared_data);
}

static enum blk_req_status blkif_rsp_to_req_status(int rsp)
{
	switch (rsp)
	{
	case BLKIF_RSP_OKAY:
		return REQ_DONE;
	case BLKIF_RSP_EOPNOTSUPP:
		return REQ_EOPNOTSUPP;
	case BLKIF_RSP_ERROR:
	default:
		return REQ_ERROR;
	}
}

/*
 * Get the final status of the block request based on two ring response
 */
static int blkif_get_final_status(enum blk_req_status s1,
				  enum blk_req_status s2)
{
	BUG_ON(s1 < REQ_DONE);
	BUG_ON(s2 < REQ_DONE);

	if (s1 == REQ_ERROR || s2 == REQ_ERROR)
		return BLKIF_RSP_ERROR;
	else if (s1 == REQ_EOPNOTSUPP || s2 == REQ_EOPNOTSUPP)
		return BLKIF_RSP_EOPNOTSUPP;
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
	struct blkfront_info *info = rinfo->dev_info;
	struct blk_shadow *s = &rinfo->shadow[*id];
	struct copy_from_grant data = {
		.grant_idx = 0,
	};

	num_grant = s->req.operation == BLKIF_OP_INDIRECT ?
		s->req.u.indirect.nr_segments : s->req.u.rw.nr_segments;

	/* The I/O request may be split in two. */
	if (unlikely(s->associated_id != NO_ASSOCIATED_ID)) {
		struct blk_shadow *s2 = &rinfo->shadow[s->associated_id];

		/* Keep the status of the current response in shadow. */
		s->status = blkif_rsp_to_req_status(bret->status);

		/* Wait the second response if not yet here. */
		if (s2->status < REQ_DONE)
			return 0;

		bret->status = blkif_get_final_status(s->status,
						      s2->status);

		/*
		 * All the grants is stored in the first shadow in order
		 * to make the completion code simpler.
		 */
		num_grant += s2->req.u.rw.nr_segments;

		/*
		 * The two responses may not come in order. Only the
		 * first request will store the scatter-gather list.
		 */
		if (s2->num_sg != 0) {
			/* Update "id" with the ID of the first response. */
			*id = s->associated_id;
			s = s2;
		}

		/*
		 * We don't need anymore the second request, so recycling
		 * it now.
		 */
		if (add_id_to_freelist(rinfo, s->associated_id))
			WARN(1, "%s: can't recycle the second part (id = %ld) of the request\n",
			     info->gd->disk_name, s->associated_id);
	}
{
	BUG_ON(s1 < REQ_DONE);
	BUG_ON(s2 < REQ_DONE);

	if (s1 == REQ_ERROR || s2 == REQ_ERROR)
		return BLKIF_RSP_ERROR;
	else if (s1 == REQ_EOPNOTSUPP || s2 == REQ_EOPNOTSUPP)
		return BLKIF_RSP_EOPNOTSUPP;
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
	struct blkfront_info *info = rinfo->dev_info;
	struct blk_shadow *s = &rinfo->shadow[*id];
	struct copy_from_grant data = {
		.grant_idx = 0,
	};

	num_grant = s->req.operation == BLKIF_OP_INDIRECT ?
		s->req.u.indirect.nr_segments : s->req.u.rw.nr_segments;

	/* The I/O request may be split in two. */
	if (unlikely(s->associated_id != NO_ASSOCIATED_ID)) {
		struct blk_shadow *s2 = &rinfo->shadow[s->associated_id];

		/* Keep the status of the current response in shadow. */
		s->status = blkif_rsp_to_req_status(bret->status);

		/* Wait the second response if not yet here. */
		if (s2->status < REQ_DONE)
			return 0;

		bret->status = blkif_get_final_status(s->status,
						      s2->status);

		/*
		 * All the grants is stored in the first shadow in order
		 * to make the completion code simpler.
		 */
		num_grant += s2->req.u.rw.nr_segments;

		/*
		 * The two responses may not come in order. Only the
		 * first request will store the scatter-gather list.
		 */
		if (s2->num_sg != 0) {
			/* Update "id" with the ID of the first response. */
			*id = s->associated_id;
			s = s2;
		}

		/*
		 * We don't need anymore the second request, so recycling
		 * it now.
		 */
		if (add_id_to_freelist(rinfo, s->associated_id))
			WARN(1, "%s: can't recycle the second part (id = %ld) of the request\n",
			     info->gd->disk_name, s->associated_id);
	}

	data.s = s;
	num_sg = s->num_sg;

	if (bret->operation == BLKIF_OP_READ && info->feature_persistent) {
		for_each_sg(s->sg, sg, num_sg, i) {
			BUG_ON(sg->offset + sg->length > PAGE_SIZE);

			data.bvec_offset = sg->offset;
			data.bvec_data = kmap_atomic(sg_page(sg));

			gnttab_foreach_grant_in_range(sg_page(sg),
						      sg->offset,
						      sg->length,
						      blkif_copy_from_grant,
						      &data);

			kunmap_atomic(data.bvec_data);
		}
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
				if (!info->feature_persistent) {
					indirect_page = s->indirect_grants[i]->page;
					list_add(&indirect_page->lru, &rinfo->indirect_pages);
				}
				s->indirect_grants[i]->gref = GRANT_INVALID_REF;
				list_add_tail(&s->indirect_grants[i]->node, &rinfo->grants);
			}
		}
	}

	return 1;
}

static irqreturn_t blkif_interrupt(int irq, void *dev_id)
{
	struct request *req;
	struct blkif_response bret;
	RING_IDX i, rp;
	unsigned long flags;
	struct blkfront_ring_info *rinfo = (struct blkfront_ring_info *)dev_id;
	struct blkfront_info *info = rinfo->dev_info;
	unsigned int eoiflag = XEN_EOI_FLAG_SPURIOUS;

	if (unlikely(info->connected != BLKIF_STATE_CONNECTED)) {
		xen_irq_lateeoi(irq, XEN_EOI_FLAG_SPURIOUS);
		return IRQ_HANDLED;
	}

	spin_lock_irqsave(&rinfo->ring_lock, flags);
 again:
	rp = READ_ONCE(rinfo->ring.sring->rsp_prod);
	virt_rmb(); /* Ensure we see queued responses up to 'rp'. */
	if (RING_RESPONSE_PROD_OVERFLOW(&rinfo->ring, rp)) {
		pr_alert("%s: illegal number of responses %u\n",
			 info->gd->disk_name, rp - rinfo->ring.rsp_cons);
		goto err;
	}

	for (i = rinfo->ring.rsp_cons; i != rp; i++) {
		unsigned long id;
		unsigned int op;

		eoiflag = 0;

		RING_COPY_RESPONSE(&rinfo->ring, i, &bret);
		id = bret.id;

		/*
		 * The backend has messed up and given us an id that we would
		 * never have given to it (we stamp it up to BLK_RING_SIZE -
		 * look in get_id_from_freelist.
		 */
		if (id >= BLK_RING_SIZE(info)) {
			pr_alert("%s: response has incorrect id (%ld)\n",
				 info->gd->disk_name, id);
			goto err;
		}
		if (rinfo->shadow[id].status != REQ_WAITING) {
			pr_alert("%s: response references no pending request\n",
				 info->gd->disk_name);
			goto err;
		}

		rinfo->shadow[id].status = REQ_PROCESSING;
		req  = rinfo->shadow[id].request;

		op = rinfo->shadow[id].req.operation;
		if (op == BLKIF_OP_INDIRECT)
			op = rinfo->shadow[id].req.u.indirect.indirect_op;
		if (bret.operation != op) {
			pr_alert("%s: response has wrong operation (%u instead of %u)\n",
				 info->gd->disk_name, bret.operation, op);
			goto err;
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
			     info->gd->disk_name, op_name(bret.operation), id);
			continue;
		}

		if (bret.status == BLKIF_RSP_OKAY)
			blkif_req(req)->error = BLK_STS_OK;
		else
			blkif_req(req)->error = BLK_STS_IOERR;

		switch (bret.operation) {
		case BLKIF_OP_DISCARD:
			if (unlikely(bret.status == BLKIF_RSP_EOPNOTSUPP)) {
				struct request_queue *rq = info->rq;

				pr_warn_ratelimited("blkfront: %s: %s op failed\n",
					   info->gd->disk_name, op_name(bret.operation));
				blkif_req(req)->error = BLK_STS_NOTSUPP;
				info->feature_discard = 0;
				info->feature_secdiscard = 0;
				blk_queue_flag_clear(QUEUE_FLAG_DISCARD, rq);
				blk_queue_flag_clear(QUEUE_FLAG_SECERASE, rq);
			}
			break;
		case BLKIF_OP_FLUSH_DISKCACHE:
		case BLKIF_OP_WRITE_BARRIER:
			if (unlikely(bret.status == BLKIF_RSP_EOPNOTSUPP)) {
				pr_warn_ratelimited("blkfront: %s: %s op failed\n",
				       info->gd->disk_name, op_name(bret.operation));
				blkif_req(req)->error = BLK_STS_NOTSUPP;
			}
			if (unlikely(bret.status == BLKIF_RSP_ERROR &&
				     rinfo->shadow[id].req.u.rw.nr_segments == 0)) {
				pr_warn_ratelimited("blkfront: %s: empty %s op failed\n",
				       info->gd->disk_name, op_name(bret.operation));
				blkif_req(req)->error = BLK_STS_NOTSUPP;
			}
			if (unlikely(blkif_req(req)->error)) {
				if (blkif_req(req)->error == BLK_STS_NOTSUPP)
					blkif_req(req)->error = BLK_STS_OK;
				info->feature_fua = 0;
				info->feature_flush = 0;
				xlvbd_flush(info);
			}
			fallthrough;
		case BLKIF_OP_READ:
		case BLKIF_OP_WRITE:
			if (unlikely(bret.status != BLKIF_RSP_OKAY))
				dev_dbg_ratelimited(&info->xbdev->dev,
					"Bad return from blkdev data request: %#x\n",
					bret.status);

			break;
		default:
			BUG();
		}

		if (likely(!blk_should_fake_timeout(req->q)))
			blk_mq_complete_request(req);
	}

	rinfo->ring.rsp_cons = i;

	if (i != rinfo->ring.req_prod_pvt) {
		int more_to_do;
		RING_FINAL_CHECK_FOR_RESPONSES(&rinfo->ring, more_to_do);
		if (more_to_do)
			goto again;
	} else
		rinfo->ring.sring->rsp_event = i + 1;

	kick_pending_request_queues_locked(rinfo);

	spin_unlock_irqrestore(&rinfo->ring_lock, flags);

	xen_irq_lateeoi(irq, eoiflag);

	return IRQ_HANDLED;

 err:
	info->connected = BLKIF_STATE_ERROR;

	spin_unlock_irqrestore(&rinfo->ring_lock, flags);

	/* No EOI in order to avoid further interrupts. */

	pr_alert("%s disabled for further use\n", info->gd->disk_name);
	return IRQ_HANDLED;
}


static int setup_blkring(struct xenbus_device *dev,
			 struct blkfront_ring_info *rinfo)
{
	struct blkif_sring *sring;
	int err, i;
	struct blkfront_info *info = rinfo->dev_info;
	unsigned long ring_size = info->nr_ring_pages * XEN_PAGE_SIZE;
	grant_ref_t gref[XENBUS_MAX_RING_GRANTS];

	for (i = 0; i < info->nr_ring_pages; i++)
		rinfo->ring_ref[i] = GRANT_INVALID_REF;

	sring = alloc_pages_exact(ring_size, GFP_NOIO);
	if (!sring) {
		xenbus_dev_fatal(dev, -ENOMEM, "allocating shared ring");
		return -ENOMEM;
	}
	SHARED_RING_INIT(sring);
	FRONT_RING_INIT(&rinfo->ring, sring, ring_size);

	err = xenbus_grant_ring(dev, rinfo->ring.sring, info->nr_ring_pages, gref);
	if (err < 0) {
		free_pages_exact(sring, ring_size);
		rinfo->ring.sring = NULL;
		goto fail;
	}
	for (i = 0; i < info->nr_ring_pages; i++)
		rinfo->ring_ref[i] = gref[i];

	err = xenbus_alloc_evtchn(dev, &rinfo->evtchn);
	if (err)
		goto fail;

	err = bind_evtchn_to_irqhandler_lateeoi(rinfo->evtchn, blkif_interrupt,
						0, "blkif", rinfo);
	if (err <= 0) {
		xenbus_dev_fatal(dev, err,
				 "bind_evtchn_to_irqhandler failed");
		goto fail;
	}
	rinfo->irq = err;

	return 0;
fail:
	blkif_free(info, 0);
	return err;
}

/*
 * Write out per-ring/queue nodes including ring-ref and event-channel, and each
 * ring buffer may have multi pages depending on ->nr_ring_pages.
 */
static int write_per_ring_nodes(struct xenbus_transaction xbt,
				struct blkfront_ring_info *rinfo, const char *dir)
{
	int err;
	unsigned int i;
	const char *message = NULL;
	struct blkfront_info *info = rinfo->dev_info;

	if (info->nr_ring_pages == 1) {
		err = xenbus_printf(xbt, dir, "ring-ref", "%u", rinfo->ring_ref[0]);
		if (err) {
			message = "writing ring-ref";
			goto abort_transaction;
		}
	} else {
		for (i = 0; i < info->nr_ring_pages; i++) {
			char ring_ref_name[RINGREF_NAME_LEN];

			snprintf(ring_ref_name, RINGREF_NAME_LEN, "ring-ref%u", i);
			err = xenbus_printf(xbt, dir, ring_ref_name,
					    "%u", rinfo->ring_ref[i]);
			if (err) {
				message = "writing ring-ref";
				goto abort_transaction;
			}
		}
	}

	err = xenbus_printf(xbt, dir, "event-channel", "%u", rinfo->evtchn);
	if (err) {
		message = "writing event-channel";
		goto abort_transaction;
	}

	return 0;

abort_transaction:
	xenbus_transaction_end(xbt, 1);
	if (message)
		xenbus_dev_fatal(info->xbdev, err, "%s", message);

	return err;
}

/* Common code used when first setting up, and when resuming. */
static int talk_to_blkback(struct xenbus_device *dev,
			   struct blkfront_info *info)
{
	const char *message = NULL;
	struct xenbus_transaction xbt;
	int err;
	unsigned int i, max_page_order;
	unsigned int ring_page_order;
	struct blkfront_ring_info *rinfo;

	if (!info)
		return -ENODEV;

	max_page_order = xenbus_read_unsigned(info->xbdev->otherend,
					      "max-ring-page-order", 0);
	ring_page_order = min(xen_blkif_max_ring_order, max_page_order);
	info->nr_ring_pages = 1 << ring_page_order;

	err = negotiate_mq(info);
	if (err)
		goto destroy_blkring;

	for_each_rinfo(info, rinfo, i) {
		/* Create shared ring, alloc event channel. */
		err = setup_blkring(dev, rinfo);
		if (err)
			goto destroy_blkring;
	}

again:
	err = xenbus_transaction_start(&xbt);
	if (err) {
		xenbus_dev_fatal(dev, err, "starting transaction");
		goto destroy_blkring;
	}

	if (info->nr_ring_pages > 1) {
		err = xenbus_printf(xbt, dev->nodename, "ring-page-order", "%u",
				    ring_page_order);
		if (err) {
			message = "writing ring-page-order";
			goto abort_transaction;
		}
	}

	/* We already got the number of queues/rings in _probe */
	if (info->nr_rings == 1) {
		err = write_per_ring_nodes(xbt, info->rinfo, dev->nodename);
		if (err)
			goto destroy_blkring;
	} else {
		char *path;
		size_t pathsize;

		err = xenbus_printf(xbt, dev->nodename, "multi-queue-num-queues", "%u",
				    info->nr_rings);
		if (err) {
			message = "writing multi-queue-num-queues";
			goto abort_transaction;
		}

		pathsize = strlen(dev->nodename) + QUEUE_NAME_LEN;
		path = kmalloc(pathsize, GFP_KERNEL);
		if (!path) {
			err = -ENOMEM;
			message = "ENOMEM while writing ring references";
			goto abort_transaction;
		}

		for_each_rinfo(info, rinfo, i) {
			memset(path, 0, pathsize);
			snprintf(path, pathsize, "%s/queue-%u", dev->nodename, i);
			err = write_per_ring_nodes(xbt, rinfo, path);
			if (err) {
				kfree(path);
				goto destroy_blkring;
			}
		}
		kfree(path);
	}
	err = xenbus_printf(xbt, dev->nodename, "protocol", "%s",
			    XEN_IO_PROTO_ABI_NATIVE);
	if (err) {
		message = "writing protocol";
		goto abort_transaction;
	}
	err = xenbus_printf(xbt, dev->nodename, "feature-persistent", "%u",
			info->feature_persistent);
	if (err)
		dev_warn(&dev->dev,
			 "writing persistent grants feature to xenbus");

	err = xenbus_transaction_end(xbt, 0);
	if (err) {
		if (err == -EAGAIN)
			goto again;
		xenbus_dev_fatal(dev, err, "completing transaction");
		goto destroy_blkring;
	}

	for_each_rinfo(info, rinfo, i) {
		unsigned int j;

		for (j = 0; j < BLK_RING_SIZE(info); j++)
			rinfo->shadow[j].req.u.rw.id = j + 1;
		rinfo->shadow[BLK_RING_SIZE(info)-1].req.u.rw.id = 0x0fffffff;
	}
	xenbus_switch_state(dev, XenbusStateInitialised);

	return 0;

 abort_transaction:
	xenbus_transaction_end(xbt, 1);
	if (message)
		xenbus_dev_fatal(dev, err, "%s", message);
 destroy_blkring:
	blkif_free(info, 0);
	return err;
}

static int negotiate_mq(struct blkfront_info *info)
{
	unsigned int backend_max_queues;
	unsigned int i;
	struct blkfront_ring_info *rinfo;

	BUG_ON(info->nr_rings);

	/* Check if backend supports multiple queues. */
	backend_max_queues = xenbus_read_unsigned(info->xbdev->otherend,
						  "multi-queue-max-queues", 1);
	info->nr_rings = min(backend_max_queues, xen_blkif_max_queues);
	/* We need at least one ring. */
	if (!info->nr_rings)
		info->nr_rings = 1;

	info->rinfo_size = struct_size(info->rinfo, shadow,
				       BLK_RING_SIZE(info));
	info->rinfo = kvcalloc(info->nr_rings, info->rinfo_size, GFP_KERNEL);
	if (!info->rinfo) {
		xenbus_dev_fatal(info->xbdev, -ENOMEM, "allocating ring_info structure");
		info->nr_rings = 0;
		return -ENOMEM;
	}

	for_each_rinfo(info, rinfo, i) {
		INIT_LIST_HEAD(&rinfo->indirect_pages);
		INIT_LIST_HEAD(&rinfo->grants);
		rinfo->dev_info = info;
		INIT_WORK(&rinfo->work, blkif_restart_queue);
		spin_lock_init(&rinfo->ring_lock);
	}
	return 0;
}

/* Enable the persistent grants feature. */
static bool feature_persistent = true;
module_param(feature_persistent, bool, 0644);
MODULE_PARM_DESC(feature_persistent,
		"Enables the persistent grants feature");

/*
 * Entry point to this code when a new device is created.  Allocate the basic
 * structures and the ring buffer for communication with the backend, and
 * inform the backend of the appropriate details for those.  Switch to
 * Initialised state.
 */
static int blkfront_probe(struct xenbus_device *dev,
			  const struct xenbus_device_id *id)
{
	int err, vdevice;
	struct blkfront_info *info;

	/* FIXME: Use dynamic device id if this is not set. */
	err = xenbus_scanf(XBT_NIL, dev->nodename,
			   "virtual-device", "%i", &vdevice);
	if (err != 1) {
		/* go looking in the extended area instead */
		err = xenbus_scanf(XBT_NIL, dev->nodename, "virtual-device-ext",
				   "%i", &vdevice);
		if (err != 1) {
			xenbus_dev_fatal(dev, err, "reading virtual-device");
			return err;
		}
	}

	if (xen_hvm_domain()) {
		char *type;
		int len;
		/* no unplug has been done: do not hook devices != xen vbds */
		if (xen_has_pv_and_legacy_disk_devices()) {
			int major;

			if (!VDEV_IS_EXTENDED(vdevice))
				major = BLKIF_MAJOR(vdevice);
			else
				major = XENVBD_MAJOR;

			if (major != XENVBD_MAJOR) {
				printk(KERN_INFO
						"%s: HVM does not support vbd %d as xen block device\n",
						__func__, vdevice);
				return -ENODEV;
			}
		}
		/* do not create a PV cdrom device if we are an HVM guest */
		type = xenbus_read(XBT_NIL, dev->nodename, "device-type", &len);
		if (IS_ERR(type))
			return -ENODEV;
		if (strncmp(type, "cdrom", 5) == 0) {
			kfree(type);
			return -ENODEV;
		}
		kfree(type);
	}
	info = kzalloc(sizeof(*info), GFP_KERNEL);
	if (!info) {
		xenbus_dev_fatal(dev, -ENOMEM, "allocating info structure");
		return -ENOMEM;
	}

	info->xbdev = dev;

	mutex_init(&info->mutex);
	info->vdevice = vdevice;
	info->connected = BLKIF_STATE_DISCONNECTED;

	info->feature_persistent = feature_persistent;

	/* Front end dir is a number, which is used as the id. */
	info->handle = simple_strtoul(strrchr(dev->nodename, '/')+1, NULL, 0);
	dev_set_drvdata(&dev->dev, info);

	mutex_lock(&blkfront_mutex);
	list_add(&info->info_list, &info_list);
	mutex_unlock(&blkfront_mutex);

	return 0;
}

static int blkif_recover(struct blkfront_info *info)
{
	unsigned int r_index;
	struct request *req, *n;
	int rc;
	struct bio *bio;
	unsigned int segs;
	struct blkfront_ring_info *rinfo;

	blkfront_gather_backend_features(info);
	/* Reset limits changed by blk_mq_update_nr_hw_queues(). */
	blkif_set_queue_limits(info);
	segs = info->max_indirect_segments ? : BLKIF_MAX_SEGMENTS_PER_REQUEST;
	blk_queue_max_segments(info->rq, segs / GRANTS_PER_PSEG);

	for_each_rinfo(info, rinfo, r_index) {
		rc = blkfront_setup_indirect(rinfo);
		if (rc)
			return rc;
	}
	xenbus_switch_state(info->xbdev, XenbusStateConnected);

	/* Now safe for us to use the shared ring */
	info->connected = BLKIF_STATE_CONNECTED;

	for_each_rinfo(info, rinfo, r_index) {
		/* Kick any other new requests queued since we resumed */
		kick_pending_request_queues(rinfo);
	}

	list_for_each_entry_safe(req, n, &info->requests, queuelist) {
		/* Requeue pending requests (flush or discard) */
		list_del_init(&req->queuelist);
		BUG_ON(req->nr_phys_segments > segs);
		blk_mq_requeue_request(req, false);
	}
	blk_mq_start_stopped_hw_queues(info->rq, true);
	blk_mq_kick_requeue_list(info->rq);

	while ((bio = bio_list_pop(&info->bio_list)) != NULL) {
		/* Traverse the list of pending bios and re-queue them */
		submit_bio(bio);
	}

	return 0;
}

/*
 * We are reconnecting to the backend, due to a suspend/resume, or a backend
 * driver restart.  We tear down our blkif structure and recreate it, but
 * leave the device-layer structures intact so that this is transparent to the
 * rest of the kernel.
 */
static int blkfront_resume(struct xenbus_device *dev)
{
	struct blkfront_info *info = dev_get_drvdata(&dev->dev);
	int err = 0;
	unsigned int i, j;
	struct blkfront_ring_info *rinfo;

	dev_dbg(&dev->dev, "blkfront_resume: %s\n", dev->nodename);

	bio_list_init(&info->bio_list);
	INIT_LIST_HEAD(&info->requests);
	for_each_rinfo(info, rinfo, i) {
		struct bio_list merge_bio;
		struct blk_shadow *shadow = rinfo->shadow;

		for (j = 0; j < BLK_RING_SIZE(info); j++) {
			/* Not in use? */
			if (!shadow[j].request)
				continue;

			/*
			 * Get the bios in the request so we can re-queue them.
			 */
			if (req_op(shadow[j].request) == REQ_OP_FLUSH ||
			    req_op(shadow[j].request) == REQ_OP_DISCARD ||
			    req_op(shadow[j].request) == REQ_OP_SECURE_ERASE ||
			    shadow[j].request->cmd_flags & REQ_FUA) {
				/*
				 * Flush operations don't contain bios, so
				 * we need to requeue the whole request
				 *
				 * XXX: but this doesn't make any sense for a
				 * write with the FUA flag set..
				 */
				list_add(&shadow[j].request->queuelist, &info->requests);
				continue;
			}
			merge_bio.head = shadow[j].request->bio;
			merge_bio.tail = shadow[j].request->biotail;
			bio_list_merge(&info->bio_list, &merge_bio);
			shadow[j].request->bio = NULL;
			blk_mq_end_request(shadow[j].request, BLK_STS_OK);
		}
	}

	blkif_free(info, info->connected == BLKIF_STATE_CONNECTED);

	err = talk_to_blkback(dev, info);
	if (!err)
		blk_mq_update_nr_hw_queues(&info->tag_set, info->nr_rings);

	/*
	 * We have to wait for the backend to switch to
	 * connected state, since we want to read which
	 * features it supports.
	 */

	return err;
}

static void blkfront_closing(struct blkfront_info *info)
{
	struct xenbus_device *xbdev = info->xbdev;
	struct blkfront_ring_info *rinfo;
	unsigned int i;

	if (xbdev->state == XenbusStateClosing)
		return;

	/* No more blkif_request(). */
	blk_mq_stop_hw_queues(info->rq);
	blk_mark_disk_dead(info->gd);
	set_capacity(info->gd, 0);

	for_each_rinfo(info, rinfo, i) {
		/* No more gnttab callback work. */
		gnttab_cancel_free_callback(&rinfo->callback);

		/* Flush gnttab callback work. Must be done with no locks held. */
		flush_work(&rinfo->work);
	}

	xenbus_frontend_closed(xbdev);
}

static void blkfront_setup_discard(struct blkfront_info *info)
{
	info->feature_discard = 1;
	info->discard_granularity = xenbus_read_unsigned(info->xbdev->otherend,
							 "discard-granularity",
							 0);
	info->discard_alignment = xenbus_read_unsigned(info->xbdev->otherend,
						       "discard-alignment", 0);
	info->feature_secdiscard =
		!!xenbus_read_unsigned(info->xbdev->otherend, "discard-secure",
				       0);
}

static int blkfront_setup_indirect(struct blkfront_ring_info *rinfo)
{
	unsigned int psegs, grants, memflags;
	int err, i;
	struct blkfront_info *info = rinfo->dev_info;

	memflags = memalloc_noio_save();

	if (info->max_indirect_segments == 0) {
		if (!HAS_EXTRA_REQ)
			grants = BLKIF_MAX_SEGMENTS_PER_REQUEST;
		else {
			/*
			 * When an extra req is required, the maximum
			 * grants supported is related to the size of the
			 * Linux block segment.
			 */
			grants = GRANTS_PER_PSEG;
		}
	}
	else
		grants = info->max_indirect_segments;
	psegs = DIV_ROUND_UP(grants, GRANTS_PER_PSEG);

	err = fill_grant_buffer(rinfo,
				(grants + INDIRECT_GREFS(grants)) * BLK_RING_SIZE(info));
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
	}

	for (i = 0; i < BLK_RING_SIZE(info); i++) {
		rinfo->shadow[i].grants_used =
			kvcalloc(grants,
				 sizeof(rinfo->shadow[i].grants_used[0]),
				 GFP_KERNEL);
		rinfo->shadow[i].sg = kvcalloc(psegs,
					       sizeof(rinfo->shadow[i].sg[0]),
					       GFP_KERNEL);
		if (info->max_indirect_segments)
			rinfo->shadow[i].indirect_grants =
				kvcalloc(INDIRECT_GREFS(grants),
					 sizeof(rinfo->shadow[i].indirect_grants[0]),
					 GFP_KERNEL);
		if ((rinfo->shadow[i].grants_used == NULL) ||
			(rinfo->shadow[i].sg == NULL) ||
		     (info->max_indirect_segments &&
		     (rinfo->shadow[i].indirect_grants == NULL)))
			goto out_of_memory;
		sg_init_table(rinfo->shadow[i].sg, psegs);
	}

	memalloc_noio_restore(memflags);

	return 0;

out_of_memory:
	for (i = 0; i < BLK_RING_SIZE(info); i++) {
		kvfree(rinfo->shadow[i].grants_used);
		rinfo->shadow[i].grants_used = NULL;
		kvfree(rinfo->shadow[i].sg);
		rinfo->shadow[i].sg = NULL;
		kvfree(rinfo->shadow[i].indirect_grants);
		rinfo->shadow[i].indirect_grants = NULL;
	}
	if (!list_empty(&rinfo->indirect_pages)) {
		struct page *indirect_page, *n;
		list_for_each_entry_safe(indirect_page, n, &rinfo->indirect_pages, lru) {
			list_del(&indirect_page->lru);
			__free_page(indirect_page);
		}
	}

	memalloc_noio_restore(memflags);

	return -ENOMEM;
}

/*
 * Gather all backend feature-*
 */
static void blkfront_gather_backend_features(struct blkfront_info *info)
{
	unsigned int indirect_segments;

	info->feature_flush = 0;
	info->feature_fua = 0;

	/*
	 * If there's no "feature-barrier" defined, then it means
	 * we're dealing with a very old backend which writes
	 * synchronously; nothing to do.
	 *
	 * If there are barriers, then we use flush.
	 */
	if (xenbus_read_unsigned(info->xbdev->otherend, "feature-barrier", 0)) {
		info->feature_flush = 1;
		info->feature_fua = 1;
	}

	/*
	 * And if there is "feature-flush-cache" use that above
	 * barriers.
	 */
	if (xenbus_read_unsigned(info->xbdev->otherend, "feature-flush-cache",
				 0)) {
		info->feature_flush = 1;
		info->feature_fua = 0;
	}

	if (xenbus_read_unsigned(info->xbdev->otherend, "feature-discard", 0))
		blkfront_setup_discard(info);

	if (info->feature_persistent)
		info->feature_persistent =
			!!xenbus_read_unsigned(info->xbdev->otherend,
					       "feature-persistent", 0);

	indirect_segments = xenbus_read_unsigned(info->xbdev->otherend,
					"feature-max-indirect-segments", 0);
	if (indirect_segments > xen_blkif_max_segments)
		indirect_segments = xen_blkif_max_segments;
	if (indirect_segments <= BLKIF_MAX_SEGMENTS_PER_REQUEST)
		indirect_segments = 0;
	info->max_indirect_segments = indirect_segments;

	if (info->feature_persistent) {
		mutex_lock(&blkfront_mutex);
		schedule_delayed_work(&blkfront_work, HZ * 10);
		mutex_unlock(&blkfront_mutex);
	}
}

/*
 * Invoked when the backend is finally 'ready' (and has told produced
 * the details about the physical device - #sectors, size, etc).
 */
static void blkfront_connect(struct blkfront_info *info)
{
	unsigned long long sectors;
	unsigned long sector_size;
	unsigned int physical_sector_size;
	int err, i;
	struct blkfront_ring_info *rinfo;

	switch (info->connected) {
	case BLKIF_STATE_CONNECTED:
		/*
		 * Potentially, the back-end may be signalling
		 * a capacity change; update the capacity.
		 */
		err = xenbus_scanf(XBT_NIL, info->xbdev->otherend,
				   "sectors", "%Lu", &sectors);
		if (XENBUS_EXIST_ERR(err))
			return;
		printk(KERN_INFO "Setting capacity to %Lu\n",
		       sectors);
		set_capacity_and_notify(info->gd, sectors);

		return;
	case BLKIF_STATE_SUSPENDED:
		/*
		 * If we are recovering from suspension, we need to wait
		 * for the backend to announce it's features before
		 * reconnecting, at least we need to know if the backend
		 * supports indirect descriptors, and how many.
		 */
		blkif_recover(info);
		return;

	default:
		break;
	}

	dev_dbg(&info->xbdev->dev, "%s:%s.\n",
		__func__, info->xbdev->otherend);

	err = xenbus_gather(XBT_NIL, info->xbdev->otherend,
			    "sectors", "%llu", &sectors,
			    "info", "%u", &info->vdisk_info,
			    "sector-size", "%lu", &sector_size,
			    NULL);
	if (err) {
		xenbus_dev_fatal(info->xbdev, err,
				 "reading backend fields at %s",
				 info->xbdev->otherend);
		return;
	}

	/*
	 * physical-sector-size is a newer field, so old backends may not
	 * provide this. Assume physical sector size to be the same as
	 * sector_size in that case.
	 */
	physical_sector_size = xenbus_read_unsigned(info->xbdev->otherend,
						    "physical-sector-size",
						    sector_size);
	blkfront_gather_backend_features(info);
	for_each_rinfo(info, rinfo, i) {
		err = blkfront_setup_indirect(rinfo);
		if (err) {
			xenbus_dev_fatal(info->xbdev, err, "setup_indirect at %s",
					 info->xbdev->otherend);
			blkif_free(info, 0);
			break;
		}
	}

	err = xlvbd_alloc_gendisk(sectors, info, sector_size,
				  physical_sector_size);
	if (err) {
		xenbus_dev_fatal(info->xbdev, err, "xlvbd_add at %s",
				 info->xbdev->otherend);
		goto fail;
	}

	xenbus_switch_state(info->xbdev, XenbusStateConnected);

	/* Kick pending requests. */
	info->connected = BLKIF_STATE_CONNECTED;
	for_each_rinfo(info, rinfo, i)
		kick_pending_request_queues(rinfo);

	err = device_add_disk(&info->xbdev->dev, info->gd, NULL);
	if (err) {
		blk_cleanup_disk(info->gd);
		blk_mq_free_tag_set(&info->tag_set);
		info->rq = NULL;
		goto fail;
	}

	info->is_ready = 1;
	return;

fail:
	blkif_free(info, 0);
	return;
}

/*
 * Callback received when the backend's state changes.
 */
static void blkback_changed(struct xenbus_device *dev,
			    enum xenbus_state backend_state)
{
	struct blkfront_info *info = dev_get_drvdata(&dev->dev);

	dev_dbg(&dev->dev, "blkfront:blkback_changed to state %d.\n", backend_state);

	switch (backend_state) {
	case XenbusStateInitWait:
		if (dev->state != XenbusStateInitialising)
			break;
		if (talk_to_blkback(dev, info))
			break;
		break;
	case XenbusStateInitialising:
	case XenbusStateInitialised:
	case XenbusStateReconfiguring:
	case XenbusStateReconfigured:
	case XenbusStateUnknown:
		break;

	case XenbusStateConnected:
		/*
		 * talk_to_blkback sets state to XenbusStateInitialised
		 * and blkfront_connect sets it to XenbusStateConnected
		 * (if connection went OK).
		 *
		 * If the backend (or toolstack) decides to poke at backend
		 * state (and re-trigger the watch by setting the state repeatedly
		 * to XenbusStateConnected (4)) we need to deal with this.
		 * This is allowed as this is used to communicate to the guest
		 * that the size of disk has changed!
		 */
		if ((dev->state != XenbusStateInitialised) &&
		    (dev->state != XenbusStateConnected)) {
			if (talk_to_blkback(dev, info))
				break;
		}

		blkfront_connect(info);
		break;

	case XenbusStateClosed:
		if (dev->state == XenbusStateClosed)
			break;
		fallthrough;
	case XenbusStateClosing:
		blkfront_closing(info);
		break;
	}
}

static int blkfront_remove(struct xenbus_device *xbdev)
{
	struct blkfront_info *info = dev_get_drvdata(&xbdev->dev);

	dev_dbg(&xbdev->dev, "%s removed", xbdev->nodename);

	del_gendisk(info->gd);

	mutex_lock(&blkfront_mutex);
	list_del(&info->info_list);
	mutex_unlock(&blkfront_mutex);

	blkif_free(info, 0);
	xlbd_release_minors(info->gd->first_minor, info->gd->minors);
	blk_cleanup_disk(info->gd);
	blk_mq_free_tag_set(&info->tag_set);

	kfree(info);
	return 0;
}

static int blkfront_is_ready(struct xenbus_device *dev)
{
	struct blkfront_info *info = dev_get_drvdata(&dev->dev);

	return info->is_ready && info->xbdev;
}

static const struct block_device_operations xlvbd_block_fops =
{
	.owner = THIS_MODULE,
	.getgeo = blkif_getgeo,
	.ioctl = blkif_ioctl,
	.compat_ioctl = blkdev_compat_ptr_ioctl,
};


static const struct xenbus_device_id blkfront_ids[] = {
	{ "vbd" },
	{ "" }
};

static struct xenbus_driver blkfront_driver = {
	.ids  = blkfront_ids,
	.probe = blkfront_probe,
	.remove = blkfront_remove,
	.resume = blkfront_resume,
	.otherend_changed = blkback_changed,
	.is_ready = blkfront_is_ready,
};

static void purge_persistent_grants(struct blkfront_info *info)
{
	unsigned int i;
	unsigned long flags;
	struct blkfront_ring_info *rinfo;

	for_each_rinfo(info, rinfo, i) {
		struct grant *gnt_list_entry, *tmp;

		spin_lock_irqsave(&rinfo->ring_lock, flags);

		if (rinfo->persistent_gnts_c == 0) {
			spin_unlock_irqrestore(&rinfo->ring_lock, flags);
			continue;
		}

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

		spin_unlock_irqrestore(&rinfo->ring_lock, flags);
	}
}

static void blkfront_delay_work(struct work_struct *work)
{
	struct blkfront_info *info;
	bool need_schedule_work = false;

	mutex_lock(&blkfront_mutex);

	list_for_each_entry(info, &info_list, info_list) {
		if (info->feature_persistent) {
			need_schedule_work = true;
			mutex_lock(&info->mutex);
			purge_persistent_grants(info);
			mutex_unlock(&info->mutex);
		}
	}

	if (need_schedule_work)
		schedule_delayed_work(&blkfront_work, HZ * 10);

	mutex_unlock(&blkfront_mutex);
}

static int __init xlblk_init(void)
{
	int ret;
	int nr_cpus = num_online_cpus();

	if (!xen_domain())
		return -ENODEV;

	if (!xen_has_pv_disk_devices())
		return -ENODEV;

	if (register_blkdev(XENVBD_MAJOR, DEV_NAME)) {
		pr_warn("xen_blk: can't get major %d with name %s\n",
			XENVBD_MAJOR, DEV_NAME);
		return -ENODEV;
	}

	if (xen_blkif_max_segments < BLKIF_MAX_SEGMENTS_PER_REQUEST)
		xen_blkif_max_segments = BLKIF_MAX_SEGMENTS_PER_REQUEST;

	if (xen_blkif_max_ring_order > XENBUS_MAX_RING_GRANT_ORDER) {
		pr_info("Invalid max_ring_order (%d), will use default max: %d.\n",
			xen_blkif_max_ring_order, XENBUS_MAX_RING_GRANT_ORDER);
		xen_blkif_max_ring_order = XENBUS_MAX_RING_GRANT_ORDER;
	}

	if (xen_blkif_max_queues > nr_cpus) {
		pr_info("Invalid max_queues (%d), will use default max: %d.\n",
			xen_blkif_max_queues, nr_cpus);
		xen_blkif_max_queues = nr_cpus;
	}

	INIT_DELAYED_WORK(&blkfront_work, blkfront_delay_work);

	ret = xenbus_register_frontend(&blkfront_driver);
	if (ret) {
		unregister_blkdev(XENVBD_MAJOR, DEV_NAME);
		return ret;
	}

	return 0;
}
module_init(xlblk_init);


static void __exit xlblk_exit(void)
{
	cancel_delayed_work_sync(&blkfront_work);

	xenbus_unregister_driver(&blkfront_driver);
	unregister_blkdev(XENVBD_MAJOR, DEV_NAME);
	kfree(minors);
}
module_exit(xlblk_exit);

MODULE_DESCRIPTION("Xen virtual block device frontend");
MODULE_LICENSE("GPL");
MODULE_ALIAS_BLOCKDEV_MAJOR(XENVBD_MAJOR);
MODULE_ALIAS("xen:vbd");
MODULE_ALIAS("xenblk");
	if (unlikely(s->associated_id != NO_ASSOCIATED_ID)) {
		struct blk_shadow *s2 = &rinfo->shadow[s->associated_id];

		/* Keep the status of the current response in shadow. */
		s->status = blkif_rsp_to_req_status(bret->status);

		/* Wait the second response if not yet here. */
		if (s2->status < REQ_DONE)
			return 0;

		bret->status = blkif_get_final_status(s->status,
						      s2->status);

		/*
		 * All the grants is stored in the first shadow in order
		 * to make the completion code simpler.
		 */
		num_grant += s2->req.u.rw.nr_segments;

		/*
		 * The two responses may not come in order. Only the
		 * first request will store the scatter-gather list.
		 */
		if (s2->num_sg != 0) {
			/* Update "id" with the ID of the first response. */
			*id = s->associated_id;
			s = s2;
		}
		for_each_sg(s->sg, sg, num_sg, i) {
			BUG_ON(sg->offset + sg->length > PAGE_SIZE);

			data.bvec_offset = sg->offset;
			data.bvec_data = kmap_atomic(sg_page(sg));

			gnttab_foreach_grant_in_range(sg_page(sg),
						      sg->offset,
						      sg->length,
						      blkif_copy_from_grant,
						      &data);

			kunmap_atomic(data.bvec_data);
		}
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
				if (!info->feature_persistent) {
					indirect_page = s->indirect_grants[i]->page;
					list_add(&indirect_page->lru, &rinfo->indirect_pages);
				}
				s->indirect_grants[i]->gref = GRANT_INVALID_REF;
				list_add_tail(&s->indirect_grants[i]->node, &rinfo->grants);
			}
		}
	}

	return 1;
}

static irqreturn_t blkif_interrupt(int irq, void *dev_id)
{
	struct request *req;
	struct blkif_response bret;
	RING_IDX i, rp;
	unsigned long flags;
	struct blkfront_ring_info *rinfo = (struct blkfront_ring_info *)dev_id;
	struct blkfront_info *info = rinfo->dev_info;
	unsigned int eoiflag = XEN_EOI_FLAG_SPURIOUS;

	if (unlikely(info->connected != BLKIF_STATE_CONNECTED)) {
		xen_irq_lateeoi(irq, XEN_EOI_FLAG_SPURIOUS);
		return IRQ_HANDLED;
	}

	spin_lock_irqsave(&rinfo->ring_lock, flags);
 again:
	rp = READ_ONCE(rinfo->ring.sring->rsp_prod);
	virt_rmb(); /* Ensure we see queued responses up to 'rp'. */
	if (RING_RESPONSE_PROD_OVERFLOW(&rinfo->ring, rp)) {
		pr_alert("%s: illegal number of responses %u\n",
			 info->gd->disk_name, rp - rinfo->ring.rsp_cons);
		goto err;
	}

	for (i = rinfo->ring.rsp_cons; i != rp; i++) {
		unsigned long id;
		unsigned int op;

		eoiflag = 0;

		RING_COPY_RESPONSE(&rinfo->ring, i, &bret);
		id = bret.id;

		/*
		 * The backend has messed up and given us an id that we would
		 * never have given to it (we stamp it up to BLK_RING_SIZE -
		 * look in get_id_from_freelist.
		 */
		if (id >= BLK_RING_SIZE(info)) {
			pr_alert("%s: response has incorrect id (%ld)\n",
				 info->gd->disk_name, id);
			goto err;
		}
		if (rinfo->shadow[id].status != REQ_WAITING) {
			pr_alert("%s: response references no pending request\n",
				 info->gd->disk_name);
			goto err;
		}

		rinfo->shadow[id].status = REQ_PROCESSING;
		req  = rinfo->shadow[id].request;

		op = rinfo->shadow[id].req.operation;
		if (op == BLKIF_OP_INDIRECT)
			op = rinfo->shadow[id].req.u.indirect.indirect_op;
		if (bret.operation != op) {
			pr_alert("%s: response has wrong operation (%u instead of %u)\n",
				 info->gd->disk_name, bret.operation, op);
			goto err;
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
			     info->gd->disk_name, op_name(bret.operation), id);
			continue;
		}

		if (bret.status == BLKIF_RSP_OKAY)
			blkif_req(req)->error = BLK_STS_OK;
		else
			blkif_req(req)->error = BLK_STS_IOERR;

		switch (bret.operation) {
		case BLKIF_OP_DISCARD:
			if (unlikely(bret.status == BLKIF_RSP_EOPNOTSUPP)) {
				struct request_queue *rq = info->rq;

				pr_warn_ratelimited("blkfront: %s: %s op failed\n",
					   info->gd->disk_name, op_name(bret.operation));
				blkif_req(req)->error = BLK_STS_NOTSUPP;
				info->feature_discard = 0;
				info->feature_secdiscard = 0;
				blk_queue_flag_clear(QUEUE_FLAG_DISCARD, rq);
				blk_queue_flag_clear(QUEUE_FLAG_SECERASE, rq);
			}
			break;
		case BLKIF_OP_FLUSH_DISKCACHE:
		case BLKIF_OP_WRITE_BARRIER:
			if (unlikely(bret.status == BLKIF_RSP_EOPNOTSUPP)) {
				pr_warn_ratelimited("blkfront: %s: %s op failed\n",
				       info->gd->disk_name, op_name(bret.operation));
				blkif_req(req)->error = BLK_STS_NOTSUPP;
			}
			if (unlikely(bret.status == BLKIF_RSP_ERROR &&
				     rinfo->shadow[id].req.u.rw.nr_segments == 0)) {
				pr_warn_ratelimited("blkfront: %s: empty %s op failed\n",
				       info->gd->disk_name, op_name(bret.operation));
				blkif_req(req)->error = BLK_STS_NOTSUPP;
			}
			if (unlikely(blkif_req(req)->error)) {
				if (blkif_req(req)->error == BLK_STS_NOTSUPP)
					blkif_req(req)->error = BLK_STS_OK;
				info->feature_fua = 0;
				info->feature_flush = 0;
				xlvbd_flush(info);
			}
			fallthrough;
		case BLKIF_OP_READ:
		case BLKIF_OP_WRITE:
			if (unlikely(bret.status != BLKIF_RSP_OKAY))
				dev_dbg_ratelimited(&info->xbdev->dev,
					"Bad return from blkdev data request: %#x\n",
					bret.status);

			break;
		default:
			BUG();
		}

		if (likely(!blk_should_fake_timeout(req->q)))
			blk_mq_complete_request(req);
	}

	rinfo->ring.rsp_cons = i;

	if (i != rinfo->ring.req_prod_pvt) {
		int more_to_do;
		RING_FINAL_CHECK_FOR_RESPONSES(&rinfo->ring, more_to_do);
		if (more_to_do)
			goto again;
	} else
		rinfo->ring.sring->rsp_event = i + 1;

	kick_pending_request_queues_locked(rinfo);

	spin_unlock_irqrestore(&rinfo->ring_lock, flags);

	xen_irq_lateeoi(irq, eoiflag);

	return IRQ_HANDLED;

 err:
	info->connected = BLKIF_STATE_ERROR;

	spin_unlock_irqrestore(&rinfo->ring_lock, flags);

	/* No EOI in order to avoid further interrupts. */

	pr_alert("%s disabled for further use\n", info->gd->disk_name);
	return IRQ_HANDLED;
}


static int setup_blkring(struct xenbus_device *dev,
			 struct blkfront_ring_info *rinfo)
{
	struct blkif_sring *sring;
	int err, i;
	struct blkfront_info *info = rinfo->dev_info;
	unsigned long ring_size = info->nr_ring_pages * XEN_PAGE_SIZE;
	grant_ref_t gref[XENBUS_MAX_RING_GRANTS];

	for (i = 0; i < info->nr_ring_pages; i++)
		rinfo->ring_ref[i] = GRANT_INVALID_REF;

	sring = alloc_pages_exact(ring_size, GFP_NOIO);
	if (!sring) {
		xenbus_dev_fatal(dev, -ENOMEM, "allocating shared ring");
		return -ENOMEM;
	}
	SHARED_RING_INIT(sring);
	FRONT_RING_INIT(&rinfo->ring, sring, ring_size);

	err = xenbus_grant_ring(dev, rinfo->ring.sring, info->nr_ring_pages, gref);
	if (err < 0) {
		free_pages_exact(sring, ring_size);
		rinfo->ring.sring = NULL;
		goto fail;
	}
	for (i = 0; i < info->nr_ring_pages; i++)
		rinfo->ring_ref[i] = gref[i];

	err = xenbus_alloc_evtchn(dev, &rinfo->evtchn);
	if (err)
		goto fail;

	err = bind_evtchn_to_irqhandler_lateeoi(rinfo->evtchn, blkif_interrupt,
						0, "blkif", rinfo);
	if (err <= 0) {
		xenbus_dev_fatal(dev, err,
				 "bind_evtchn_to_irqhandler failed");
		goto fail;
	}
	rinfo->irq = err;

	return 0;
fail:
	blkif_free(info, 0);
	return err;
}

/*
 * Write out per-ring/queue nodes including ring-ref and event-channel, and each
 * ring buffer may have multi pages depending on ->nr_ring_pages.
 */
static int write_per_ring_nodes(struct xenbus_transaction xbt,
				struct blkfront_ring_info *rinfo, const char *dir)
{
	int err;
	unsigned int i;
	const char *message = NULL;
	struct blkfront_info *info = rinfo->dev_info;

	if (info->nr_ring_pages == 1) {
		err = xenbus_printf(xbt, dir, "ring-ref", "%u", rinfo->ring_ref[0]);
		if (err) {
			message = "writing ring-ref";
			goto abort_transaction;
		}
	} else {
		for (i = 0; i < info->nr_ring_pages; i++) {
			char ring_ref_name[RINGREF_NAME_LEN];

			snprintf(ring_ref_name, RINGREF_NAME_LEN, "ring-ref%u", i);
			err = xenbus_printf(xbt, dir, ring_ref_name,
					    "%u", rinfo->ring_ref[i]);
			if (err) {
				message = "writing ring-ref";
				goto abort_transaction;
			}
		}
	}

	err = xenbus_printf(xbt, dir, "event-channel", "%u", rinfo->evtchn);
	if (err) {
		message = "writing event-channel";
		goto abort_transaction;
	}

	return 0;

abort_transaction:
	xenbus_transaction_end(xbt, 1);
	if (message)
		xenbus_dev_fatal(info->xbdev, err, "%s", message);

	return err;
}

/* Common code used when first setting up, and when resuming. */
static int talk_to_blkback(struct xenbus_device *dev,
			   struct blkfront_info *info)
{
	const char *message = NULL;
	struct xenbus_transaction xbt;
	int err;
	unsigned int i, max_page_order;
	unsigned int ring_page_order;
	struct blkfront_ring_info *rinfo;

	if (!info)
		return -ENODEV;

	max_page_order = xenbus_read_unsigned(info->xbdev->otherend,
					      "max-ring-page-order", 0);
	ring_page_order = min(xen_blkif_max_ring_order, max_page_order);
	info->nr_ring_pages = 1 << ring_page_order;

	err = negotiate_mq(info);
	if (err)
		goto destroy_blkring;

	for_each_rinfo(info, rinfo, i) {
		/* Create shared ring, alloc event channel. */
		err = setup_blkring(dev, rinfo);
		if (err)
			goto destroy_blkring;
	}

again:
	err = xenbus_transaction_start(&xbt);
	if (err) {
		xenbus_dev_fatal(dev, err, "starting transaction");
		goto destroy_blkring;
	}

	if (info->nr_ring_pages > 1) {
		err = xenbus_printf(xbt, dev->nodename, "ring-page-order", "%u",
				    ring_page_order);
		if (err) {
			message = "writing ring-page-order";
			goto abort_transaction;
		}
	}

	/* We already got the number of queues/rings in _probe */
	if (info->nr_rings == 1) {
		err = write_per_ring_nodes(xbt, info->rinfo, dev->nodename);
		if (err)
			goto destroy_blkring;
	} else {
		char *path;
		size_t pathsize;

		err = xenbus_printf(xbt, dev->nodename, "multi-queue-num-queues", "%u",
				    info->nr_rings);
		if (err) {
			message = "writing multi-queue-num-queues";
			goto abort_transaction;
		}

		pathsize = strlen(dev->nodename) + QUEUE_NAME_LEN;
		path = kmalloc(pathsize, GFP_KERNEL);
		if (!path) {
			err = -ENOMEM;
			message = "ENOMEM while writing ring references";
			goto abort_transaction;
		}

		for_each_rinfo(info, rinfo, i) {
			memset(path, 0, pathsize);
			snprintf(path, pathsize, "%s/queue-%u", dev->nodename, i);
			err = write_per_ring_nodes(xbt, rinfo, path);
			if (err) {
				kfree(path);
				goto destroy_blkring;
			}
		}
		kfree(path);
	}
	err = xenbus_printf(xbt, dev->nodename, "protocol", "%s",
			    XEN_IO_PROTO_ABI_NATIVE);
	if (err) {
		message = "writing protocol";
		goto abort_transaction;
	}
	err = xenbus_printf(xbt, dev->nodename, "feature-persistent", "%u",
			info->feature_persistent);
	if (err)
		dev_warn(&dev->dev,
			 "writing persistent grants feature to xenbus");

	err = xenbus_transaction_end(xbt, 0);
	if (err) {
		if (err == -EAGAIN)
			goto again;
		xenbus_dev_fatal(dev, err, "completing transaction");
		goto destroy_blkring;
	}

	for_each_rinfo(info, rinfo, i) {
		unsigned int j;

		for (j = 0; j < BLK_RING_SIZE(info); j++)
			rinfo->shadow[j].req.u.rw.id = j + 1;
		rinfo->shadow[BLK_RING_SIZE(info)-1].req.u.rw.id = 0x0fffffff;
	}
	xenbus_switch_state(dev, XenbusStateInitialised);

	return 0;

 abort_transaction:
	xenbus_transaction_end(xbt, 1);
	if (message)
		xenbus_dev_fatal(dev, err, "%s", message);
 destroy_blkring:
	blkif_free(info, 0);
	return err;
}

static int negotiate_mq(struct blkfront_info *info)
{
	unsigned int backend_max_queues;
	unsigned int i;
	struct blkfront_ring_info *rinfo;

	BUG_ON(info->nr_rings);

	/* Check if backend supports multiple queues. */
	backend_max_queues = xenbus_read_unsigned(info->xbdev->otherend,
						  "multi-queue-max-queues", 1);
	info->nr_rings = min(backend_max_queues, xen_blkif_max_queues);
	/* We need at least one ring. */
	if (!info->nr_rings)
		info->nr_rings = 1;

	info->rinfo_size = struct_size(info->rinfo, shadow,
				       BLK_RING_SIZE(info));
	info->rinfo = kvcalloc(info->nr_rings, info->rinfo_size, GFP_KERNEL);
	if (!info->rinfo) {
		xenbus_dev_fatal(info->xbdev, -ENOMEM, "allocating ring_info structure");
		info->nr_rings = 0;
		return -ENOMEM;
	}

	for_each_rinfo(info, rinfo, i) {
		INIT_LIST_HEAD(&rinfo->indirect_pages);
		INIT_LIST_HEAD(&rinfo->grants);
		rinfo->dev_info = info;
		INIT_WORK(&rinfo->work, blkif_restart_queue);
		spin_lock_init(&rinfo->ring_lock);
	}
	return 0;
}

/* Enable the persistent grants feature. */
static bool feature_persistent = true;
module_param(feature_persistent, bool, 0644);
MODULE_PARM_DESC(feature_persistent,
		"Enables the persistent grants feature");

/*
 * Entry point to this code when a new device is created.  Allocate the basic
 * structures and the ring buffer for communication with the backend, and
 * inform the backend of the appropriate details for those.  Switch to
 * Initialised state.
 */
static int blkfront_probe(struct xenbus_device *dev,
			  const struct xenbus_device_id *id)
{
	int err, vdevice;
	struct blkfront_info *info;

	/* FIXME: Use dynamic device id if this is not set. */
	err = xenbus_scanf(XBT_NIL, dev->nodename,
			   "virtual-device", "%i", &vdevice);
	if (err != 1) {
		/* go looking in the extended area instead */
		err = xenbus_scanf(XBT_NIL, dev->nodename, "virtual-device-ext",
				   "%i", &vdevice);
		if (err != 1) {
			xenbus_dev_fatal(dev, err, "reading virtual-device");
			return err;
		}
	}

	if (xen_hvm_domain()) {
		char *type;
		int len;
		/* no unplug has been done: do not hook devices != xen vbds */
		if (xen_has_pv_and_legacy_disk_devices()) {
			int major;

			if (!VDEV_IS_EXTENDED(vdevice))
				major = BLKIF_MAJOR(vdevice);
			else
				major = XENVBD_MAJOR;

			if (major != XENVBD_MAJOR) {
				printk(KERN_INFO
						"%s: HVM does not support vbd %d as xen block device\n",
						__func__, vdevice);
				return -ENODEV;
			}
		}
		/* do not create a PV cdrom device if we are an HVM guest */
		type = xenbus_read(XBT_NIL, dev->nodename, "device-type", &len);
		if (IS_ERR(type))
			return -ENODEV;
		if (strncmp(type, "cdrom", 5) == 0) {
			kfree(type);
			return -ENODEV;
		}
		kfree(type);
	}
	info = kzalloc(sizeof(*info), GFP_KERNEL);
	if (!info) {
		xenbus_dev_fatal(dev, -ENOMEM, "allocating info structure");
		return -ENOMEM;
	}

	info->xbdev = dev;

	mutex_init(&info->mutex);
	info->vdevice = vdevice;
	info->connected = BLKIF_STATE_DISCONNECTED;

	info->feature_persistent = feature_persistent;

	/* Front end dir is a number, which is used as the id. */
	info->handle = simple_strtoul(strrchr(dev->nodename, '/')+1, NULL, 0);
	dev_set_drvdata(&dev->dev, info);

	mutex_lock(&blkfront_mutex);
	list_add(&info->info_list, &info_list);
	mutex_unlock(&blkfront_mutex);

	return 0;
}

static int blkif_recover(struct blkfront_info *info)
{
	unsigned int r_index;
	struct request *req, *n;
	int rc;
	struct bio *bio;
	unsigned int segs;
	struct blkfront_ring_info *rinfo;

	blkfront_gather_backend_features(info);
	/* Reset limits changed by blk_mq_update_nr_hw_queues(). */
	blkif_set_queue_limits(info);
	segs = info->max_indirect_segments ? : BLKIF_MAX_SEGMENTS_PER_REQUEST;
	blk_queue_max_segments(info->rq, segs / GRANTS_PER_PSEG);

	for_each_rinfo(info, rinfo, r_index) {
		rc = blkfront_setup_indirect(rinfo);
		if (rc)
			return rc;
	}
	xenbus_switch_state(info->xbdev, XenbusStateConnected);

	/* Now safe for us to use the shared ring */
	info->connected = BLKIF_STATE_CONNECTED;

	for_each_rinfo(info, rinfo, r_index) {
		/* Kick any other new requests queued since we resumed */
		kick_pending_request_queues(rinfo);
	}

	list_for_each_entry_safe(req, n, &info->requests, queuelist) {
		/* Requeue pending requests (flush or discard) */
		list_del_init(&req->queuelist);
		BUG_ON(req->nr_phys_segments > segs);
		blk_mq_requeue_request(req, false);
	}
	blk_mq_start_stopped_hw_queues(info->rq, true);
	blk_mq_kick_requeue_list(info->rq);

	while ((bio = bio_list_pop(&info->bio_list)) != NULL) {
		/* Traverse the list of pending bios and re-queue them */
		submit_bio(bio);
	}

	return 0;
}

/*
 * We are reconnecting to the backend, due to a suspend/resume, or a backend
 * driver restart.  We tear down our blkif structure and recreate it, but
 * leave the device-layer structures intact so that this is transparent to the
 * rest of the kernel.
 */
static int blkfront_resume(struct xenbus_device *dev)
{
	struct blkfront_info *info = dev_get_drvdata(&dev->dev);
	int err = 0;
	unsigned int i, j;
	struct blkfront_ring_info *rinfo;

	dev_dbg(&dev->dev, "blkfront_resume: %s\n", dev->nodename);

	bio_list_init(&info->bio_list);
	INIT_LIST_HEAD(&info->requests);
	for_each_rinfo(info, rinfo, i) {
		struct bio_list merge_bio;
		struct blk_shadow *shadow = rinfo->shadow;

		for (j = 0; j < BLK_RING_SIZE(info); j++) {
			/* Not in use? */
			if (!shadow[j].request)
				continue;

			/*
			 * Get the bios in the request so we can re-queue them.
			 */
			if (req_op(shadow[j].request) == REQ_OP_FLUSH ||
			    req_op(shadow[j].request) == REQ_OP_DISCARD ||
			    req_op(shadow[j].request) == REQ_OP_SECURE_ERASE ||
			    shadow[j].request->cmd_flags & REQ_FUA) {
				/*
				 * Flush operations don't contain bios, so
				 * we need to requeue the whole request
				 *
				 * XXX: but this doesn't make any sense for a
				 * write with the FUA flag set..
				 */
				list_add(&shadow[j].request->queuelist, &info->requests);
				continue;
			}
			merge_bio.head = shadow[j].request->bio;
			merge_bio.tail = shadow[j].request->biotail;
			bio_list_merge(&info->bio_list, &merge_bio);
			shadow[j].request->bio = NULL;
			blk_mq_end_request(shadow[j].request, BLK_STS_OK);
		}
	}

	blkif_free(info, info->connected == BLKIF_STATE_CONNECTED);

	err = talk_to_blkback(dev, info);
	if (!err)
		blk_mq_update_nr_hw_queues(&info->tag_set, info->nr_rings);

	/*
	 * We have to wait for the backend to switch to
	 * connected state, since we want to read which
	 * features it supports.
	 */

	return err;
}

static void blkfront_closing(struct blkfront_info *info)
{
	struct xenbus_device *xbdev = info->xbdev;
	struct blkfront_ring_info *rinfo;
	unsigned int i;

	if (xbdev->state == XenbusStateClosing)
		return;

	/* No more blkif_request(). */
	blk_mq_stop_hw_queues(info->rq);
	blk_mark_disk_dead(info->gd);
	set_capacity(info->gd, 0);

	for_each_rinfo(info, rinfo, i) {
		/* No more gnttab callback work. */
		gnttab_cancel_free_callback(&rinfo->callback);

		/* Flush gnttab callback work. Must be done with no locks held. */
		flush_work(&rinfo->work);
	}

	xenbus_frontend_closed(xbdev);
}

static void blkfront_setup_discard(struct blkfront_info *info)
{
	info->feature_discard = 1;
	info->discard_granularity = xenbus_read_unsigned(info->xbdev->otherend,
							 "discard-granularity",
							 0);
	info->discard_alignment = xenbus_read_unsigned(info->xbdev->otherend,
						       "discard-alignment", 0);
	info->feature_secdiscard =
		!!xenbus_read_unsigned(info->xbdev->otherend, "discard-secure",
				       0);
}

static int blkfront_setup_indirect(struct blkfront_ring_info *rinfo)
{
	unsigned int psegs, grants, memflags;
	int err, i;
	struct blkfront_info *info = rinfo->dev_info;

	memflags = memalloc_noio_save();

	if (info->max_indirect_segments == 0) {
		if (!HAS_EXTRA_REQ)
			grants = BLKIF_MAX_SEGMENTS_PER_REQUEST;
		else {
			/*
			 * When an extra req is required, the maximum
			 * grants supported is related to the size of the
			 * Linux block segment.
			 */
			grants = GRANTS_PER_PSEG;
		}
	}
	else
		grants = info->max_indirect_segments;
	psegs = DIV_ROUND_UP(grants, GRANTS_PER_PSEG);

	err = fill_grant_buffer(rinfo,
				(grants + INDIRECT_GREFS(grants)) * BLK_RING_SIZE(info));
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
	}

	for (i = 0; i < BLK_RING_SIZE(info); i++) {
		rinfo->shadow[i].grants_used =
			kvcalloc(grants,
				 sizeof(rinfo->shadow[i].grants_used[0]),
				 GFP_KERNEL);
		rinfo->shadow[i].sg = kvcalloc(psegs,
					       sizeof(rinfo->shadow[i].sg[0]),
					       GFP_KERNEL);
		if (info->max_indirect_segments)
			rinfo->shadow[i].indirect_grants =
				kvcalloc(INDIRECT_GREFS(grants),
					 sizeof(rinfo->shadow[i].indirect_grants[0]),
					 GFP_KERNEL);
		if ((rinfo->shadow[i].grants_used == NULL) ||
			(rinfo->shadow[i].sg == NULL) ||
		     (info->max_indirect_segments &&
		     (rinfo->shadow[i].indirect_grants == NULL)))
			goto out_of_memory;
		sg_init_table(rinfo->shadow[i].sg, psegs);
	}

	memalloc_noio_restore(memflags);

	return 0;

out_of_memory:
	for (i = 0; i < BLK_RING_SIZE(info); i++) {
		kvfree(rinfo->shadow[i].grants_used);
		rinfo->shadow[i].grants_used = NULL;
		kvfree(rinfo->shadow[i].sg);
		rinfo->shadow[i].sg = NULL;
		kvfree(rinfo->shadow[i].indirect_grants);
		rinfo->shadow[i].indirect_grants = NULL;
	}
	if (!list_empty(&rinfo->indirect_pages)) {
		struct page *indirect_page, *n;
		list_for_each_entry_safe(indirect_page, n, &rinfo->indirect_pages, lru) {
			list_del(&indirect_page->lru);
			__free_page(indirect_page);
		}
	}

	memalloc_noio_restore(memflags);

	return -ENOMEM;
}

/*
 * Gather all backend feature-*
 */
static void blkfront_gather_backend_features(struct blkfront_info *info)
{
	unsigned int indirect_segments;

	info->feature_flush = 0;
	info->feature_fua = 0;

	/*
	 * If there's no "feature-barrier" defined, then it means
	 * we're dealing with a very old backend which writes
	 * synchronously; nothing to do.
	 *
	 * If there are barriers, then we use flush.
	 */
	if (xenbus_read_unsigned(info->xbdev->otherend, "feature-barrier", 0)) {
		info->feature_flush = 1;
		info->feature_fua = 1;
	}

	/*
	 * And if there is "feature-flush-cache" use that above
	 * barriers.
	 */
	if (xenbus_read_unsigned(info->xbdev->otherend, "feature-flush-cache",
				 0)) {
		info->feature_flush = 1;
		info->feature_fua = 0;
	}

	if (xenbus_read_unsigned(info->xbdev->otherend, "feature-discard", 0))
		blkfront_setup_discard(info);

	if (info->feature_persistent)
		info->feature_persistent =
			!!xenbus_read_unsigned(info->xbdev->otherend,
					       "feature-persistent", 0);

	indirect_segments = xenbus_read_unsigned(info->xbdev->otherend,
					"feature-max-indirect-segments", 0);
	if (indirect_segments > xen_blkif_max_segments)
		indirect_segments = xen_blkif_max_segments;
	if (indirect_segments <= BLKIF_MAX_SEGMENTS_PER_REQUEST)
		indirect_segments = 0;
	info->max_indirect_segments = indirect_segments;

	if (info->feature_persistent) {
		mutex_lock(&blkfront_mutex);
		schedule_delayed_work(&blkfront_work, HZ * 10);
		mutex_unlock(&blkfront_mutex);
	}
}

/*
 * Invoked when the backend is finally 'ready' (and has told produced
 * the details about the physical device - #sectors, size, etc).
 */
static void blkfront_connect(struct blkfront_info *info)
{
	unsigned long long sectors;
	unsigned long sector_size;
	unsigned int physical_sector_size;
	int err, i;
	struct blkfront_ring_info *rinfo;

	switch (info->connected) {
	case BLKIF_STATE_CONNECTED:
		/*
		 * Potentially, the back-end may be signalling
		 * a capacity change; update the capacity.
		 */
		err = xenbus_scanf(XBT_NIL, info->xbdev->otherend,
				   "sectors", "%Lu", &sectors);
		if (XENBUS_EXIST_ERR(err))
			return;
		printk(KERN_INFO "Setting capacity to %Lu\n",
		       sectors);
		set_capacity_and_notify(info->gd, sectors);

		return;
	case BLKIF_STATE_SUSPENDED:
		/*
		 * If we are recovering from suspension, we need to wait
		 * for the backend to announce it's features before
		 * reconnecting, at least we need to know if the backend
		 * supports indirect descriptors, and how many.
		 */
		blkif_recover(info);
		return;

	default:
		break;
	}

	dev_dbg(&info->xbdev->dev, "%s:%s.\n",
		__func__, info->xbdev->otherend);

	err = xenbus_gather(XBT_NIL, info->xbdev->otherend,
			    "sectors", "%llu", &sectors,
			    "info", "%u", &info->vdisk_info,
			    "sector-size", "%lu", &sector_size,
			    NULL);
	if (err) {
		xenbus_dev_fatal(info->xbdev, err,
				 "reading backend fields at %s",
				 info->xbdev->otherend);
		return;
	}

	/*
	 * physical-sector-size is a newer field, so old backends may not
	 * provide this. Assume physical sector size to be the same as
	 * sector_size in that case.
	 */
	physical_sector_size = xenbus_read_unsigned(info->xbdev->otherend,
						    "physical-sector-size",
						    sector_size);
	blkfront_gather_backend_features(info);
	for_each_rinfo(info, rinfo, i) {
		err = blkfront_setup_indirect(rinfo);
		if (err) {
			xenbus_dev_fatal(info->xbdev, err, "setup_indirect at %s",
					 info->xbdev->otherend);
			blkif_free(info, 0);
			break;
		}
	}

	err = xlvbd_alloc_gendisk(sectors, info, sector_size,
				  physical_sector_size);
	if (err) {
		xenbus_dev_fatal(info->xbdev, err, "xlvbd_add at %s",
				 info->xbdev->otherend);
		goto fail;
	}

	xenbus_switch_state(info->xbdev, XenbusStateConnected);

	/* Kick pending requests. */
	info->connected = BLKIF_STATE_CONNECTED;
	for_each_rinfo(info, rinfo, i)
		kick_pending_request_queues(rinfo);

	err = device_add_disk(&info->xbdev->dev, info->gd, NULL);
	if (err) {
		blk_cleanup_disk(info->gd);
		blk_mq_free_tag_set(&info->tag_set);
		info->rq = NULL;
		goto fail;
	}

	info->is_ready = 1;
	return;

fail:
	blkif_free(info, 0);
	return;
}

/*
 * Callback received when the backend's state changes.
 */
static void blkback_changed(struct xenbus_device *dev,
			    enum xenbus_state backend_state)
{
	struct blkfront_info *info = dev_get_drvdata(&dev->dev);

	dev_dbg(&dev->dev, "blkfront:blkback_changed to state %d.\n", backend_state);

	switch (backend_state) {
	case XenbusStateInitWait:
		if (dev->state != XenbusStateInitialising)
			break;
		if (talk_to_blkback(dev, info))
			break;
		break;
	case XenbusStateInitialising:
	case XenbusStateInitialised:
	case XenbusStateReconfiguring:
	case XenbusStateReconfigured:
	case XenbusStateUnknown:
		break;

	case XenbusStateConnected:
		/*
		 * talk_to_blkback sets state to XenbusStateInitialised
		 * and blkfront_connect sets it to XenbusStateConnected
		 * (if connection went OK).
		 *
		 * If the backend (or toolstack) decides to poke at backend
		 * state (and re-trigger the watch by setting the state repeatedly
		 * to XenbusStateConnected (4)) we need to deal with this.
		 * This is allowed as this is used to communicate to the guest
		 * that the size of disk has changed!
		 */
		if ((dev->state != XenbusStateInitialised) &&
		    (dev->state != XenbusStateConnected)) {
			if (talk_to_blkback(dev, info))
				break;
		}

		blkfront_connect(info);
		break;

	case XenbusStateClosed:
		if (dev->state == XenbusStateClosed)
			break;
		fallthrough;
	case XenbusStateClosing:
		blkfront_closing(info);
		break;
	}
}

static int blkfront_remove(struct xenbus_device *xbdev)
{
	struct blkfront_info *info = dev_get_drvdata(&xbdev->dev);

	dev_dbg(&xbdev->dev, "%s removed", xbdev->nodename);

	del_gendisk(info->gd);

	mutex_lock(&blkfront_mutex);
	list_del(&info->info_list);
	mutex_unlock(&blkfront_mutex);

	blkif_free(info, 0);
	xlbd_release_minors(info->gd->first_minor, info->gd->minors);
	blk_cleanup_disk(info->gd);
	blk_mq_free_tag_set(&info->tag_set);

	kfree(info);
	return 0;
}

static int blkfront_is_ready(struct xenbus_device *dev)
{
	struct blkfront_info *info = dev_get_drvdata(&dev->dev);

	return info->is_ready && info->xbdev;
}

static const struct block_device_operations xlvbd_block_fops =
{
	.owner = THIS_MODULE,
	.getgeo = blkif_getgeo,
	.ioctl = blkif_ioctl,
	.compat_ioctl = blkdev_compat_ptr_ioctl,
};


static const struct xenbus_device_id blkfront_ids[] = {
	{ "vbd" },
	{ "" }
};

static struct xenbus_driver blkfront_driver = {
	.ids  = blkfront_ids,
	.probe = blkfront_probe,
	.remove = blkfront_remove,
	.resume = blkfront_resume,
	.otherend_changed = blkback_changed,
	.is_ready = blkfront_is_ready,
};

static void purge_persistent_grants(struct blkfront_info *info)
{
	unsigned int i;
	unsigned long flags;
	struct blkfront_ring_info *rinfo;

	for_each_rinfo(info, rinfo, i) {
		struct grant *gnt_list_entry, *tmp;

		spin_lock_irqsave(&rinfo->ring_lock, flags);

		if (rinfo->persistent_gnts_c == 0) {
			spin_unlock_irqrestore(&rinfo->ring_lock, flags);
			continue;
		}

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

		spin_unlock_irqrestore(&rinfo->ring_lock, flags);
	}
}

static void blkfront_delay_work(struct work_struct *work)
{
	struct blkfront_info *info;
	bool need_schedule_work = false;

	mutex_lock(&blkfront_mutex);

	list_for_each_entry(info, &info_list, info_list) {
		if (info->feature_persistent) {
			need_schedule_work = true;
			mutex_lock(&info->mutex);
			purge_persistent_grants(info);
			mutex_unlock(&info->mutex);
		}
	}

	if (need_schedule_work)
		schedule_delayed_work(&blkfront_work, HZ * 10);

	mutex_unlock(&blkfront_mutex);
}

static int __init xlblk_init(void)
{
	int ret;
	int nr_cpus = num_online_cpus();

	if (!xen_domain())
		return -ENODEV;

	if (!xen_has_pv_disk_devices())
		return -ENODEV;

	if (register_blkdev(XENVBD_MAJOR, DEV_NAME)) {
		pr_warn("xen_blk: can't get major %d with name %s\n",
			XENVBD_MAJOR, DEV_NAME);
		return -ENODEV;
	}

	if (xen_blkif_max_segments < BLKIF_MAX_SEGMENTS_PER_REQUEST)
		xen_blkif_max_segments = BLKIF_MAX_SEGMENTS_PER_REQUEST;

	if (xen_blkif_max_ring_order > XENBUS_MAX_RING_GRANT_ORDER) {
		pr_info("Invalid max_ring_order (%d), will use default max: %d.\n",
			xen_blkif_max_ring_order, XENBUS_MAX_RING_GRANT_ORDER);
		xen_blkif_max_ring_order = XENBUS_MAX_RING_GRANT_ORDER;
	}

	if (xen_blkif_max_queues > nr_cpus) {
		pr_info("Invalid max_queues (%d), will use default max: %d.\n",
			xen_blkif_max_queues, nr_cpus);
		xen_blkif_max_queues = nr_cpus;
	}

	INIT_DELAYED_WORK(&blkfront_work, blkfront_delay_work);

	ret = xenbus_register_frontend(&blkfront_driver);
	if (ret) {
		unregister_blkdev(XENVBD_MAJOR, DEV_NAME);
		return ret;
	}

	return 0;
}
module_init(xlblk_init);


static void __exit xlblk_exit(void)
{
	cancel_delayed_work_sync(&blkfront_work);

	xenbus_unregister_driver(&blkfront_driver);
	unregister_blkdev(XENVBD_MAJOR, DEV_NAME);
	kfree(minors);
}
module_exit(xlblk_exit);

MODULE_DESCRIPTION("Xen virtual block device frontend");
MODULE_LICENSE("GPL");
MODULE_ALIAS_BLOCKDEV_MAJOR(XENVBD_MAJOR);
MODULE_ALIAS("xen:vbd");
MODULE_ALIAS("xenblk");
				if (!info->feature_persistent) {
					indirect_page = s->indirect_grants[i]->page;
					list_add(&indirect_page->lru, &rinfo->indirect_pages);
				}
				s->indirect_grants[i]->gref = GRANT_INVALID_REF;
				list_add_tail(&s->indirect_grants[i]->node, &rinfo->grants);
			}
		}
	}

	return 1;
}

static irqreturn_t blkif_interrupt(int irq, void *dev_id)
{
	struct request *req;
	struct blkif_response bret;
	RING_IDX i, rp;
	unsigned long flags;
	struct blkfront_ring_info *rinfo = (struct blkfront_ring_info *)dev_id;
	struct blkfront_info *info = rinfo->dev_info;
	unsigned int eoiflag = XEN_EOI_FLAG_SPURIOUS;

	if (unlikely(info->connected != BLKIF_STATE_CONNECTED)) {
		xen_irq_lateeoi(irq, XEN_EOI_FLAG_SPURIOUS);
		return IRQ_HANDLED;
	}

	spin_lock_irqsave(&rinfo->ring_lock, flags);
 again:
	rp = READ_ONCE(rinfo->ring.sring->rsp_prod);
	virt_rmb(); /* Ensure we see queued responses up to 'rp'. */
	if (RING_RESPONSE_PROD_OVERFLOW(&rinfo->ring, rp)) {
		pr_alert("%s: illegal number of responses %u\n",
			 info->gd->disk_name, rp - rinfo->ring.rsp_cons);
		goto err;
	}

	for (i = rinfo->ring.rsp_cons; i != rp; i++) {
		unsigned long id;
		unsigned int op;

		eoiflag = 0;

		RING_COPY_RESPONSE(&rinfo->ring, i, &bret);
		id = bret.id;

		/*
		 * The backend has messed up and given us an id that we would
		 * never have given to it (we stamp it up to BLK_RING_SIZE -
		 * look in get_id_from_freelist.
		 */
		if (id >= BLK_RING_SIZE(info)) {
			pr_alert("%s: response has incorrect id (%ld)\n",
				 info->gd->disk_name, id);
			goto err;
		}
		if (bret.operation != op) {
			pr_alert("%s: response has wrong operation (%u instead of %u)\n",
				 info->gd->disk_name, bret.operation, op);
			goto err;
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
{
	struct blkif_sring *sring;
	int err, i;
	struct blkfront_info *info = rinfo->dev_info;
	unsigned long ring_size = info->nr_ring_pages * XEN_PAGE_SIZE;
	grant_ref_t gref[XENBUS_MAX_RING_GRANTS];

	for (i = 0; i < info->nr_ring_pages; i++)
		rinfo->ring_ref[i] = GRANT_INVALID_REF;

	sring = alloc_pages_exact(ring_size, GFP_NOIO);
	if (!sring) {
		xenbus_dev_fatal(dev, -ENOMEM, "allocating shared ring");
		return -ENOMEM;
	}
	if (!sring) {
		xenbus_dev_fatal(dev, -ENOMEM, "allocating shared ring");
		return -ENOMEM;
	}
	SHARED_RING_INIT(sring);
	FRONT_RING_INIT(&rinfo->ring, sring, ring_size);

	err = xenbus_grant_ring(dev, rinfo->ring.sring, info->nr_ring_pages, gref);
	if (err < 0) {
		free_pages_exact(sring, ring_size);
		rinfo->ring.sring = NULL;
		goto fail;
	}
					 node) {
			if (gnt_list_entry->gref == GRANT_INVALID_REF ||
			    !gnttab_try_end_foreign_access(gnt_list_entry->gref))
				continue;

			list_del(&gnt_list_entry->node);
			rinfo->persistent_gnts_c--;
			gnt_list_entry->gref = GRANT_INVALID_REF;
			list_add_tail(&gnt_list_entry->node, &rinfo->grants);
		}
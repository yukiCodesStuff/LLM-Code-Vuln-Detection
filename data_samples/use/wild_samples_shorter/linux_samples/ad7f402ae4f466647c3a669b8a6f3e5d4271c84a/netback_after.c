

struct xenvif_tx_cb {
	u16 copy_pending_idx[XEN_NETBK_LEGACY_SLOTS_MAX + 1];
	u8 copy_count;
};

#define XENVIF_TX_CB(skb) ((struct xenvif_tx_cb *)(skb)->cb)
#define copy_pending_idx(skb, i) (XENVIF_TX_CB(skb)->copy_pending_idx[i])
#define copy_count(skb) (XENVIF_TX_CB(skb)->copy_count)

static inline void xenvif_tx_create_map_op(struct xenvif_queue *queue,
					   u16 pending_idx,
					   struct xen_netif_tx_request *txp,
	return skb;
}

static void xenvif_get_requests(struct xenvif_queue *queue,
				struct sk_buff *skb,
				struct xen_netif_tx_request *first,
				struct xen_netif_tx_request *txfrags,
			        unsigned *copy_ops,
			        unsigned *map_ops,
				unsigned int frag_overflow,
				struct sk_buff *nskb,
				unsigned int extra_count,
				unsigned int data_len)
{
	struct skb_shared_info *shinfo = skb_shinfo(skb);
	skb_frag_t *frags = shinfo->frags;
	u16 pending_idx;
	pending_ring_idx_t index;
	unsigned int nr_slots;
	struct gnttab_copy *cop = queue->tx_copy_ops + *copy_ops;
	struct gnttab_map_grant_ref *gop = queue->tx_map_ops + *map_ops;
	struct xen_netif_tx_request *txp = first;

	nr_slots = shinfo->nr_frags + 1;

	copy_count(skb) = 0;

	/* Create copy ops for exactly data_len bytes into the skb head. */
	__skb_put(skb, data_len);
	while (data_len > 0) {
		int amount = data_len > txp->size ? txp->size : data_len;

		cop->source.u.ref = txp->gref;
		cop->source.domid = queue->vif->domid;
		cop->source.offset = txp->offset;

		cop->dest.domid = DOMID_SELF;
		cop->dest.offset = (offset_in_page(skb->data +
						   skb_headlen(skb) -
						   data_len)) & ~XEN_PAGE_MASK;
		cop->dest.u.gmfn = virt_to_gfn(skb->data + skb_headlen(skb)
				               - data_len);

		cop->len = amount;
		cop->flags = GNTCOPY_source_gref;

		index = pending_index(queue->pending_cons);
		pending_idx = queue->pending_ring[index];
		callback_param(queue, pending_idx).ctx = NULL;
		copy_pending_idx(skb, copy_count(skb)) = pending_idx;
		copy_count(skb)++;

		cop++;
		data_len -= amount;

		if (amount == txp->size) {
			/* The copy op covered the full tx_request */

			memcpy(&queue->pending_tx_info[pending_idx].req,
			       txp, sizeof(*txp));
			queue->pending_tx_info[pending_idx].extra_count =
				(txp == first) ? extra_count : 0;

			if (txp == first)
				txp = txfrags;
			else
				txp++;
			queue->pending_cons++;
			nr_slots--;
		} else {
			/* The copy op partially covered the tx_request.
			 * The remainder will be mapped.
			 */
			txp->offset += amount;
			txp->size -= amount;
		}
	}

	for (shinfo->nr_frags = 0; shinfo->nr_frags < nr_slots;
	     shinfo->nr_frags++, gop++) {
		index = pending_index(queue->pending_cons++);
		pending_idx = queue->pending_ring[index];
		xenvif_tx_create_map_op(queue, pending_idx, txp,
				        txp == first ? extra_count : 0, gop);
		frag_set_pending_idx(&frags[shinfo->nr_frags], pending_idx);

		if (txp == first)
			txp = txfrags;
		else
			txp++;
	}

	if (frag_overflow) {

		skb_shinfo(skb)->frag_list = nskb;
	}

	(*copy_ops) = cop - queue->tx_copy_ops;
	(*map_ops) = gop - queue->tx_map_ops;
}

static inline void xenvif_grant_handle_set(struct xenvif_queue *queue,
					   u16 pending_idx,
			       struct gnttab_copy **gopp_copy)
{
	struct gnttab_map_grant_ref *gop_map = *gopp_map;
	u16 pending_idx;
	/* This always points to the shinfo of the skb being checked, which
	 * could be either the first or the one on the frag_list
	 */
	struct skb_shared_info *shinfo = skb_shinfo(skb);
	struct skb_shared_info *first_shinfo = NULL;
	int nr_frags = shinfo->nr_frags;
	const bool sharedslot = nr_frags &&
				frag_get_pending_idx(&shinfo->frags[0]) ==
				    copy_pending_idx(skb, copy_count(skb) - 1);
	int i, err;

	for (i = 0; i < copy_count(skb); i++) {
		int newerr;

		/* Check status of header. */
		pending_idx = copy_pending_idx(skb, i);

		newerr = (*gopp_copy)->status;
		if (likely(!newerr)) {
			/* The first frag might still have this slot mapped */
			if (i < copy_count(skb) - 1 || !sharedslot)
				xenvif_idx_release(queue, pending_idx,
						   XEN_NETIF_RSP_OKAY);
		} else {
			err = newerr;
			if (net_ratelimit())
				netdev_dbg(queue->vif->dev,
					   "Grant copy of header failed! status: %d pending_idx: %u ref: %u\n",
					   (*gopp_copy)->status,
					   pending_idx,
					   (*gopp_copy)->source.u.ref);
			/* The first frag might still have this slot mapped */
			if (i < copy_count(skb) - 1 || !sharedslot)
				xenvif_idx_release(queue, pending_idx,
						   XEN_NETIF_RSP_ERROR);
		}
		(*gopp_copy)++;
	}

check_frags:
	for (i = 0; i < nr_frags; i++, gop_map++) {
		int j, newerr;
		if (err)
			continue;

		/* Invalidate preceding fragments of this skb. */
		for (j = 0; j < i; j++) {
			pending_idx = frag_get_pending_idx(&shinfo->frags[j]);
			xenvif_idx_unmap(queue, pending_idx);
				     unsigned *copy_ops,
				     unsigned *map_ops)
{
	struct sk_buff *skb, *nskb;
	int ret;
	unsigned int frag_overflow;

			continue;
		}

		data_len = (txreq.size > XEN_NETBACK_TX_COPY_LEN) ?
			XEN_NETBACK_TX_COPY_LEN : txreq.size;

		ret = xenvif_count_requests(queue, &txreq, extra_count,
					    txfrags, work_to_do);

		if (unlikely(ret < 0))
			break;

		idx += ret;
		index = pending_index(queue->pending_cons);
		pending_idx = queue->pending_ring[index];

		if (ret >= XEN_NETBK_LEGACY_SLOTS_MAX - 1 && data_len < txreq.size)
			data_len = txreq.size;

		skb = xenvif_alloc_skb(data_len);
		if (unlikely(skb == NULL)) {
			netdev_dbg(queue->vif->dev,
		}

		skb_shinfo(skb)->nr_frags = ret;
		/* At this point shinfo->nr_frags is in fact the number of
		 * slots, which can be as large as XEN_NETBK_LEGACY_SLOTS_MAX.
		 */
		frag_overflow = 0;
					     type);
		}

		xenvif_get_requests(queue, skb, &txreq, txfrags, copy_ops,
				    map_ops, frag_overflow, nskb, extra_count,
				    data_len);

		__skb_queue_tail(&queue->tx_queue, skb);

		queue->tx.req_cons = idx;

		if ((*map_ops >= ARRAY_SIZE(queue->tx_map_ops)) ||
		    (*copy_ops >= ARRAY_SIZE(queue->tx_copy_ops)))
			break;
	}

	return;
}

/* Consolidate skb with a frag_list into a brand new one with local pages on
	while ((skb = __skb_dequeue(&queue->tx_queue)) != NULL) {
		struct xen_netif_tx_request *txp;
		u16 pending_idx;

		pending_idx = copy_pending_idx(skb, 0);
		txp = &queue->pending_tx_info[pending_idx].req;

		/* Check the remap error code. */
		if (unlikely(xenvif_tx_check_gop(queue, skb, &gop_map, &gop_copy))) {
			continue;
		}

		if (txp->flags & XEN_NETTXF_csum_blank)
			skb->ip_summed = CHECKSUM_PARTIAL;
		else if (txp->flags & XEN_NETTXF_data_validated)
			skb->ip_summed = CHECKSUM_UNNECESSARY;
/* Called after netfront has transmitted */
int xenvif_tx_action(struct xenvif_queue *queue, int budget)
{
	unsigned nr_mops = 0, nr_cops = 0;
	int work_done, ret;

	if (unlikely(!tx_work_todo(queue)))
		return 0;
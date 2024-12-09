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
	XENVIF_TX_CB(skb)->split_mask = 0;

	/* Create copy ops for exactly data_len bytes into the skb head. */
	__skb_put(skb, data_len);
	while (data_len > 0) {
		int amount = data_len > txp->size ? txp->size : data_len;
		bool split = false;

		cop->source.u.ref = txp->gref;
		cop->source.domid = queue->vif->domid;
		cop->source.offset = txp->offset;

		cop->dest.domid = DOMID_SELF;
		cop->dest.offset = (offset_in_page(skb->data +
						   skb_headlen(skb) -
						   data_len)) & ~XEN_PAGE_MASK;
		cop->dest.u.gmfn = virt_to_gfn(skb->data + skb_headlen(skb)
				               - data_len);

		/* Don't cross local page boundary! */
		if (cop->dest.offset + amount > XEN_PAGE_SIZE) {
			amount = XEN_PAGE_SIZE - cop->dest.offset;
			XENVIF_TX_CB(skb)->split_mask |= 1U << copy_count(skb);
			split = true;
		}

		cop->len = amount;
		cop->flags = GNTCOPY_source_gref;

		index = pending_index(queue->pending_cons);
		pending_idx = queue->pending_ring[index];
		callback_param(queue, pending_idx).ctx = NULL;
		copy_pending_idx(skb, copy_count(skb)) = pending_idx;
		if (!split)
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
			 * The remainder will be mapped or copied in the next
			 * iteration.
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

		shinfo = skb_shinfo(nskb);
		frags = shinfo->frags;

		for (shinfo->nr_frags = 0; shinfo->nr_frags < frag_overflow;
		     shinfo->nr_frags++, txp++, gop++) {
			index = pending_index(queue->pending_cons++);
			pending_idx = queue->pending_ring[index];
			xenvif_tx_create_map_op(queue, pending_idx, txp, 0,
						gop);
			frag_set_pending_idx(&frags[shinfo->nr_frags],
					     pending_idx);
		}

		skb_shinfo(skb)->frag_list = nskb;
	}

	(*copy_ops) = cop - queue->tx_copy_ops;
	(*map_ops) = gop - queue->tx_map_ops;
}
		} else {
			/* The copy op partially covered the tx_request.
			 * The remainder will be mapped or copied in the next
			 * iteration.
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

		shinfo = skb_shinfo(nskb);
		frags = shinfo->frags;

		for (shinfo->nr_frags = 0; shinfo->nr_frags < frag_overflow;
		     shinfo->nr_frags++, txp++, gop++) {
			index = pending_index(queue->pending_cons++);
			pending_idx = queue->pending_ring[index];
			xenvif_tx_create_map_op(queue, pending_idx, txp, 0,
						gop);
			frag_set_pending_idx(&frags[shinfo->nr_frags],
					     pending_idx);
		}
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

		shinfo = skb_shinfo(nskb);
		frags = shinfo->frags;

		for (shinfo->nr_frags = 0; shinfo->nr_frags < frag_overflow;
		     shinfo->nr_frags++, txp++, gop++) {
			index = pending_index(queue->pending_cons++);
			pending_idx = queue->pending_ring[index];
			xenvif_tx_create_map_op(queue, pending_idx, txp, 0,
						gop);
			frag_set_pending_idx(&frags[shinfo->nr_frags],
					     pending_idx);
		}
		     shinfo->nr_frags++, txp++, gop++) {
			index = pending_index(queue->pending_cons++);
			pending_idx = queue->pending_ring[index];
			xenvif_tx_create_map_op(queue, pending_idx, txp, 0,
						gop);
			frag_set_pending_idx(&frags[shinfo->nr_frags],
					     pending_idx);
		}

		skb_shinfo(skb)->frag_list = nskb;
	}

	(*copy_ops) = cop - queue->tx_copy_ops;
	(*map_ops) = gop - queue->tx_map_ops;
}

static inline void xenvif_grant_handle_set(struct xenvif_queue *queue,
					   u16 pending_idx,
					   grant_handle_t handle)
{
	if (unlikely(queue->grant_tx_handle[pending_idx] !=
		     NETBACK_INVALID_HANDLE)) {
		netdev_err(queue->vif->dev,
			   "Trying to overwrite active handle! pending_idx: 0x%x\n",
			   pending_idx);
		BUG();
	}
	queue->grant_tx_handle[pending_idx] = handle;
}

static inline void xenvif_grant_handle_reset(struct xenvif_queue *queue,
					     u16 pending_idx)
{
	if (unlikely(queue->grant_tx_handle[pending_idx] ==
		     NETBACK_INVALID_HANDLE)) {
		netdev_err(queue->vif->dev,
			   "Trying to unmap invalid handle! pending_idx: 0x%x\n",
			   pending_idx);
		BUG();
	}
	queue->grant_tx_handle[pending_idx] = NETBACK_INVALID_HANDLE;
}

static int xenvif_tx_check_gop(struct xenvif_queue *queue,
			       struct sk_buff *skb,
			       struct gnttab_map_grant_ref **gopp_map,
			       struct gnttab_copy **gopp_copy)
{
	struct gnttab_map_grant_ref *gop_map = *gopp_map;
	u16 pending_idx;
	/* This always points to the shinfo of the skb being checked, which
	 * could be either the first or the one on the frag_list
	 */
	struct skb_shared_info *shinfo = skb_shinfo(skb);
	/* If this is non-NULL, we are currently checking the frag_list skb, and
	 * this points to the shinfo of the first one
	 */
	struct skb_shared_info *first_shinfo = NULL;
	int nr_frags = shinfo->nr_frags;
	const bool sharedslot = nr_frags &&
				frag_get_pending_idx(&shinfo->frags[0]) ==
				    copy_pending_idx(skb, copy_count(skb) - 1);
	int i, err = 0;

	for (i = 0; i < copy_count(skb); i++) {
		int newerr;

		/* Check status of header. */
		pending_idx = copy_pending_idx(skb, i);

		newerr = (*gopp_copy)->status;

		/* Split copies need to be handled together. */
		if (XENVIF_TX_CB(skb)->split_mask & (1U << i)) {
			(*gopp_copy)++;
			if (!newerr)
				newerr = (*gopp_copy)->status;
		}
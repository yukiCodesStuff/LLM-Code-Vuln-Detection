	atomic_dec(&netbk->netfront_count);
}

static void xen_netbk_idx_release(struct xen_netbk *netbk, u16 pending_idx,
				  u8 status);
static void make_tx_response(struct xenvif *vif,
			     struct xen_netif_tx_request *txp,
			     s8       st);
static struct xen_netif_rx_response *make_rx_response(struct xenvif *vif,

	do {
		make_tx_response(vif, txp, XEN_NETIF_RSP_ERROR);
		if (cons == end)
			break;
		txp = RING_GET_REQUEST(&vif->tx, cons++);
	} while (1);
	vif->tx.req_cons = cons;
	xenvif_put(vif);
}

static void netbk_fatal_tx_err(struct xenvif *vif)
{
	netdev_err(vif->dev, "fatal error; disabling device\n");
	xenvif_carrier_off(vif);
	xenvif_put(vif);
}

static int netbk_count_requests(struct xenvif *vif,
				struct xen_netif_tx_request *first,
				struct xen_netif_tx_request *txp,
				int work_to_do)

	do {
		if (frags >= work_to_do) {
			netdev_err(vif->dev, "Need more frags\n");
			netbk_fatal_tx_err(vif);
			return -frags;
		}

		if (unlikely(frags >= MAX_SKB_FRAGS)) {
			netdev_err(vif->dev, "Too many frags\n");
			netbk_fatal_tx_err(vif);
			return -frags;
		}

		memcpy(txp, RING_GET_REQUEST(&vif->tx, cons + frags),
		       sizeof(*txp));
		if (txp->size > first->size) {
			netdev_err(vif->dev, "Frag is bigger than frame.\n");
			netbk_fatal_tx_err(vif);
			return -frags;
		}

		first->size -= txp->size;
		frags++;

		if (unlikely((txp->offset + txp->size) > PAGE_SIZE)) {
			netdev_err(vif->dev, "txp->offset: %x, size: %u\n",
				 txp->offset, txp->size);
			netbk_fatal_tx_err(vif);
			return -frags;
		}
	} while ((txp++)->flags & XEN_NETTXF_more_data);
	return frags;
		pending_idx = netbk->pending_ring[index];
		page = xen_netbk_alloc_page(netbk, skb, pending_idx);
		if (!page)
			goto err;

		gop->source.u.ref = txp->gref;
		gop->source.domid = vif->domid;
		gop->source.offset = txp->offset;
	}

	return gop;
err:
	/* Unwind, freeing all pages and sending error responses. */
	while (i-- > start) {
		xen_netbk_idx_release(netbk, frag_get_pending_idx(&frags[i]),
				      XEN_NETIF_RSP_ERROR);
	}
	/* The head too, if necessary. */
	if (start)
		xen_netbk_idx_release(netbk, pending_idx, XEN_NETIF_RSP_ERROR);

	return NULL;
}

static int xen_netbk_tx_check_gop(struct xen_netbk *netbk,
				  struct sk_buff *skb,
{
	struct gnttab_copy *gop = *gopp;
	u16 pending_idx = *((u16 *)skb->data);
	struct skb_shared_info *shinfo = skb_shinfo(skb);
	int nr_frags = shinfo->nr_frags;
	int i, err, start;

	/* Check status of header. */
	err = gop->status;
	if (unlikely(err))
		xen_netbk_idx_release(netbk, pending_idx, XEN_NETIF_RSP_ERROR);

	/* Skip first skb fragment if it is on same page as header fragment. */
	start = (frag_get_pending_idx(&shinfo->frags[0]) == pending_idx);

	for (i = start; i < nr_frags; i++) {
		int j, newerr;

		pending_idx = frag_get_pending_idx(&shinfo->frags[i]);

		/* Check error status: if okay then remember grant handle. */
		if (likely(!newerr)) {
			/* Had a previous error? Invalidate this fragment. */
			if (unlikely(err))
				xen_netbk_idx_release(netbk, pending_idx, XEN_NETIF_RSP_OKAY);
			continue;
		}

		/* Error on this fragment: respond to client with an error. */
		xen_netbk_idx_release(netbk, pending_idx, XEN_NETIF_RSP_ERROR);

		/* Not the first error? Preceding frags already invalidated. */
		if (err)
			continue;

		/* First error: invalidate header and preceding fragments. */
		pending_idx = *((u16 *)skb->data);
		xen_netbk_idx_release(netbk, pending_idx, XEN_NETIF_RSP_OKAY);
		for (j = start; j < i; j++) {
			pending_idx = frag_get_pending_idx(&shinfo->frags[j]);
			xen_netbk_idx_release(netbk, pending_idx, XEN_NETIF_RSP_OKAY);
		}

		/* Remember the error: invalidate all subsequent fragments. */
		err = newerr;

		/* Take an extra reference to offset xen_netbk_idx_release */
		get_page(netbk->mmap_pages[pending_idx]);
		xen_netbk_idx_release(netbk, pending_idx, XEN_NETIF_RSP_OKAY);
	}
}

static int xen_netbk_get_extras(struct xenvif *vif,

	do {
		if (unlikely(work_to_do-- <= 0)) {
			netdev_err(vif->dev, "Missing extra info\n");
			netbk_fatal_tx_err(vif);
			return -EBADR;
		}

		memcpy(&extra, RING_GET_REQUEST(&vif->tx, cons),
		if (unlikely(!extra.type ||
			     extra.type >= XEN_NETIF_EXTRA_TYPE_MAX)) {
			vif->tx.req_cons = ++cons;
			netdev_err(vif->dev,
				   "Invalid extra type: %d\n", extra.type);
			netbk_fatal_tx_err(vif);
			return -EINVAL;
		}

		memcpy(&extras[extra.type - 1], &extra, sizeof(extra));
			     struct xen_netif_extra_info *gso)
{
	if (!gso->u.gso.size) {
		netdev_err(vif->dev, "GSO size must not be zero.\n");
		netbk_fatal_tx_err(vif);
		return -EINVAL;
	}

	/* Currently only TCPv4 S.O. is supported. */
	if (gso->u.gso.type != XEN_NETIF_GSO_TYPE_TCPV4) {
		netdev_err(vif->dev, "Bad GSO type %d.\n", gso->u.gso.type);
		netbk_fatal_tx_err(vif);
		return -EINVAL;
	}

	skb_shinfo(skb)->gso_size = gso->u.gso.size;

		/* Get a netif from the list with work to do. */
		vif = poll_net_schedule_list(netbk);
		/* This can sometimes happen because the test of
		 * list_empty(net_schedule_list) at the top of the
		 * loop is unlocked.  Just go back and have another
		 * look.
		 */
		if (!vif)
			continue;

		if (vif->tx.sring->req_prod - vif->tx.req_cons >
		    XEN_NETIF_TX_RING_SIZE) {
			netdev_err(vif->dev,
				   "Impossible number of requests. "
				   "req_prod %d, req_cons %d, size %ld\n",
				   vif->tx.sring->req_prod, vif->tx.req_cons,
				   XEN_NETIF_TX_RING_SIZE);
			netbk_fatal_tx_err(vif);
			continue;
		}

		RING_FINAL_CHECK_FOR_REQUESTS(&vif->tx, work_to_do);
		if (!work_to_do) {
			xenvif_put(vif);
			continue;
			work_to_do = xen_netbk_get_extras(vif, extras,
							  work_to_do);
			idx = vif->tx.req_cons;
			if (unlikely(work_to_do < 0))
				continue;
		}

		ret = netbk_count_requests(vif, &txreq, txfrags, work_to_do);
		if (unlikely(ret < 0))
			continue;

		idx += ret;

		if (unlikely(txreq.size < ETH_HLEN)) {
			netdev_dbg(vif->dev,

		/* No crossing a page as the payload mustn't fragment. */
		if (unlikely((txreq.offset + txreq.size) > PAGE_SIZE)) {
			netdev_err(vif->dev,
				   "txreq.offset: %x, size: %u, end: %lu\n",
				   txreq.offset, txreq.size,
				   (txreq.offset&~PAGE_MASK) + txreq.size);
			netbk_fatal_tx_err(vif);
			continue;
		}

		index = pending_index(netbk->pending_cons);
			gso = &extras[XEN_NETIF_EXTRA_TYPE_GSO - 1];

			if (netbk_set_skb_gso(vif, skb, gso)) {
				/* Failure in netbk_set_skb_gso is fatal. */
				kfree_skb(skb);
				continue;
			}
		}

			txp->size -= data_len;
		} else {
			/* Schedule a response immediately. */
			xen_netbk_idx_release(netbk, pending_idx, XEN_NETIF_RSP_OKAY);
		}

		if (txp->flags & XEN_NETTXF_csum_blank)
			skb->ip_summed = CHECKSUM_PARTIAL;
	xen_netbk_tx_submit(netbk);
}

static void xen_netbk_idx_release(struct xen_netbk *netbk, u16 pending_idx,
				  u8 status)
{
	struct xenvif *vif;
	struct pending_tx_info *pending_tx_info;
	pending_ring_idx_t index;

	vif = pending_tx_info->vif;

	make_tx_response(vif, &pending_tx_info->req, status);

	index = pending_index(netbk->pending_prod++);
	netbk->pending_ring[index] = pending_idx;
